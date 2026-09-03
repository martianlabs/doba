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

#include <iostream>

#include "common/byte_storage.h"

using namespace martianlabs::doba::common;

int main() {
  byte_storage storage({.spill_threshold = 4, .spill_dir = {}});
  if (!storage.write("doba", 4) || !storage.write(" storage", 8)) return 1;
  storage.finish(12);
  char output[12]{};
  if (storage.read(output, sizeof(output)) != sizeof(output)) return 1;
  std::cout.write(output, sizeof(output));
  std::cout << '\n';
  return 0;
}

