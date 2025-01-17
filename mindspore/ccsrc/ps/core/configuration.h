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

#ifndef MINDSPORE_CCSRC_PS_CORE_CONFIGURATION_H_
#define MINDSPORE_CCSRC_PS_CORE_CONFIGURATION_H_

#include <atomic>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <unordered_map>

#include "ps/constants.h"
#include "utils/log_adapter.h"

namespace mindspore {
namespace ps {
namespace core {
// This is a basic configuration class to store persistent information, which can be stored in a database or file
class Configuration {
 public:
  Configuration() = default;
  virtual ~Configuration() = default;

  // Initialize database connection or load config file.
  virtual bool Initialize() = 0;

  // Get configuration data from database or config file.
  virtual std::string Get(const std::string &key, const std::string &defaultvalue) const = 0;

  // Put configuration data to database or config file.
  virtual void Put(const std::string &key, const std::string &defaultvalue) = 0;
};
}  // namespace core
}  // namespace ps
}  // namespace mindspore
#endif  // MINDSPORE_CCSRC_PS_CORE_CONFIGURATION_H_
