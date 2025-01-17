# Copyright 2021 Huawei Technologies Co., Ltd
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ============================================================================

"""
TopK for text generation
"""

import numpy as np
import mindspore.common.dtype as mstype
from mindspore.common.tensor import Tensor

def generate(model, origin_inputs, seq_length, end_token=50256):
    """
    TopK for text generation

    Inputs:
        model: the model for inferencing
        origin_inputs: the original inputs based on which the model will continue writing
        seq_length: seq_length for the model
        end_token: end of sentence token id

    Returns:
        outputs: the ids for the generated text
    """
    seq_length = seq_length
    _, valid_length = origin_inputs.shape
    pad_length = seq_length - origin_inputs.shape[-1]
    # Pad original inputs to seq_length
    input_ids = np.pad(origin_inputs, ((0, 0), (0, pad_length)), 'constant', constant_values=(0, 0))
    print("input_ids is ", input_ids)

    # A single loop generates one token, loop until reaching target seq_length or generating eod token
    while valid_length < seq_length:
        inputs = Tensor(input_ids, mstype.int32)
        # Call a single inference
        probs, p_args = model.predict(inputs)
        # Get the topk value and index for the current position from the padded outputs
        probs = probs.asnumpy()[valid_length-1, :]
        p_args = p_args.asnumpy()[valid_length-1, :]
        # Random select a token as final output for this round
        p = probs
        p = p / sum(p)
        target_index = np.random.choice(len(p), p=p)
        # Stop judgment
        if p_args[target_index] == end_token or valid_length == seq_length-1:
            outputs = input_ids
            break
        # Modify input_ids with newly generated token
        input_ids[0][valid_length] = p_args[target_index]
        valid_length += 1
    # Return valid outputs out of padded outputs
    length = np.sum(outputs != 0)
    outputs = outputs[0][:length]
    return outputs
