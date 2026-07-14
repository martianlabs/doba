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

#ifndef martianlabs_doba_protocol_http11_body_decoder_chunked_h
#define martianlabs_doba_protocol_http11_body_decoder_chunked_h

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>

#include "protocol/http11/body/decoder.h"

namespace martianlabs::doba::protocol::http11::body {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] decoder_chunked                                             ( class ) |
// +---------------------------------------------------------------------------+
// | This class represents the HTTP/1.1 chunked body decoder.                  |
// | It handles the decoding of chunked transfer encoding, managing the        |
// | state transitions and error handling.                                     |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class decoder_chunked {
  // +=========================================================================+
  // | [>] TYPEs                                                   ( private ) |
  // +=========================================================================+
  enum class state : std::uint8_t {
    reading_chunk_size,
    reading_chunk_extension,
    reading_chunk_size_lf,
    reading_chunk_data,
    reading_chunk_data_cr,
    reading_chunk_data_lf,
    reading_trailers,
    complete,
    error
  };

 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  decoder_chunked() = default;
  decoder_chunked(std::size_t max_chunk_extension_size,
                  std::size_t max_trailer_size)
      : max_chunk_extension_size_(max_chunk_extension_size),
        max_trailer_size_(max_trailer_size) {}
  // +=========================================================================+
  // | [>] decode                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  // | Decodes chunked transfer encoding from source into the output buffer.   |
  // +=========================================================================+
  template <typename RDty>
  decode_result decode(RDty& src, std::span<std::byte> output,
                       std::size_t& consumed) {
    decode_result result;
    if (state_ == state::complete) {
      result.complete = true;
      return result;
    }
    if (state_ == state::error) {
      result.has_error = true;
      result.error = error_;
      return result;
    }
    std::size_t output_position = 0;
    bool keep_going = true;
    while (keep_going && state_ != state::complete && state_ != state::error) {
      switch (state_) {
        case state::reading_chunk_size:
          keep_going = step_chunk_size(src, consumed);
          break;
        case state::reading_chunk_extension:
          keep_going = step_chunk_extension(src, consumed);
          break;
        case state::reading_chunk_size_lf:
          keep_going = step_chunk_size_lf(src, consumed);
          break;
        case state::reading_chunk_data:
          keep_going = step_chunk_data(src, consumed, output, output_position);
          break;
        case state::reading_chunk_data_cr:
          keep_going = step_chunk_data_cr(src, consumed);
          break;
        case state::reading_chunk_data_lf:
          keep_going = step_chunk_data_lf(src, consumed);
          break;
        case state::reading_trailers:
          keep_going = step_trailers(src, consumed);
          break;
        default:
          keep_going = false;
          break;
      }
    }
    if (state_ == state::error) {
      result.has_error = true;
      result.error = error_;
      return result;
    }
    result.produced = output_position;
    result.complete = state_ == state::complete;
    return result;
  }

 private:
  // +=========================================================================+
  // | [>] fail                                                     ( public ) |
  // +=========================================================================+
  void fail(decoder_error error) {
    error_ = error;
    state_ = state::error;
  }
  // +=========================================================================+
  // | [>] fetch_byte                                              ( private ) |
  // +=========================================================================+
  template <typename RDty>
  bool fetch_byte(RDty& source, std::size_t& source_consumed, std::byte& byte) {
    if (!source.fetch(byte)) {
      if (state_ != state::complete && state_ != state::error) {
        fail(source.ok() ? decoder_error::chunked_incomplete
                         : decoder_error::io_error);
      }
      return false;
    }
    ++source_consumed;
    return true;
  }
  // +=========================================================================+
  // | [>] step_chunk_size                                         ( private ) |
  // +=========================================================================+
  template <typename RDty>
  bool step_chunk_size(RDty& source, std::size_t& source_consumed) {
    std::byte byte;
    if (!fetch_byte(source, source_consumed, byte)) return false;
    char character = static_cast<char>(byte);
    if (character == ';') {
      extension_size_ = 0;
      state_ = state::reading_chunk_extension;
      return true;
    }
    if (character == '\r') {
      state_ = state::reading_chunk_size_lf;
      return true;
    }
    int digit = hex_digit(character);
    if (digit < 0) {
      fail(decoder_error::invalid_chunk_size);
      return false;
    }
    if (chunk_size_ > (std::numeric_limits<std::size_t>::max() -
                       static_cast<std::size_t>(digit)) /
                          16) {
      fail(decoder_error::chunk_size_overflow);
      return false;
    }
    chunk_size_ = chunk_size_ * 16 + static_cast<std::size_t>(digit);
    return true;
  }
  // +=========================================================================+
  // | [>] step_chunk_extension                                    ( private ) |
  // +=========================================================================+
  template <typename RDty>
  bool step_chunk_extension(RDty& source, std::size_t& source_consumed) {
    std::byte byte;
    if (!fetch_byte(source, source_consumed, byte)) return false;
    if (static_cast<char>(byte) == '\r') {
      state_ = state::reading_chunk_size_lf;
      return true;
    }
    extension_size_++;
    if (max_chunk_extension_size_ > 0 &&
        extension_size_ > max_chunk_extension_size_) {
      fail(decoder_error::chunk_extension_size_limit_exceeded);
      return false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] step_chunk_size_lf                                      ( private ) |
  // +=========================================================================+
  template <typename RDty>
  bool step_chunk_size_lf(RDty& source, std::size_t& source_consumed) {
    std::byte byte;
    if (!fetch_byte(source, source_consumed, byte)) return false;
    if (static_cast<char>(byte) != '\n') {
      fail(decoder_error::invalid_chunk_crlf);
      return false;
    }
    if (chunk_size_ == 0) {
      trailer_size_ = 0;
      trailer_eol_count_ = 1;
      trailer_cr_seen_ = false;
      state_ = state::reading_trailers;
    } else {
      chunk_remaining_ = chunk_size_;
      chunk_size_ = 0;
      state_ = state::reading_chunk_data;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] step_chunk_data                                         ( private ) |
  // +=========================================================================+
  template <typename RDty>
  bool step_chunk_data(RDty& source, std::size_t& source_consumed,
                       std::span<std::byte> output,
                       std::size_t& output_position) {
    std::size_t want =
        std::min(chunk_remaining_, output.size() - output_position);
    if (want == 0) return false;
    std::size_t bytes = source.read(output.subspan(output_position, want));
    if (bytes == 0) {
      fail(source.ok() ? decoder_error::chunked_incomplete
                       : decoder_error::io_error);
      return false;
    }
    source_consumed += bytes;
    output_position += bytes;
    chunk_remaining_ -= bytes;
    if (chunk_remaining_ == 0) state_ = state::reading_chunk_data_cr;
    return true;
  }
  // +=========================================================================+
  // | [>] step_chunk_data_cr                                      ( private ) |
  // +=========================================================================+
  template <typename RDty>
  bool step_chunk_data_cr(RDty& source, std::size_t& source_consumed) {
    std::byte byte;
    if (!fetch_byte(source, source_consumed, byte)) return false;
    if (static_cast<char>(byte) != '\r') {
      fail(decoder_error::invalid_chunk_crlf);
      return false;
    }
    state_ = state::reading_chunk_data_lf;
    return true;
  }
  // +=========================================================================+
  // | [>] step_chunk_data_lf                                      ( private ) |
  // +=========================================================================+
  template <typename RDty>
  bool step_chunk_data_lf(RDty& source, std::size_t& source_consumed) {
    std::byte byte;
    if (!fetch_byte(source, source_consumed, byte)) return false;
    if (static_cast<char>(byte) != '\n') {
      fail(decoder_error::invalid_chunk_crlf);
      return false;
    }
    chunk_size_ = 0;
    extension_size_ = 0;
    state_ = state::reading_chunk_size;
    return true;
  }
  // +=========================================================================+
  // | [>] step_trailers                                           ( private ) |
  // +=========================================================================+
  template <typename RDty>
  bool step_trailers(RDty& source, std::size_t& source_consumed) {
    std::byte byte;
    if (!fetch_byte(source, source_consumed, byte)) return false;
    char character = static_cast<char>(byte);
    trailer_size_++;
    if (max_trailer_size_ > 0 && trailer_size_ > max_trailer_size_) {
      fail(decoder_error::trailer_size_limit_exceeded);
      return false;
    }
    if (character == '\r') {
      trailer_cr_seen_ = true;
    } else if (character == '\n' && trailer_cr_seen_) {
      trailer_cr_seen_ = false;
      if (++trailer_eol_count_ >= 2) state_ = state::complete;
    } else {
      trailer_cr_seen_ = false;
      trailer_eol_count_ = 0;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] hex_digit                                               ( private ) |
  // +=========================================================================+
  static int hex_digit(char character) {
    if (character >= '0' && character <= '9') return character - '0';
    if (character >= 'a' && character <= 'f') return character - 'a' + 10;
    if (character >= 'A' && character <= 'F') return character - 'A' + 10;
    return -1;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  std::size_t max_chunk_extension_size_{0};
  std::size_t max_trailer_size_{0};
  decoder_error error_{decoder_error::none};
  state state_{state::reading_chunk_size};
  std::size_t chunk_size_{0};
  std::size_t chunk_remaining_{0};
  std::size_t extension_size_{0};
  std::size_t trailer_size_{0};
  bool trailer_cr_seen_{false};
  int trailer_eol_count_{0};
};
}  // namespace martianlabs::doba::protocol::http11::body

#endif
