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

#include "ps/server/round.h"
#include <memory>
#include <string>

namespace mindspore {
namespace ps {
namespace server {
Round::Round(const std::string &name, bool check_timeout, size_t time_window, bool check_count, size_t threshold_count)
    : name_(name),
      check_timeout_(check_timeout),
      time_window_(time_window),
      check_count_(check_count),
      threshold_count_(threshold_count) {}

void Round::Initialize(const std::shared_ptr<core::CommunicatorBase> &communicator, TimeOutCb timeout_cb,
                       FinishIterCb finish_iteration_cb) {
  MS_EXCEPTION_IF_NULL(communicator);
  communicator_ = communicator;

  // Register callback for round kernel.
  communicator_->RegisterMsgCallBack(
    name_, [&](std::shared_ptr<core::MessageHandler> message) { LaunchRoundKernel(message); });

  // Callback when the iteration is finished.
  finish_iteration_cb_ = [this, finish_iteration_cb](bool is_iteration_valid) -> void {
    MS_LOG(INFO) << "Round " << name_ << " finished! This iteration is valid. Proceed to next iteration.";
    finish_iteration_cb(is_iteration_valid);
  };

  // Callback for finalizing the server. This can only be called once.
  finalize_cb_ = [&](void) -> void { communicator_->Stop(); };

  if (check_timeout_) {
    iter_timer_ = std::make_shared<IterationTimer>();

    // 1.Set the timeout callback for the timer.
    iter_timer_->SetTimeOutCallBack([this, timeout_cb](bool is_iteration_valid) -> void {
      MS_LOG(INFO) << "Round " << name_ << " timeout! This iteration is invalid. Proceed to next iteration.";
      timeout_cb(is_iteration_valid);
    });

    // 2.Stopping timer callback which will be set to the round kernel.
    stop_timer_cb_ = [&](void) -> void {
      MS_LOG(INFO) << "Round " << name_ << " kernel stops its timer.";
      iter_timer_->Stop();
    };
  }

  // Set counter event callbacks for this round if the round kernel is stateful.
  if (check_count_) {
    auto first_count_handler = std::bind(&Round::OnFirstCountEvent, this, std::placeholders::_1);
    auto last_count_handler = std::bind(&Round::OnLastCountEvent, this, std::placeholders::_1);
    DistributedCountService::GetInstance().RegisterCounter(name_, threshold_count_,
                                                           {first_count_handler, last_count_handler});
  }
}

void Round::BindRoundKernel(const std::shared_ptr<kernel::RoundKernel> &kernel) {
  MS_EXCEPTION_IF_NULL(kernel);
  kernel_ = kernel;
  kernel_->set_stop_timer_cb(stop_timer_cb_);
  kernel_->set_finish_iteration_cb(finish_iteration_cb_);
  return;
}

void Round::LaunchRoundKernel(const std::shared_ptr<core::MessageHandler> &message) {
  if (message == nullptr) {
    MS_LOG(ERROR) << "Message is nullptr.";
    return;
  }

  AddressPtr input = std::make_shared<Address>();
  AddressPtr output = std::make_shared<Address>();
  input->addr = message->data();
  input->size = message->len();
  bool ret = kernel_->Launch({input}, {}, {output});
  if (output->size == 0) {
    std::string reason = "The output of the round " + name_ + " is empty.";
    MS_LOG(WARNING) << reason;
    communicator_->SendResponse(reason.c_str(), reason.size(), message);
    return;
  }

  // Must send response back no matter what value Launch method returns.
  if (!ret) {
    MS_LOG(WARNING) << "Launching round kernel of round " << name_ << " failed.";
  }
  communicator_->SendResponse(output->addr, output->size, message);
  kernel_->Release(output);
  return;
}

void Round::Reset() { kernel_->Reset(); }

const std::string &Round::name() const { return name_; }

size_t Round::threshold_count() const { return threshold_count_; }

bool Round::check_timeout() const { return check_timeout_; }

size_t Round::time_window() const { return time_window_; }

void Round::OnFirstCountEvent(const std::shared_ptr<core::MessageHandler> &message) {
  MS_LOG(INFO) << "Round " << name_ << " first count event is triggered.";
  // The timer starts only after the first count event is triggered by DistributedCountService.
  if (check_timeout_) {
    iter_timer_->Start(std::chrono::milliseconds(time_window_));
  }

  // Some kernels override the OnFirstCountEvent method.
  kernel_->OnFirstCountEvent(message);
  return;
}

void Round::OnLastCountEvent(const std::shared_ptr<core::MessageHandler> &message) {
  MS_LOG(INFO) << "Round " << name_ << " last count event is triggered.";
  // Same as the first count event, the timer must be stopped by DistributedCountService.
  if (check_timeout_) {
    iter_timer_->Stop();
  }

  // Some kernels override the OnLastCountEvent method.
  kernel_->OnLastCountEvent(message);
  return;
}
}  // namespace server
}  // namespace ps
}  // namespace mindspore
