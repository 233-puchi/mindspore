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
"""eval Xception."""
import argparse
import numpy as np

from mindspore import Tensor, context, load_checkpoint, load_param_into_net, export

from src.Xception import xception
from src.config import config_ascend, config_gpu

parser = argparse.ArgumentParser(description="Image classification")
parser.add_argument("--device_id", type=int, default=0, help="Device id")
parser.add_argument("--batch_size", type=int, default=1, help="batch size")
parser.add_argument("--ckpt_file", type=str, required=True, help="xception ckpt file.")
parser.add_argument("--width", type=int, default=299, help="input width")
parser.add_argument("--height", type=int, default=299, help="input height")
parser.add_argument("--file_name", type=str, default="xception", help="xception output file name.")
parser.add_argument("--file_format", type=str, choices=["AIR", "MINDIR"], default="MINDIR", help="file format")
parser.add_argument("--device_target", type=str, choices=["Ascend", "GPU", "CPU"], default="Ascend",
                    help="device target")

args = parser.parse_args()
if args.device_target == "Ascend":
    config = config_ascend
elif args.device_target == "GPU":
    config = config_gpu
else:
    raise ValueError("Unsupported device_target.")

context.set_context(mode=context.GRAPH_MODE, device_target=args.device_target)
context.set_context(device_id=args.device_id)

if __name__ == "__main__":
    # define net
    net = xception(class_num=config.class_num)

    # load checkpoint
    param_dict = load_checkpoint(args.ckpt_file)
    load_param_into_net(net, param_dict)
    net.set_train(False)

    image = Tensor(np.zeros([args.batch_size, 3, args.height, args.width], np.float32))
    export(net, image, file_name=args.file_name, file_format=args.file_format)
