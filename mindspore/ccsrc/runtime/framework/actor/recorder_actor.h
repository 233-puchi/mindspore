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

#ifndef MINDSPORE_CCSRC_RUNTIME_FRAMEWORK_ACTOR_RECORDER_ACTOR_H_
#define MINDSPORE_CCSRC_RUNTIME_FRAMEWORK_ACTOR_RECORDER_ACTOR_H_

#include <memory>
#include "runtime/framework/actor/actor_common.h"
#include "runtime/framework/device_tensor_store.h"
#include "runtime/hardware/device_context.h"

namespace mindspore {
namespace runtime {
using mindspore::device::DeviceContext;
using mindspore::kernel::KernelLaunchInfo;

// The recorder actor is used to record kernel info for RDR module.
class RecorderActor : public ActorBase {
 public:
  RecorderActor() : ActorBase("RecorderActor") {}
  ~RecorderActor() override = default;

  // The memory recorder of each node.
  void RecordMemAddressInfo(const AnfNode *node, const KernelLaunchInfo *launch_info_,
                            const DeviceContext *device_context, OpContext<DeviceTensor> *op_context);

  // Clear memory recorder at the step end.
  void ClearMemAddressInfo(OpContext<DeviceTensor> *op_context);
};
}  // namespace runtime
}  // namespace mindspore

#endif  // MINDSPORE_CCSRC_RUNTIME_FRAMEWORK_ACTOR_RECORDER_ACTOR_H_
