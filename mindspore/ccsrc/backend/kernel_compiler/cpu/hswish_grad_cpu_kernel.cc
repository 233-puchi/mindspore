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

#include "backend/kernel_compiler/cpu/hswish_grad_cpu_kernel.h"
#include <algorithm>
#include "runtime/device/cpu/cpu_device_address.h"

namespace mindspore {
namespace kernel {
template <typename T>
void HSwishGradCPUKernel<T>::InitKernel(const CNodePtr &kernel_node) {
  CheckParam(kernel_node);
  x_shape_ = AnfAlgo::GetPrevNodeOutputInferShape(kernel_node, 1);
  for (const uint64_t &d : x_shape_) {
    tensor_size_ *= d;
  }
}

template <typename T>
bool HSwishGradCPUKernel<T>::Launch(const std::vector<kernel::AddressPtr> &inputs,
                                    const std::vector<kernel::AddressPtr> & /*workspace*/,
                                    const std::vector<kernel::AddressPtr> &outputs) {
  auto dy = reinterpret_cast<T *>(inputs[0]->addr);
  auto x = reinterpret_cast<T *>(inputs[1]->addr);
  auto out = reinterpret_cast<T *>(outputs[0]->addr);
  auto task = [&](size_t start, size_t end) {
    for (uint64_t i = start; i < end; ++i) {
      if (x[i] <= -3) {
        out[i] = 0;
      } else if (x[i] >= 3) {
        out[i] = dy[i];
      } else {
        out[i] = dy[i] * (2 * x[i] + 3) / 6;
      }
    }
  };
  CPUKernelUtils::ParallelFor(task, tensor_size_);
  return true;
}

template <typename T>
void HSwishGradCPUKernel<T>::CheckParam(const CNodePtr &kernel_node) {
  size_t input_num = AnfAlgo::GetInputTensorNum(kernel_node);
  if (input_num != 2) {
    MS_LOG(EXCEPTION) << "Input number is " << input_num << ", but HSwishGradCPUKernel needs 2 input.";
  }
  size_t output_num = AnfAlgo::GetOutputTensorNum(kernel_node);
  if (output_num != 1) {
    MS_LOG(EXCEPTION) << "Output number is " << output_num << ", but HSwishGradCPUKernel needs 1 output.";
  }
}
}  // namespace kernel
}  // namespace mindspore