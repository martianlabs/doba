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

#ifndef martianlabs_doba_tests_fuzz_fuzz_helpers_h
#define martianlabs_doba_tests_fuzz_fuzz_helpers_h

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <span>
#include <string>
#include <string_view>

namespace doba_fuzz {
// +---------------------------------------------------------------------------+
// | [>] check                                                    ( function ) |
// +---------------------------------------------------------------------------+
inline void check(bool condition, const char* property) {
  // assert() is disabled in Release. Properties must remain executable.
  if (!condition) {
    std::fprintf(stderr, "doba fuzz property failed: %s\n", property);
    std::abort();
  }
}
// +---------------------------------------------------------------------------+
// | [>] bytes                                                    ( function ) |
// +---------------------------------------------------------------------------+
inline auto bytes(std::string_view wire) {
  return std::as_bytes(std::span(wire.data(), wire.size()));
}
// +---------------------------------------------------------------------------+
// | [>] fragment_size                                            ( function ) |
// +---------------------------------------------------------------------------+
inline std::size_t fragment_size(std::string_view wire, std::size_t pos) {
  return std::min(
      wire.size() - pos,
      std::size_t{1} + static_cast<unsigned char>(wire[pos]) % std::size_t{31});
}
// +---------------------------------------------------------------------------+
// | [>] chunked_wire                                             ( function ) |
// +---------------------------------------------------------------------------+
inline std::string chunked_wire(std::string_view payload) {
  if (payload.empty()) return "0\r\n\r\n";
  std::array<char, 32> size_line{};
  std::snprintf(size_line.data(), size_line.size(), "%zx", payload.size());
  return std::string(size_line.data()) + "\r\n" + std::string(payload) +
         "\r\n0\r\n\r\n";
}
}  // namespace doba_fuzz

#endif
