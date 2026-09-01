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

#include <cstdint>
#include <type_traits>

#include "protocol/http/v11/body/writer_error.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::body::writer_error;
}  // namespace

// +===========================================================================+
// | [>] error values preserve the public contract               ( test-case ) |
// +===========================================================================+
DOBA_TEST("error values preserve the public contract") {
  static_assert(
      std::same_as<std::underlying_type_t<writer_error>, std::uint8_t>);
  constexpr writer_error values[] = {
      writer_error::none,
      writer_error::io_error,
      writer_error::invalid_chunk_size,
      writer_error::chunk_size_overflow,
      writer_error::invalid_chunk_crlf,
      writer_error::invalid_trailer,
      writer_error::chunked_incomplete,
      writer_error::raw_size_limit_exceeded,
      writer_error::chunk_extension_size_limit_exceeded,
      writer_error::trailer_size_limit_exceeded,
  };
  for (std::size_t i = 0; i < std::size(values); i++) {
    DOBA_EXPECT_EQUAL(static_cast<std::size_t>(values[i]), i);
  }
}
