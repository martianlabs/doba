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

#ifndef martianlabs_doba_tests_resilience_resources_windows_h
#define martianlabs_doba_tests_resilience_resources_windows_h

#include "platform.h"
#include <psapi.h>

namespace martianlabs::doba::tests::integration {
// +---------------------------------------------------------------------------+
// | [>] measure_resilience_resources                             ( function ) |
// +---------------------------------------------------------------------------+
inline resilience_resources measure_resilience_resources() {
  resilience_resources result;
  DWORD handles = 0;
  PROCESS_MEMORY_COUNTERS_EX memory{};
  if (!GetProcessHandleCount(GetCurrentProcess(), &handles) ||
      !K32GetProcessMemoryInfo(
          GetCurrentProcess(),
          reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&memory), sizeof(memory)))
    throw std::runtime_error("Cannot measure process resources");
  result.handles = handles;
  result.memory_bytes = memory.PrivateUsage;
  return result;
}
// +---------------------------------------------------------------------------+
// | [>] resilience_process_id                                    ( function ) |
// +---------------------------------------------------------------------------+
inline auto resilience_process_id() {
  return GetCurrentProcessId();
}
// +---------------------------------------------------------------------------+
// | [>] resilience_temp_variable                                 ( function ) |
// +---------------------------------------------------------------------------+
inline const char* resilience_temp_variable() {
  return "TMP";
}
// +---------------------------------------------------------------------------+
// | [>] set_resilience_temp_variable                             ( function ) |
// +---------------------------------------------------------------------------+
inline void set_resilience_temp_variable(const char* value) {
  _putenv_s(resilience_temp_variable(), value ? value : "");
}
}  // namespace martianlabs::doba::tests::integration

#endif
