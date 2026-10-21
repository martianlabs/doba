//                              _       _
//                           __| | ___ | |__   __ _
//                          / _` |/ _ \| '_ \ / _` |
//                         | (_| | (_) | |_) | (_| |
//                          \__,_|\___/|_.__/ \__,_|
//
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/LICENSE-2.0
//
// Copyright 2025 martianLabs
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied. See the License for the specific language governing
// permissions and limitations under the License.

#ifndef martianlabs_doba_tests_integration_helpers_resilience_resources_h
#define martianlabs_doba_tests_integration_helpers_resilience_resources_h

#include <atomic>
#include <cstdlib>
#include <filesystem>
#include <stdexcept>
#include <string>

#include "../../helpers/platform.h"

namespace martianlabs::doba::tests::integration {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] resilience_resources                                       ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct resilience_resources {
  std::size_t handles = 0;
  std::size_t memory_bytes = 0;
};

}  // namespace martianlabs::doba::tests::integration

#include DOBA_TEST_PLATFORM_HEADER(resilience_resources)

namespace martianlabs::doba::tests::integration {
// Tests execute sequentially in this runner. Change the environment only before
// starting server/client threads and restore it after their destruction.
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] resilience_temp_directory                                   ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class resilience_temp_directory {
 public:
  // +-------------------------------------------------------------------------+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  resilience_temp_directory() {
    const auto* old = std::getenv(variable());
    present_ = old != nullptr;
    if (old) previous_ = old;
    static std::atomic<unsigned> sequence{0};
    const auto pid = resilience_process_id();
    path = std::filesystem::temp_directory_path() /
           ("doba_resilience_" + std::to_string(pid) + "_" +
            std::to_string(sequence++));
    if (!std::filesystem::create_directory(path))
      throw std::runtime_error(
          "Cannot create isolated resilience temporary directory");
    set(path.string().c_str());
  }
  ~resilience_temp_directory() {
    set(present_ ? previous_.c_str() : nullptr);
    // A leaked file is evidence: leave a nonempty directory for inspection.
    std::error_code ignored;
    std::filesystem::remove(path, ignored);
  }
  resilience_temp_directory(const resilience_temp_directory&) = delete;
  resilience_temp_directory& operator=(const resilience_temp_directory&) =
      delete;
  // +-------------------------------------------------------------------------+
  // | [>] empty                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  bool empty() const { return std::filesystem::is_empty(path); }
  std::filesystem::path path;

 private:
  // +-------------------------------------------------------------------------+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  // +-------------------------------------------------------------------------+
  // | [>] variable                                                ( private ) |
  // +-------------------------------------------------------------------------+
  static const char* variable() { return resilience_temp_variable(); }
  // +-------------------------------------------------------------------------+
  // | [>] set                                                     ( private ) |
  // +-------------------------------------------------------------------------+
  static void set(const char* value) { set_resilience_temp_variable(value); }
  bool present_ = false;
  std::string previous_;
};
}  // namespace martianlabs::doba::tests::integration

#endif
