/**
 * Copyright 2021 Huawei Technologies Co., Ltd
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

#include "nnacl/infer/fill_infer.h"

int FillInferShape(const TensorC *const *inputs, size_t inputs_size, TensorC **outputs, size_t outputs_size,
                   OpParameter *parameter) {
  int check_ret = CheckAugmentNullSize(inputs, inputs_size, outputs, outputs_size, parameter, 1, 1);
  if (check_ret != NNACL_OK) {
    return check_ret;
  }
  const TensorC *input = inputs[0];
  TensorC *output = outputs[0];
  SetDataTypeFormat(output, input);
  const TensorC *dst_shape_tensor = inputs[1];
  const int32_t *dst_shape = (int32_t *)(dst_shape_tensor->data_);
  const size_t num_dims = (size_t)(dst_shape_tensor->shape_[0]);
  if (!parameter->infer_flag_) {
    return NNACL_INFER_INVALID;
  }

  int output_shape[MAX_SHAPE_SIZE];
  size_t output_shape_size = 0;
  for (size_t i = 0; i < num_dims; i++) {
    ShapePush(output_shape, &output_shape_size, dst_shape[i]);
  }
  SetShapeArray(output, output_shape, output_shape_size);
  return NNACL_OK;
}