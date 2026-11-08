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
// Copyright 2026 martianLabs
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

#ifndef martianlabs_doba_tests_qualification_profile_identity_h
#define martianlabs_doba_tests_qualification_profile_identity_h

#include <cstdlib>
#include <cstdio>
#include <cstring>

extern "C" int __llvm_profile_write_file();
extern "C" void __llvm_profile_set_filename(const char*);
extern "C" void __llvm_profile_reset_counters();

namespace martianlabs::doba::tests::qualification {
#if defined(DOBA_PROFILE_BINARY)
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] profile_identity                                         ( constant ) |
// +---------------------------------------------------------------------------+
// | Coverage-only probe: split profiles by target, subprocesses included.     |
// | Reset counters and exit before main for an explicit zero denominator.     |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
inline const bool profile_identity = [] {
#if defined(_WIN32)
  char* pattern = nullptr;
  std::size_t length = 0;
  if (_dupenv_s(&pattern, &length, "LLVM_PROFILE_FILE")) std::_Exit(3);
  if (!pattern) return false;
#else
  const char* pattern = std::getenv("LLVM_PROFILE_FILE");
  if (!pattern) return false;
#endif
  // Keep the profile directory. Children replace the parent's target prefix
  // instead of piling prefixes up. Keep this storage alive through profile
  // flush.
  const char* separator = std::strrchr(pattern, '/');
  if (!separator) separator = std::strrchr(pattern, '\\');
  if (!separator) std::_Exit(3);
  static char filename[8192];
  const auto size = std::snprintf(filename, sizeof(filename),
      "%.*s%s-%%p-%%m.profraw", int(separator - pattern + 1), pattern,
      DOBA_PROFILE_BINARY);
  if (size < 0 || std::size_t(size) >= sizeof(filename)) std::_Exit(3);
#if defined(_WIN32)
  std::free(pattern);
  if (_putenv_s("LLVM_PROFILE_FILE", filename)) std::_Exit(3);
  char* identify = nullptr;
  if (_dupenv_s(&identify, &length, "DOBA_COVERAGE_IDENTIFY")) std::_Exit(3);
  const bool probe = identify != nullptr;
  std::free(identify);
#else
  if (::setenv("LLVM_PROFILE_FILE", filename, 1)) std::_Exit(3);
  const bool probe = std::getenv("DOBA_COVERAGE_IDENTIFY") != nullptr;
#endif
  __llvm_profile_set_filename(filename);
  if (!probe) return false;
  __llvm_profile_reset_counters();
  std::_Exit(__llvm_profile_write_file() == 0 ? 0 : 3);
}();
#endif
}  // namespace martianlabs::doba::tests::qualification

#endif
