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
#include <cstdint>
#include <string>

#include "fuzz_helpers.h"
#include "protocol/http/v11/body/framer_chunked.h"
#include "protocol/http/v11/body/reader_chunked.h"

namespace {
// +---------------------------------------------------------------------------+
// | [>] general                                                    ( usings ) |
// +---------------------------------------------------------------------------+
namespace body = martianlabs::doba::protocol::http::v11::body;
using doba_fuzz::check;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] framed                                                     ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct framed {
  body::framer_state state;
  std::size_t consumed = 0;
};

// +---------------------------------------------------------------------------+
// | [>] frame                                                    ( function ) |
// +---------------------------------------------------------------------------+
framed frame(std::string_view wire, bool fragmented, std::size_t limit,
             std::size_t extensions, std::size_t trailers) {
  body::framer_chunked parser(limit, extensions, trailers);
  framed result;
  while (result.consumed < wire.size()) {
    const auto count = fragmented
                           ? doba_fuzz::fragment_size(wire, result.consumed)
                           : wire.size() - result.consumed;
    result.state =
        parser.consume(doba_fuzz::bytes(wire.substr(result.consumed, count)));
    check(result.state.consumed <= count, "chunked consumption within input");
    result.consumed += result.state.consumed;
    if (result.state.complete || result.state.has_error) {
      const auto sticky = parser.consume(doba_fuzz::bytes(wire));
      check(sticky.consumed == 0 && sticky.complete == result.state.complete &&
                sticky.has_error == result.state.has_error &&
                sticky.error == result.state.error,
            "chunked terminal state is sticky");
      break;
    }
    check(result.state.consumed == count,
          "chunked incomplete consumes fragment");
    const auto empty = parser.consume({});
    check(empty.consumed == 0 && !empty.has_error && !empty.complete,
          "empty chunked fragment preserves incomplete state");
  }
  return result;
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] decoded                                                    ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct decoded {
  std::string payload;
  std::string suffix;
  bool complete = false;
  bool error = false;
};

// +---------------------------------------------------------------------------+
// | [>] read                                                     ( function ) |
// +---------------------------------------------------------------------------+
decoded read(std::string_view wire, std::size_t capacity,
             std::size_t extensions, std::size_t trailers) {
  auto source =
      martianlabs::doba::common::reader::borrowed(doba_fuzz::bytes(wire));
  body::reader_chunked parser(extensions, trailers);
  std::array<std::byte, 97> output{};
  decoded result;
  for (std::size_t step = 0; step <= wire.size() + 1; ++step) {
    const auto state = parser.read(source, std::span(output).first(capacity));
    check(state.produced <= capacity, "chunked reader output within capacity");
    result.payload.append(reinterpret_cast<const char*>(output.data()),
                          state.produced);
    check(result.payload.size() <= wire.size(),
          "chunked payload bounded by wire size");
    result.complete = state.complete;
    result.error = state.has_error;
    if (state.complete || state.has_error || state.produced == 0) break;
  }
  while (const auto count = source.read(output)) {
    result.suffix.append(reinterpret_cast<const char*>(output.data()), count);
  }
  return result;
}
}  // namespace

// +---------------------------------------------------------------------------+
// | [>] LLVMFuzzerTestOneInput                                   ( function ) |
// +---------------------------------------------------------------------------+
extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data,
                                      std::size_t size) {
  if (size > 16384) return 0;
  const std::string_view wire(reinterpret_cast<const char*>(data), size);
  const std::size_t extensions = size % 2 ? 16 : 1024;
  const std::size_t trailers = size % 3 ? 64 : 4096;
  const auto whole = frame(wire, false, 4096, extensions, trailers);
  const auto split = frame(wire, true, 4096, extensions, trailers);
  check(whole.state.complete == split.state.complete &&
            whole.state.has_error == split.state.has_error &&
            whole.state.error == split.state.error,
        "chunked fragmentation equivalence");
  if (whole.state.complete) {
    check(whole.consumed == split.consumed,
          "chunked message boundary equivalence");
  }
  const auto wide = read(wire, 97, extensions, trailers);
  const auto narrow = read(wire, 1 + size % 31, extensions, trailers);
  check(wide.complete == narrow.complete && wide.error == narrow.error &&
            wide.payload == narrow.payload && wide.suffix == narrow.suffix,
        "chunked output capacity equivalence");
  if (whole.state.complete) {
    check(wide.complete && !wide.error, "reader accepts framed chunked body");
    check(wide.suffix == wire.substr(whole.consumed),
          "chunked reader preserves successor");
  }
  const auto payload = wire.substr(0, 512);
  const auto encoded = doba_fuzz::chunked_wire(payload);
  const std::string successor = "GET /next HTTP/1.1\r\nHost: x\r\n\r\n";
  const auto valid = encoded + successor;
  const auto known_frame = frame(valid, true, 4096, extensions, trailers);
  check(known_frame.state.complete && !known_frame.state.has_error &&
            known_frame.consumed == encoded.size(),
        "valid chunked boundary");
  const auto known_read = read(valid, 1 + size % 31, extensions, trailers);
  check(known_read.complete && !known_read.error &&
            known_read.payload == payload && known_read.suffix == successor,
        "valid chunked payload and successor");
  return 0;
}
