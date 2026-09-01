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

#include <cstddef>
#include <span>
#include <string>
#include <string_view>

#include "common/reader.h"
#include "common/writer.h"
#include "protocol/http/v11/body/writer_raw.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::reader;
using martianlabs::doba::common::writer;
using martianlabs::doba::protocol::http::v11::body::writer_raw;

std::string release(writer& value) {
  reader source(value.release());
  std::string output;
  source.read_all(output);
  return output;
}
}  // namespace

// +===========================================================================+
// | [>] writes string and byte spans without framing            ( test-case ) |
// +===========================================================================+
DOBA_TEST("writes string and byte spans without framing") {
  writer destination;
  DOBA_EXPECT(writer_raw::write(std::string_view("ab\0c", 4), destination));
  const std::byte bytes[] = {std::byte{'d'}, std::byte{0xff}};
  DOBA_EXPECT(writer_raw::write(bytes, destination));
  DOBA_EXPECT(writer_raw::end(destination));
  const std::string output = release(destination);
  DOBA_EXPECT_EQUAL(output.size(), 6);
  DOBA_EXPECT_EQUAL(std::string_view(output.data(), 4),
                    std::string_view("ab\0c", 4));
  DOBA_EXPECT_EQUAL(static_cast<unsigned char>(output[4]), 'd');
  DOBA_EXPECT_EQUAL(static_cast<unsigned char>(output[5]), 0xff);
}
// +===========================================================================+
// | [>] empty buffers and repeated end calls write nothing      ( test-case ) |
// +===========================================================================+
DOBA_TEST("empty buffers and repeated end calls write nothing") {
  writer destination;
  DOBA_EXPECT(writer_raw::write(std::string_view{}, destination));
  DOBA_EXPECT(writer_raw::write(std::span<const std::byte>{}, destination));
  DOBA_EXPECT(writer_raw::end(destination));
  DOBA_EXPECT(writer_raw::end(destination));
  DOBA_EXPECT(release(destination).empty());
}
