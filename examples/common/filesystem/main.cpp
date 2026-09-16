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

#include <array>
#include <cstddef>
#include <iostream>
#include <utility>

#include "common/filesystem.h"
#include "common/reader.h"

using namespace martianlabs::doba::common;

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: common_filesystem <root> <relative-file>\n";
    return 1;
  }
  std::error_code error;
  const auto root = filesystem_root(argv[1], error);
  filesystem_file file;
  if (error || !file.open(root, argv[2], error)) {
    std::cerr << error.message() << '\n';
    return 1;
  }
  reader input(std::move(file));
  std::array<std::byte, 4096> block{};
  for (;;) {
    const auto count = input.read(block);
    if (!count) break;
    std::cout.write(reinterpret_cast<const char*>(block.data()),
                    static_cast<std::streamsize>(count));
  }
  return input.ok() && input.eof() && std::cout ? 0 : 1;
}
