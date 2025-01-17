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
#include "nnacl/base/unsorted_segment_sum_base.h"
#include "nnacl/errorcode.h"

#define UNSORTEDSEGMENTSUM(type)                                                                                   \
  int UnsortedSegmentSum_##type(const type *input, int unit_num, int input_dim1, const int *indices, type *output, \
                                int output_dim0, int output_dim1) {                                                \
    if (input_dim1 == 0) {                                                                                         \
      return NNACL_ERR;                                                                                            \
    }                                                                                                              \
    for (int i = 0; i < unit_num; ++i) {                                                                           \
      int j = i / input_dim1;                                                                                      \
      int k = i % input_dim1;                                                                                      \
                                                                                                                   \
      int index = indices[j];                                                                                      \
      if (index < 0 || index >= output_dim0) {                                                                     \
        continue;                                                                                                  \
      }                                                                                                            \
      int output_index = index * output_dim1 + k;                                                                  \
      output[output_index] += input[i];                                                                            \
    }                                                                                                              \
    return NNACL_OK;                                                                                               \
  }

UNSORTEDSEGMENTSUM(int)
UNSORTEDSEGMENTSUM(float)
