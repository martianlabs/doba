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

#ifndef martianlabs_doba_tests_helpers_tool_sha256_h
#define martianlabs_doba_tests_helpers_tool_sha256_h

#include "tool_json.h"

#include <array>
#include <bit>
#include <cstdint>

namespace martianlabs::doba::tests::tools {

// +---------------------------------------------------------------------------+
// | [>] sha256                                                   ( function ) |
// +---------------------------------------------------------------------------+
// FIPS 180-4 digest for evidence identity, never credential authentication.
// Tests include the standard empty and abc known-answer vectors.
inline std::string sha256(std::string input) {
  constexpr std::array<uint32_t, 64> constants{
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b,
    0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01,
    0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7,
    0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152,
    0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
    0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc,
    0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819,
    0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08,
    0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f,
    0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
  std::array<uint32_t, 8> hash{0x6a09e667, 0xbb67ae85, 0x3c6ef372,
    0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
  uint64_t bits = static_cast<uint64_t>(input.size()) * 8;
  input += static_cast<char>(128);
  while (input.size() % 64 != 56) input += '\0';
  for (int shift = 56; shift >= 0; shift -= 8) {
    input += static_cast<char>(bits >> shift);
  }
  for (size_t offset = 0; offset < input.size(); offset += 64) {
    std::array<uint32_t, 64> words{};
    for (size_t index = 0; index < 16; ++index) {
      for (size_t byte = 0; byte < 4; ++byte) {
        words[index] = (words[index] << 8) |
          static_cast<unsigned char>(input[offset + index * 4 + byte]);
      }
    }
    for (size_t index = 16; index < 64; ++index) {
      auto x = words[index - 15], y = words[index - 2];
      auto a = std::rotr(x, 7) ^ std::rotr(x, 18) ^ (x >> 3);
      auto b = std::rotr(y, 17) ^ std::rotr(y, 19) ^ (y >> 10);
      words[index] = words[index - 16] + a + words[index - 7] + b;
    }
    auto state = hash;
    for (size_t index = 0; index < 64; ++index) {
      auto a = std::rotr(state[4], 6) ^ std::rotr(state[4], 11) ^
        std::rotr(state[4], 25);
      auto choose = (state[4] & state[5]) ^ (~state[4] & state[6]);
      auto first = state[7] + a + choose + constants[index] + words[index];
      auto b = std::rotr(state[0], 2) ^ std::rotr(state[0], 13) ^
        std::rotr(state[0], 22);
      auto majority = (state[0] & state[1]) ^ (state[0] & state[2]) ^
        (state[1] & state[2]);
      for (size_t slot = 7; slot > 0; --slot) state[slot] = state[slot - 1];
      state[4] += first;
      state[0] = first + b + majority;
    }
    for (size_t index = 0; index < 8; ++index) hash[index] += state[index];
  }
  std::ostringstream output;
  for (auto word : hash) output << std::hex << std::setfill('0') <<
    std::setw(8) << word;
  return output.str();
}

inline json source_hashes(const fs::path& root,
                         std::initializer_list<const char*> directories) {
  json result(json::object{});
  for (auto directory : directories) {
    auto entries = fs::recursive_directory_iterator(root / directory);
    for (const auto& entry : entries) {
      if (!entry.is_regular_file()) continue;
      auto name = entry.path().lexically_relative(root).generic_string();
      result[name] = sha256(read(entry.path()));
    }
  }
  return result;
}

} // namespace martianlabs::doba::tests::tools

#endif
