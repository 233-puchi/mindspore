/**
 * Copyright 2020 Huawei Technologies Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MINDSPORE_LITE_TOOLS_CONVERTER_QUANTIZER_QUANTIZE_UTIL_H_
#define MINDSPORE_LITE_TOOLS_CONVERTER_QUANTIZER_QUANTIZE_UTIL_H_

#include <dirent.h>
#include <sys/stat.h>
#include <memory>
#include <string>
#include <cmath>
#include <array>
#include <vector>
#include <algorithm>
#include <limits>
#include <utility>
#include "ops/mat_mul.h"
#include "ops/lstm.h"
#include "ops/fusion/full_connection.h"
#include "tools/converter/quantizer/quantizer.h"
#include "include/errorcode.h"
#include "ir/func_graph.h"
#include "ir/anf.h"
#include "include/model.h"
#include "base/base.h"
#include "ir/primitive.h"
#include "abstract/dshape.h"
#include "tools/converter/quantizer/huffman_encode.h"
#include "tools/converter/quantizer/bitpacking.h"
#include "src/lite_session.h"
#include "tools/converter/graphdef_transform.h"
#include "src/common/file_utils.h"
#include "src/common/quant_utils.h"

namespace mindspore::lite::quant {
static constexpr size_t UINT8_QUANTIZATION = 8;
static constexpr size_t WEIGHT_INDEX = 1;
static constexpr double SCALE_THREASHOLD = 1e-38;
const char kMethodMaxMin[] = "MAX_MIN";
const char kMethodKL[] = "KL";
const char kMethodOutlier[] = "RemovalOutlier";

struct PostQuantConfig {
  std::vector<std::string> image_paths;
  uint32_t batch_count{100};
  std::string method_x{kMethodKL};
  uint32_t thread_num{1};
  bool bias_correction{false};
  bool mixed{false};
  float mean_error_threshold{0.04};
  std::vector<std::vector<std::vector<int>>> input_shapes;  // different input
  bool inited{false};
};

struct SessionModel {
  session::LiteSession *session{nullptr};
  Model *model{nullptr};
};

/**
 * 1. when op's weight size > mWeightSize just skip
 * 2. only do conv/deconv/convdepthwise/deconvdepthwise/mul/matmul/batchmatmul quantization
 * 3. when conv/deconv/convdepthwise/deconvdepthwise ops' weight channel size > covWeightQuantChannelThreshold just skip
 * */
class QuantStrategy {
 public:
  explicit QuantStrategy(size_t weightSize, size_t covWeightQuantChannelThreshold = 16);

  ~QuantStrategy() = default;

  bool CanConvOpQuantized(const CNodePtr &node) const;
  bool CanMulOpQuantized(const CNodePtr &node) const;
  bool CanOpPostQuantized(const AnfNodePtr &node) const;
  bool CanTensorQuantized(const AnfNodePtr &inputNode) const;

  size_t m_weight_size_;
  size_t m_conv_weight_quant_channel_threshold_;

 private:
  static const std::vector<std::string> conv_types_;
  static const std::vector<std::string> mul_types_;
};

constexpr float delta = 0.1;
constexpr float ratio = 10.0;
constexpr int percent = 10;
constexpr int quant_param_size = 32 * 8;

QuantParamHolderPtr GetCNodeQuantHolder(const PrimitivePtr &primitive);

STATUS CalQuantizationParams(schema::QuantParamT *quantParam, double mMin, double mMax, bool narrowRange = false,
                             int numBits = UINT8_QUANTIZATION);

std::pair<float, float> OutlierMethod(std::vector<float> min_datas, std::vector<float> max_datas);

std::vector<int8_t> KMeans(float *data, size_t elem_count, size_t k, size_t epochs, schema::QuantParamT *quantParam);

STATUS UpdateTensorDataAndSize(const tensor::TensorPtr &weight, void *quant_datas, int new_size, TypeId new_data_type);

int CalChannels(const ShapeVector &dims, int channel_cnt, bool *channel_at_first);

void CalQuantAssitInfo(const PrimitivePtr &primitive, const ShapeVector &shapes, int index, bool *channel_at_first,
                       int *channel_cnt);

void CalQuantAssitInfo(const schema::PrimitiveT &primitive, const std::vector<int> &shapes, int index,
                       bool *channel_at_first, int *channel_cnt);

bool QuantParamEqual(const schema::QuantParamT &quant_param1, const schema::QuantParamT &quant_param2);

bool TensorQuantParamsInited(const schema::TensorT &tensor);

template <typename T>
STATUS DoBitPack(const tensor::TensorPtr &weight, const size_t &bit_num, const std::vector<T> &quant_datas) {
  if (bit_num != 8 && bit_num != 16) {
    std::vector<T> data{};
    for (size_t i = 0; i < quant_datas.size(); ++i) {
      data.emplace_back((static_cast<T>(quant_datas[i])));
    }
    if (bit_num > 0 && bit_num < 8) {
      std::vector<uint8_t> pack_data{};
      BitPack::BitPacking<T, uint8_t>(bit_num, data, &pack_data);
      auto status =
        UpdateTensorDataAndSize(weight, pack_data.data(), pack_data.size() * sizeof(uint8_t), kNumberTypeUInt8);
      if (status != RET_OK) {
        MS_LOG(ERROR) << "UpdateTensorDataAndSize error";
        return RET_ERROR;
      }
    } else if (bit_num > 8 && bit_num < 16) {
      std::vector<uint16_t> pack_data{};
      BitPack::BitPacking<T, uint16_t>(bit_num, data, &pack_data);
      auto status =
        UpdateTensorDataAndSize(weight, pack_data.data(), pack_data.size() * sizeof(uint16_t), kNumberTypeUInt16);
      if (status != RET_OK) {
        MS_LOG(ERROR) << "UpdateTensorDataAndSize error";
        return RET_ERROR;
      }
    }
  }
  return RET_OK;
}

template <typename T>
STATUS QuantFilter(const tensor::TensorPtr &weight, const PrimitivePtr &primitive, QuantType quant_type, int quant_max,
                   int quant_min, size_t bit_num, bool per_channel, TypeId quant_data_type, int index = 1,
                   bool k_means = false) {
  MS_ASSERT(weight != nullptr);
  MS_ASSERT(primitive != nullptr);
  auto dims = weight->shape();
  if (per_channel) {
    if (dims.size() <= 1) {
      MS_LOG(WARNING) << "dims is " << dims.size() << " can not per_channel";
      per_channel = false;
    }
  }

  std::vector<schema::QuantParamT> quant_params;
  size_t elem_count = weight->DataSize();
  auto *raw_data = static_cast<float *>(weight->data_c());
  if (raw_data == nullptr) {
    MS_LOG(ERROR) << "rawDatas is nullptr";
    return RET_ERROR;
  }

  std::vector<T> quant_data(elem_count);
  int ret = RET_OK;
  if (per_channel) {
    bool channel_at_first = true;
    int channel_cnt = -1;
    CalQuantAssitInfo(primitive, dims, index, &channel_at_first, &channel_cnt);
    auto channels = CalChannels(dims, channel_cnt, &channel_at_first);
    if (channels == 0) {
      MS_LOG(ERROR) << "channels is zero";
      return RET_ERROR;
    }
    ret = DoPerChannelQuant<T>(static_cast<float *>(weight->data_c()), weight->DataSize(),
                               static_cast<mindspore::schema::QuantType>(quant_type), &quant_params, quant_max,
                               quant_min, bit_num, k_means, &quant_data, channels, channel_at_first);
    if (ret == RET_CONTINUE) {
      return ret;
    } else if (ret != RET_OK) {
      MS_LOG(ERROR) << "Do per channel quant failed.";
      return ret;
    }
  } else {
    ret = DoPerLayerQuant<T>(static_cast<float *>(weight->data_c()), weight->DataSize(), &quant_params, quant_max,
                             quant_min, bit_num, k_means, &quant_data);
    if (ret != RET_OK) {
      MS_LOG(ERROR) << "Do per layer quant failed.";
      return ret;
    }
  }
  auto status = UpdateTensorDataAndSize(weight, quant_data.data(), quant_data.size() * sizeof(T), quant_data_type);
  if (status != RET_OK) {
    MS_LOG(ERROR) << "UpdateTensorDataAndSize error";
    return RET_ERROR;
  }

#ifdef HUFFMAN_ENCODE
  auto huffman_encode = std::make_unique<lite::HuffmanEncode>();
  ret = huffman_encode->DoHuffmanEncode(weight, primitive, quant_datas.data(), bit_num);
  if (ret != RET_OK) {
    MS_LOG(ERROR) << "Do huffman encode failed.";
    return ret;
  }
#endif

  if (quant_params.empty()) {
    MS_LOG(ERROR) << "quant_params empty";
    return RET_ERROR;
  }
  auto quant_param_holder = GetCNodeQuantHolder(primitive);
  if (quant_type == QuantType_PostTraining) {
    quant_param_holder->AddInputQuantParam(quant_params);
  } else {
    quant_param_holder->set_input_quant_param(index, quant_params);
  }
  return ret;
}

// utils

std::string NodePrimitiveType(const CNodePtr &cnode);

STATUS ParseConfigFile(std::string config_file, PostQuantConfig *post_quant_config);

SessionModel CreateSessionByFuncGraph(const FuncGraphPtr &func_graph, const converter::Flags &flags, int thread_num);

STATUS CollectCalibInputs(const std::vector<std::string> &input_dirs, size_t count_limited,
                          std::vector<std::vector<std::string>> *inputs);

STATUS CopyInputDataToTensor(size_t input_index, size_t image_index,
                             const std::vector<std::vector<std::string>> &images, mindspore::tensor::MSTensor *tensor);

FuncGraphPtr CopyFuncGraph(const FuncGraphPtr &);

void GetLiteParameter(const AnfNodePtr &node, ParameterPtr *param_node, tensor::TensorPtr *tensor_info);
}  // namespace mindspore::lite::quant
#endif  // MINDSPORE_LITE_TOOLS_CONVERTER_QUANTIZER_QUANTIZE_UTIL_H_
