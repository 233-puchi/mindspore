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

#include "ps/core/node_manager.h"

namespace mindspore {
namespace ps {
namespace core {
void NodeManager::InitNode() {
  initial_total_node_num_ = PSContext::instance()->cluster_config().initial_server_num +
                            PSContext::instance()->cluster_config().initial_worker_num;
  meta_data_ = std::make_unique<ClusterMetadata>(PSContext::instance()->cluster_config().initial_worker_num,
                                                 PSContext::instance()->cluster_config().initial_server_num);
  total_node_num_ = initial_total_node_num_;
}

int NodeManager::NextRankId(const RegisterMessage &register_message) {
  std::lock_guard<std::mutex> lock(assign_rank_id_mutex_);
  int rank_id = -1;

  const std::string &node_id = register_message.node_id();
  if (nodes_info_.find(node_id) != nodes_info_.end()) {
    rank_id = nodes_info_[node_id].rank_id_;
    MS_LOG(INFO) << "The node id: " << node_id << " is already assigned!";
    return rank_id;
  }

  if (register_message.role() == NodeRole::SERVER) {
    const std::string &ip = register_message.ip();
    uint32_t port = register_message.port();

    rank_id = ++next_server_rank_id_;
    if (IntToUint(rank_id) >= meta_data_->server_num) {
      MS_LOG(WARNING) << "The rank id is greater than the number of servers:" << meta_data_->server_num;
      rank_id = -1;
      --next_server_rank_id_;
    }
    NodeInfo node_info;
    node_info.node_role_ = NodeRole::SERVER;
    node_info.node_id_ = node_id;
    node_info.rank_id_ = rank_id;
    node_info.ip_ = ip;
    node_info.port_ = port;
    nodes_info_[node_id] = node_info;
    MS_LOG(INFO) << "The server node id:" << node_id << ",node ip: " << node_info.ip_ << ",node port:" << port
                 << " assign rank id:" << rank_id;
  } else if (register_message.role() == NodeRole::WORKER) {
    const std::string &ip = register_message.ip();
    uint32_t port = register_message.port();
    rank_id = ++next_worker_rank_id_;
    if (IntToUint(rank_id) >= meta_data_->worker_num) {
      MS_LOG(WARNING) << "The rank id is greater than the number of workers:" << meta_data_->worker_num;
      rank_id = -1;
      --next_worker_rank_id_;
    }
    NodeInfo node_info;
    node_info.node_role_ = NodeRole::WORKER;
    node_info.node_id_ = node_id;
    node_info.rank_id_ = rank_id;
    node_info.ip_ = ip;
    node_info.port_ = port;
    nodes_info_[node_id] = node_info;
    MS_LOG(INFO) << "The worker node id:" << node_id << " assign rank id:" << rank_id;
  }
  return rank_id;
}

void NodeManager::UpdateHeartbeat(const std::string &node_id) {
  std::lock_guard<std::mutex> lock(heartbeat_mutex_);
  struct timeval current_time {};
  (void)gettimeofday(&current_time, nullptr);
  heartbeats_[node_id] = current_time;
}

void NodeManager::UpdateNodeScaleInState(const std::string &node_id) { heartbeats_scale_in_nodes_.insert(node_id); }

bool NodeManager::CheckNodesScaluOutState() { return SizeToInt(heartbeats_scale_out_nodes_.size()) == total_node_num_; }

bool NodeManager::CheckNodesScaleInState() { return SizeToInt(heartbeats_scale_in_nodes_.size()) == total_node_num_; }

std::vector<ServersMeta> NodeManager::FetchServersMeta() {
  std::vector<ServersMeta> servers_meta_list;
  for (auto it = nodes_info_.begin(); it != nodes_info_.end(); ++it) {
    if (it->second.node_role_ == NodeRole::SERVER) {
      ServersMeta servers_meta;
      servers_meta.set_rank_id(it->second.rank_id_);
      servers_meta.set_ip(it->second.ip_);
      servers_meta.set_port(it->second.port_);
      servers_meta_list.push_back(servers_meta);
    }
  }
  return servers_meta_list;
}

void NodeManager::UpdateCluster() {
  // 1. update cluster timeout state
  struct timeval current_time {};
  (void)gettimeofday(&current_time, nullptr);
  timeout_nodes_info_.clear();
  for (auto it = heartbeats_.begin(); it != heartbeats_.end(); ++it) {
    if (it->second.tv_sec + PSContext::instance()->cluster_config().heartbeat_timeout < current_time.tv_sec) {
      if (nodes_info_.count(it->first)) {
        MS_LOG(WARNING) << "The node id:" << it->first << " is timeout!";
        timeout_nodes_info_[it->first] = nodes_info_[it->first];
      }
    }
  }
  if (!timeout_nodes_info_.empty()) {
    UpdateClusterState(ClusterState::CLUSTER_TIMEOUT);
    for (auto it = timeout_nodes_info_.begin(); it != timeout_nodes_info_.end(); ++it) {
      finish_nodes_id_.insert(it->first);
    }
  }

  // 2. update cluster finish state
  if (SizeToInt(finish_nodes_id_.size()) == total_node_num_ ||
      SizeToInt(finish_nodes_id_.size()) == current_node_num_) {
    UpdateClusterState(ClusterState::CLUSTER_FINISH);
  }
}

void NodeManager::CheckClusterTimeout() {
  if (total_node_num_ != SizeToInt(nodes_info_.size())) {
    MS_LOG(WARNING) << "The cluster is not ready after "
                    << PSContext::instance()->cluster_config().cluster_available_timeout
                    << " seconds,so finish the cluster, and change total node number from " << total_node_num_ << " to "
                    << nodes_info_.size();
    current_node_num_ = nodes_info_.size();
    UpdateClusterState(ClusterState::CLUSTER_TIMEOUT);
  }
}

void NodeManager::AddFinishNode(const std::string &finish_message) { finish_nodes_id_.insert(finish_message); }

void NodeManager::AddScaleOutDoneNode(const std::string &node_id) { scale_out_done_nodes_id_.insert(node_id); }

void NodeManager::AddScaleInDoneNode(const std::string &node_id) { scale_in_done_nodes_id_.insert(node_id); }

bool NodeManager::IsAllNodesRegistered() { return SizeToInt(nodes_info_.size()) == total_node_num_; }

bool NodeManager::IsAllNodesFinished() { return SizeToInt(finish_nodes_id_.size()) == total_node_num_; }

bool NodeManager::IsAllNodesScaleOutDone() { return SizeToInt(scale_out_done_nodes_id_.size()) == total_node_num_; }

bool NodeManager::IsAllNodesScaleInDone() { return SizeToInt(scale_in_done_nodes_id_.size()) == total_node_num_; }

std::unordered_map<std::string, NodeInfo> &NodeManager::nodes_info() { return nodes_info_; }

void NodeManager::UpdateNodeState(const NodeState &state) {
  std::lock_guard<std::mutex> lk(node_mutex_);
  node_state_ = state;
}

void NodeManager::UpdateClusterState(const ClusterState &state) {
  std::lock_guard<std::mutex> lk(cluster_mutex_);
  cluster_state_ = state;
}

NodeState NodeManager::GetNodeState() {
  std::lock_guard<std::mutex> lk(node_mutex_);
  return node_state_;
}

ClusterState NodeManager::GetClusterState() {
  std::lock_guard<std::mutex> lk(cluster_mutex_);
  return cluster_state_;
}

void NodeManager::ResetMetadata() {
  MS_LOG(WARNING) << "Reset metadata.";
  nodes_info_.clear();
  heartbeats_.clear();
  next_worker_rank_id_ = -1;
  next_server_rank_id_ = -1;
}

void NodeManager::set_total_node_num(const int32_t &node_num) { total_node_num_ = node_num; }

const int32_t &NodeManager::total_node_num() { return total_node_num_; }

void NodeManager::set_worker_num(const int32_t &worker_num) { meta_data_->worker_num = worker_num; }

void NodeManager::set_server_num(const int32_t &server_num) { meta_data_->server_num = server_num; }

int32_t NodeManager::worker_num() const { return UintToInt(meta_data_->worker_num); }

int32_t NodeManager::server_num() const { return UintToInt(meta_data_->server_num); }
}  // namespace core
}  // namespace ps
}  // namespace mindspore
