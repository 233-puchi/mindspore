# Copyright 2020 Huawei Technologies Co., Ltd
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
"""export checkpoint file into air, onnx, mindir models"""
import argparse
import numpy as np

import mindspore.nn as nn
import mindspore.ops as ops
from mindspore import Tensor, context, load_checkpoint, load_param_into_net, export
from src.nets import net_factory

parser = argparse.ArgumentParser(description='checkpoint export')
parser.add_argument("--device_id", type=int, default=0, help="Device id")
parser.add_argument("--batch_size", type=int, default=1, help="batch size")
parser.add_argument("--input_size", type=int, default=513, help="batch size")
parser.add_argument("--ckpt_file", type=str, required=True, help="Checkpoint file path.")
parser.add_argument("--file_name", type=str, default="deeplabv3", help="output file name.")
parser.add_argument('--file_format', type=str, choices=["AIR", "MINDIR"], default='AIR', help='file format')
parser.add_argument("--device_target", type=str, choices=["Ascend", "GPU", "CPU"], default="Ascend",
                    help="device target")
parser.add_argument('--model', type=str.lower, default='deeplab_v3_s8', choices=['deeplab_v3_s16', 'deeplab_v3_s8'],
                    help='Select model structure (Default: deeplab_v3_s8)')
parser.add_argument('--num_classes', type=int, default=21, help='the number of classes (Default: 21)')
parser.add_argument("--input_format", type=str, choices=["NCHW", "NHWC"], default="NCHW",
                    help="NCHW or NHWC")
args = parser.parse_args()

context.set_context(mode=context.GRAPH_MODE, device_target=args.device_target)
if args.device_target == "Ascend":
    context.set_context(device_id=args.device_id)

class BuildEvalNetwork(nn.Cell):
    def __init__(self, net, input_format="NCHW"):
        super(BuildEvalNetwork, self).__init__()
        self.network = net
        self.softmax = nn.Softmax(axis=1)
        self.transpose = ops.Transpose()
        self.format = input_format

    def construct(self, x):
        if self.format == "NHWC":
            x = self.transpose(x, (0, 3, 1, 2))
        output = self.network(x)
        output = self.softmax(output)
        return output

if __name__ == '__main__':
    if args.model == 'deeplab_v3_s16':
        network = net_factory.nets_map['deeplab_v3_s16']('eval', args.num_classes, 16, True)
    else:
        network = net_factory.nets_map['deeplab_v3_s8']('eval', args.num_classes, 8, True)
    network = BuildEvalNetwork(network, args.input_format)
    param_dict = load_checkpoint(args.ckpt_file)

    # load the parameter into net
    load_param_into_net(network, param_dict)
    if args.input_format == "NHWC":
        input_data = Tensor(np.ones([args.batch_size, args.input_size, args.input_size, 3]).astype(np.float32))
    else:
        input_data = Tensor(np.ones([args.batch_size, 3, args.input_size, args.input_size]).astype(np.float32))
    export(network, input_data, file_name=args.file_name, file_format=args.file_format)
