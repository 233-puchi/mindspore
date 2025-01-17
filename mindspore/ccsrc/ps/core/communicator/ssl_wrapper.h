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

#ifndef MINDSPORE_CCSRC_PS_CORE_COMMUNICATOR_SSL_WRAPPER_H_
#define MINDSPORE_CCSRC_PS_CORE_COMMUNICATOR_SSL_WRAPPER_H_

#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#include "utils/log_adapter.h"

namespace mindspore {
namespace ps {
namespace core {
class SSLWrapper {
 public:
  static SSLWrapper &GetInstance() {
    static SSLWrapper instance;
    return instance;
  }
  SSL_CTX *GetSSLCtx(bool is_server = true);

 private:
  SSLWrapper();
  virtual ~SSLWrapper();
  SSLWrapper(const SSLWrapper &) = delete;
  SSLWrapper &operator=(const SSLWrapper &) = delete;

  void InitSSL();
  void CleanSSL();

  SSL_CTX *ssl_ctx_;
  SSL_CTX *client_ssl_ctx_;
};
}  // namespace core
}  // namespace ps
}  // namespace mindspore
#endif  // MINDSPORE_CCSRC_PS_CORE_COMMUNICATOR_SSL_WRAPPER_H_
