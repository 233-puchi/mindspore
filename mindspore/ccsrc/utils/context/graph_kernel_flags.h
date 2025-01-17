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

#ifndef MINDSPORE_CCSRC_UTILS_GRAPH_KERNEL_FLAGS_H
#define MINDSPORE_CCSRC_UTILS_GRAPH_KERNEL_FLAGS_H

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include "utils/ms_context.h"

namespace mindspore {
namespace context {
class GraphKernelFlags {
 public:
  static const GraphKernelFlags &GetInstance() {
    static std::unique_ptr<GraphKernelFlags> flags(nullptr);
    auto contexts = GetGraphKernelContext();
    if (flags == nullptr || contexts.first != flags->flags_cache_ || contexts.second != flags->enable_cache_) {
      flags.reset(new GraphKernelFlags(contexts.first, contexts.second));
      flags->Refresh();
    }
    return *flags;
  }

  // Dump all flags to json-format string
  std::string DumpAllFlags() const;

  // Check whether graph_kernel is enabled
  bool IsEnableGraphKernel() const { return opt_level > 0; }

  GraphKernelFlags(const GraphKernelFlags &flags) = delete;
  ~GraphKernelFlags() = default;

 public:
  /**
   * Dump info as human-readable text.
   * A directory "graph_kernel_dump" will be created, and all information will be dumped in this directory.
   */
  bool dump_as_text{false};

  /**
   * Enable stitch fusion in graph kernel fusion strategy.
   */
  bool enable_stitch_fusion{false};

  /**
   * Enable parallel fusion in graph kernel fusion strategy.
   */
  bool enable_parallel_fusion{false};

  /**
   * Optimization level, value from 0 to 3.
   * 0: Disable GraphKernel
   * 1: Enable GraphKernel with basic features only.
   * 2: Enable GraphKernel with all stable features.
   * 3: Enable GraphKernel with all experimental features.
   * The default value is level 2 when the context "enable_graph_kernel" is set,
   * but if it's also changed in "graph_kernel_flags", then the "graph_kernel_flags" will prevail.
   */
  unsigned int opt_level;  // defaults 0 or 2

  /**
   * auto_tune, unsupported now.
   */
  unsigned int auto_tune{0};

  /**
   * cluster_limit, unsupported now.
   */
  unsigned int cluster_limit{0};

  /**
   * Additional expanding operators (case sensitive).
   * The operators to be added into the default expanding operator list.
   */
  std::vector<std::string> enable_expand_ops;

  /**
   * Expanding operators to be enabled (case sensitive).
   * Unlike the "enable_expand_ops", the default list will be overwritten by this list.
   * Note that the "enable_expand_ops" and "disable_expand_ops" will be ignored if this flag is set.
   */
  std::vector<std::string> enable_expand_ops_only;

  /**
   * Expanding operators to be disabled (case sensitive).
   * The behavior is undefined when this list overlaps with "enable_expand_ops".
   */
  std::vector<std::string> disable_expand_ops;

  /**
   * Additional clustering operators (case sensitive).
   * The operators to be added into the default clustering operator list.
   */
  std::vector<std::string> enable_cluster_ops;

  /**
   * Clustering operators to be enabled (case sensitive).
   * Unlike the "enable_cluster_ops", the default list will be overwritten by this list.
   * Note that the "enable_cluster_ops" and "disable_cluster_ops" will be ignored if this flag is set.
   */
  std::vector<std::string> enable_cluster_ops_only;

  /**
   * Clustering operators to be disabled (case sensitive).
   * The behavior is undefined when this list overlaps with "enable_cluster_ops".
   */
  std::vector<std::string> disable_cluster_ops;

  /**
   * Passes to be enabled.
   * By default, the passes is controlled by "opt_level" and target device,
   * user can manually enable some passes by setting this flag.
   * The format is "stage_id.pass_id" or "stage_name.pass_name", which corresponds to the ir filename.
   */
  std::vector<std::string> enable_pass;

  /**
   * Passes to be disabled.
   * By default, the passes is controlled by "opt_level" and target device,
   * user can manually disable some passes by setting this flag.
   * The format is "stage_id.pass_id" or "stage_name.pass_name", which corresponds to the ir filename.
   */
  std::vector<std::string> disable_pass;

 private:
  GraphKernelFlags(const std::string &graph_kernel_flags, bool enable_graph_kernel)
      : flags_cache_(graph_kernel_flags), enable_cache_(enable_graph_kernel) {
    // Default optimization level is level 2 when enable graphkernel
    opt_level = enable_graph_kernel ? 2 : 0;
  }

  // get the `graph_kernel_flags` and `enable_graph_kernel`
  static std::pair<std::string, bool> GetGraphKernelContext() {
    auto context = MsContext::GetInstance();
    MS_EXCEPTION_IF_NULL(context);
    // Use the environment variable in priority
    auto env_flags = std::getenv("MS_GRAPH_KERNEL_FLAGS");
    std::string flags = env_flags ? std::string(env_flags) : context->get_param<std::string>(MS_CTX_GRAPH_KERNEL_FLAGS);
    return std::make_pair(flags, context->get_param<bool>(MS_CTX_ENABLE_GRAPH_KERNEL));
  }

  // parse and refresh the flags
  void Refresh();
  // register the flags defined above
  void RegisterFlags(std::map<std::string, std::string> *flag_map);

  // cache the flag string to check whether the flags is changed.
  std::string flags_cache_;
  // cache the enable_graph_kernel value to check whether the context is changed.
  bool enable_cache_;
};
}  // namespace context
}  // namespace mindspore
#endif  // MINDSPORE_CCSRC_UTILS_GRAPH_KERNEL_FLAGS_H
