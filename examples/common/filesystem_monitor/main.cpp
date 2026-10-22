//                              _       _
//                           __| | ___ | |__   __ _
//                          / _` |/ _ \| '_ \ / _` |
//                         | (_| | (_) | |_) | (_| |
//                          \__,_|\___/|_.__/ \__,_|
//
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/
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

#include <charconv>
#include <chrono>
#include <iostream>
#include <string_view>
#include <thread>

#include "common/filesystem.h"

using martianlabs::doba::common::filesystem_monitor;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// | Poll native changes on the calling thread; no callback thread is created. |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
  if (argc < 3 || argc > 4) {
    std::cerr << "Usage: common_filesystem_monitor <root> <relative-file> [poll-count]\n";
    return 1;
  }
  unsigned count = 100;
  if (argc == 4) {
    const std::string_view text(argv[3]);
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), count);
    if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size() || !count || count > 100000) {
      std::cerr << "Invalid poll count\n";
      return 1;
    }
  }
  filesystem_monitor monitor(argv[1]);
  std::error_code error;
  auto version = monitor.observe(argv[2], error);
  if (!version) { std::cerr << error.message() << '\n'; return 1; }
  std::cout << "generation: " << version << std::endl;
  for (unsigned i = 0; i < count; ++i) {
    const auto current = monitor.poll(error);
    if (current != version) {
      std::cout << "change or lost coverage; previous generation: " << version << std::endl;
      version = monitor.observe(argv[2], error);
      if (!version) std::cerr << "revalidation required: " << error.message() << '\n';
      else std::cout << "generation: " << version << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  return std::cout ? 0 : 1;
}
