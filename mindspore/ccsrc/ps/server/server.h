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

#ifndef MINDSPORE_CCSRC_PS_SERVER_SERVER_H_
#define MINDSPORE_CCSRC_PS_SERVER_SERVER_H_

#include <memory>
#include <string>
#include <vector>
#include "ps/core/communicator/communicator_base.h"
#include "ps/core/communicator/tcp_communicator.h"
#include "ps/core/communicator/task_executor.h"
#include "ps/server/common.h"
#include "ps/server/executor.h"
#include "ps/server/iteration.h"

namespace mindspore {
namespace ps {
namespace server {
// Class Server is the entrance of MindSpore's parameter server training mode and federated learning.
class Server {
 public:
  static Server &GetInstance() {
    static Server instance;
    return instance;
  }

  void Initialize(bool use_tcp, bool use_http, uint16_t http_port, const std::vector<RoundConfig> &rounds_config,
                  const FuncGraphPtr &func_graph, size_t executor_threshold);

  // According to the current MindSpore framework, method Run is a step of the server pipeline. This method will be
  // blocked until the server is finalized.
  // func_graph is the frontend graph which will be parse in server's exector and aggregator.
  void Run();

 private:
  Server()
      : server_node_(nullptr),
        task_executor_(nullptr),
        use_tcp_(false),
        use_http_(false),
        http_port_(0),
        func_graph_(nullptr),
        executor_threshold_(0),
        communicator_with_server_(nullptr),
        communicators_with_worker_({}),
        iteration_(nullptr),
        scheduler_ip_(""),
        scheduler_port_(0),
        server_num_(0),
        worker_num_(0) {}
  ~Server() = default;
  Server(const Server &) = delete;
  Server &operator=(const Server &) = delete;

  // Load variables which is set by ps_context.
  void InitServerContext();

  // Initialize the server cluster, server node and communicators.
  void InitCluster();
  bool InitCommunicatorWithServer();
  bool InitCommunicatorWithWorker();

  // Initialize iteration with rounds. Which rounds to use could be set by ps_context as well.
  void InitIteration();

  // Initialize executor according to the server mode.
  void InitExecutor();

  // Create round kernels and bind these kernels with corresponding Round.
  void RegisterRoundKernel();

  // The communicators should be started after all initializations are completed.
  void StartCommunicator();

  // The server node is initialized in Server.
  std::shared_ptr<core::ServerNode> server_node_;

  // The task executor of the communicators. This helps server to handle network message concurrently. The tasks
  // submitted to this task executor is asynchronous.
  std::shared_ptr<core::TaskExecutor> task_executor_;

  // Which protocol should communicators use.
  bool use_tcp_;
  bool use_http_;
  uint64_t http_port_;

  // The configure of all rounds.
  std::vector<RoundConfig> rounds_config_;

  // The graph passed by the frontend without backend optimizing.
  FuncGraphPtr func_graph_;

  // The threshold count for executor to do aggregation or optimizing.
  size_t executor_threshold_;

  // Server need a tcp communicator to communicate with other servers for counting, metadata storing, collective
  // operations, etc.
  std::shared_ptr<core::CommunicatorBase> communicator_with_server_;

  // The communication with workers(including mobile devices), has multiple protocol types: HTTP and TCP.
  // In some cases, both types should be supported in one distributed training job. So here we may have multiple
  // communicators.
  std::vector<std::shared_ptr<core::CommunicatorBase>> communicators_with_worker_;

  // Iteration consists of multiple kinds of rounds.
  Iteration *iteration_;

  // Variables set by ps context.
  std::string scheduler_ip_;
  uint16_t scheduler_port_;
  uint32_t server_num_;
  uint32_t worker_num_;
};
}  // namespace server
}  // namespace ps
}  // namespace mindspore
#endif  // MINDSPORE_CCSRC_PS_SERVER_SERVER_H_
