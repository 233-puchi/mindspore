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

#ifndef MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_OPCLIB_FP32_SOFTMAX_GRAD_H_
#define MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_OPCLIB_FP32_SOFTMAX_GRAD_H_

#include "src/runtime/kernel/arm/opclib/op_base.h"

struct SoftmaxCrossEntropyParameter {
    OpParameter op_parameter;
    int32_t batch_size_;
    unsigned int number_of_classes_;
    int n_dim_;
    int input_shape_[5];
};
#endif  // MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_OPCLIB_FP32_SOFTMAX_GRAD_H_
