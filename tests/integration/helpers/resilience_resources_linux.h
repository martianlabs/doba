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

#ifndef martianlabs_doba_tests_integration_helpers_resilience_resources_linux_h
#define martianlabs_doba_tests_integration_helpers_resilience_resources_linux_h

#include <fstream>

#include "platform.h"

namespace martianlabs::doba::tests::integration {
// +---------------------------------------------------------------------------+
// | [>] measure_resilience_resources                             ( function ) |
// +---------------------------------------------------------------------------+
inline resilience_resources measure_resilience_resources() {
  resilience_resources result;
  for ([[maybe_unused]] const auto& entry :
       std::filesystem::directory_iterator("/proc/self/fd"))
    ++result.handles;
  std::ifstream statm("/proc/self/statm");
  std::size_t virtual_pages = 0, resident_pages = 0;
  if (!(statm >> virtual_pages >> resident_pages))
    throw std::runtime_error("Cannot measure resident memory");
  result.memory_bytes =
      resident_pages * static_cast<std::size_t>(::sysconf(_SC_PAGESIZE));
  return result;
}
// +---------------------------------------------------------------------------+
// | [>] resilience_process_id                                    ( function ) |
// +---------------------------------------------------------------------------+
inline auto resilience_process_id() {
  return ::getpid();
}
// +---------------------------------------------------------------------------+
// | [>] resilience_temp_variable                                 ( function ) |
// +---------------------------------------------------------------------------+
inline const char* resilience_temp_variable() {
  return "TMPDIR";
}
// +---------------------------------------------------------------------------+
// | [>] set_resilience_temp_variable                             ( function ) |
// +---------------------------------------------------------------------------+
inline void set_resilience_temp_variable(const char* value) {
  if (value)
    ::setenv(resilience_temp_variable(), value, 1);
  else
    ::unsetenv(resilience_temp_variable());
}
}  // namespace martianlabs::doba::tests::integration

#endif
