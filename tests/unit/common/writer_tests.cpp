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

#include <string>

#include "common/reader.h"
#include "common/writer.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::reader;
using martianlabs::doba::common::writer;
}  // namespace

// +===========================================================================+
// | [>] finishing seals the writer                              ( test-case ) |
// +===========================================================================+
DOBA_TEST("finishing seals the writer") {
  writer value;
  DOBA_EXPECT(value.write("body"));
  value.finish(4);
  DOBA_EXPECT(!value.write("x"));
  DOBA_EXPECT(!value.write(""));
  value.finish(1);
  reader source(value.release());
  DOBA_EXPECT_EQUAL(source.size().value(), 4);
  std::string output;
  DOBA_EXPECT_EQUAL(source.read_all(output), 4);
  DOBA_EXPECT_EQUAL(output, "body");
}
