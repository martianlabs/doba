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

#ifndef martianlabs_doba_protocol_http_v11_body_framer_chunked_h
#define martianlabs_doba_protocol_http_v11_body_framer_chunked_h

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>

#include "common/writer.h"
#include "protocol/http/common/helpers.h"
#include "protocol/http/v11/body/framer_state.h"
#include "protocol/http/v11/limits.h"

namespace martianlabs::doba::protocol::http::v11::body {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] framer_chunked                                              ( class ) |
// +---------------------------------------------------------------------------+
// | Validates and accumulates a chunked Transfer-Encoding body into a         |
// | common::writer.                                                           |
// |                                                                           |
// | The caller pushes incoming transport spans via write(). Each call         |
// | validates the chunked framing and writes ALL wire bytes - including       |
// | chunk-size lines, extensions, trailers and terminating CRLF - verbatim    |
// | into dst. No decoding is performed here; decoding is deferred to a        |
// | reader pass over the completed buffer.                                    |
// |                                                                           |
// | framer_state::consumed reports the exact number of bytes belonging to     |
// | this body that were taken from the input span. Any remaining bytes in the |
// | span belong to the next request.                                          |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class framer_chunked {
  // +=========================================================================+
  // | [>] TYPEs                                                   ( private ) |
  // +=========================================================================+
  enum class state : std::uint8_t {
    chunk_size,
    extension_before_semicolon,
    extension_before_name,
    extension_name,
    extension_after_name,
    extension_before_value,
    extension_token_value,
    extension_quoted_value,
    extension_quoted_pair,
    extension_after_value,
    size_lf,
    data,
    data_cr,
    data_lf,
    trailer_line_start,
    trailer_name,
    trailer_value,
    trailer_line_lf,
    trailer_end_lf,
    complete,
    error
  };

 public:
  // +=========================================================================+
  // | [>] CONSTANTs                                                ( public ) |
  // +=========================================================================+
  // +=========================================================================+
  // | [>] CONSTRUCTORs                                             ( public ) |
  // +=========================================================================+
  framer_chunked() = default;
  // +=========================================================================+
  // | [>] write                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // | Validates the chunked framing in input and writes every wire byte into  |
  // | dst unchanged. Returns the number of bytes consumed from input and      |
  // | whether the body is complete (last-chunk + terminating CRLF seen).      |
  // +=========================================================================+
  framer_state write(std::span<const std::byte> input, common::writer& dst) {
    framer_state result;
    if (state_ == state::complete) {
      result.complete = true;
      return result;
    }
    if (state_ == state::error) {
      result.has_error = true;
      result.error = error_;
      return result;
    }
    std::size_t i = 0;
    while (i < input.size()) {
      const char c = static_cast<char>(input[i]);
      if (state_ >= state::extension_before_semicolon &&
          state_ <= state::extension_after_value &&
          ++extension_size_ > limits::kMaxChunkedExtensionSize) {
        return fail(result, framer_error::chunk_extension_size_limit_exceeded);
      }
      if (state_ >= state::trailer_line_start &&
          state_ <= state::trailer_end_lf &&
          ++trailer_size_ > limits::kMaxChunkedTrailerSize) {
        return fail(result, framer_error::trailer_size_limit_exceeded);
      }
      switch (state_) {
        // ---------------------------------------------------------------------
        // Chunk-size line: accumulate hex digits, handle extension and CR
        // ---------------------------------------------------------------------
        case state::chunk_size: {
          if (c == ';') {
            if (!chunk_size_started_) {
              return fail(result, framer_error::invalid_chunk_size);
            }
            state_ = state::extension_before_name;
            break;
          }
          if (c == '\r') {
            if (!chunk_size_started_) {
              return fail(result, framer_error::invalid_chunk_size);
            }
            state_ = state::size_lf;
            break;
          }
          if ((c == ' ' || c == '\t') && chunk_size_started_) {
            state_ = state::extension_before_semicolon;
            break;
          }
          int digit = hex_digit(c);
          if (digit < 0) {
            // Invalid hex digit in chunk-size line!
            return fail(result, framer_error::invalid_chunk_size);
          }
          if (chunk_remaining_ >
              (std::numeric_limits<std::size_t>::max() >> 4)) {
            // Overflow in chunk size accumulation!
            return fail(result, framer_error::chunk_size_overflow);
          }
          chunk_remaining_ =
              (chunk_remaining_ << 4) | static_cast<std::size_t>(digit);
          chunk_size_started_ = true;
          break;
        }
        // ---------------------------------------------------------------------
        // Chunk-extension
        // ---------------------------------------------------------------------
        case state::extension_before_semicolon: {
          if (c == ' ' || c == '\t') break;
          if (c == ';') {
            state_ = state::extension_before_name;
            break;
          }
          return fail(result, framer_error::invalid_chunk_size);
        }
        case state::extension_before_name: {
          if (c == ' ' || c == '\t') break;
          if (helpers::is_token(c)) {
            state_ = state::extension_name;
            break;
          }
          return fail(result, framer_error::invalid_chunk_size);
        }
        case state::extension_name: {
          if (helpers::is_token(c)) break;
          if (c == ' ' || c == '\t') {
            state_ = state::extension_after_name;
            break;
          }
          if (c == '=') {
            state_ = state::extension_before_value;
            break;
          }
          if (c == ';') {
            state_ = state::extension_before_name;
            break;
          }
          if (c == '\r') {
            state_ = state::size_lf;
            break;
          }
          return fail(result, framer_error::invalid_chunk_size);
        }
        case state::extension_after_name: {
          if (c == ' ' || c == '\t') break;
          if (c == '=') {
            state_ = state::extension_before_value;
            break;
          }
          if (c == ';') {
            state_ = state::extension_before_name;
            break;
          }
          if (c == '\r') {
            state_ = state::size_lf;
            break;
          }
          return fail(result, framer_error::invalid_chunk_size);
        }
        case state::extension_before_value: {
          if (c == ' ' || c == '\t') break;
          if (helpers::is_token(c)) {
            state_ = state::extension_token_value;
            break;
          }
          if (c == '"') {
            state_ = state::extension_quoted_value;
            break;
          }
          return fail(result, framer_error::invalid_chunk_size);
        }
        case state::extension_token_value: {
          if (helpers::is_token(c)) break;
          if (c == ' ' || c == '\t') {
            state_ = state::extension_after_value;
            break;
          }
          if (c == ';') {
            state_ = state::extension_before_name;
            break;
          }
          if (c == '\r') {
            state_ = state::size_lf;
            break;
          }
          return fail(result, framer_error::invalid_chunk_size);
        }
        case state::extension_quoted_value: {
          if (helpers::is_qdtext(c)) break;
          if (c == '\\') {
            state_ = state::extension_quoted_pair;
            break;
          }
          if (c == '"') {
            state_ = state::extension_after_value;
            break;
          }
          return fail(result, framer_error::invalid_chunk_size);
        }
        case state::extension_quoted_pair: {
          if (c == '\t' || c == ' ' || helpers::is_vchar(c) ||
              helpers::is_obs_text(c)) {
            state_ = state::extension_quoted_value;
            break;
          }
          return fail(result, framer_error::invalid_chunk_size);
        }
        case state::extension_after_value: {
          if (c == ' ' || c == '\t') break;
          if (c == ';') {
            state_ = state::extension_before_name;
            break;
          }
          if (c == '\r') {
            state_ = state::size_lf;
            break;
          }
          return fail(result, framer_error::invalid_chunk_size);
        }
        // ---------------------------------------------------------------------
        // LF after chunk-size CR: decide data vs last-chunk
        // ---------------------------------------------------------------------
        case state::size_lf: {
          if (c != '\n') {
            // Invalid LF after chunk-size CR!
            return fail(result, framer_error::invalid_chunk_crlf);
          }
          extension_size_ = 0;
          if (chunk_remaining_ == 0) {
            state_ = state::trailer_line_start;
            trailer_size_ = 0;
          } else {
            state_ = state::data;
          }
          break;
        }
        // ---------------------------------------------------------------------
        // Chunk data: bulk-write min(remaining, available) bytes
        // ---------------------------------------------------------------------
        case state::data: {
          std::size_t to_take = std::min(chunk_remaining_, input.size() - i);
          if (!dst.write(input.subspan(i, to_take))) {
            // An I/O error occurred while writing to the destination!
            return fail(result, framer_error::io_error);
          }
          result.consumed += to_take;
          i += to_take;
          chunk_remaining_ -= to_take;
          if (chunk_remaining_ == 0) {
            // All chunk data consumed; expect post-data CRLF next.
            state_ = state::data_cr;
          }
          continue;  // i already advanced - skip the single-byte path below
        }
        // ---------------------------------------------------------------------
        // Post-data CRLF
        // ---------------------------------------------------------------------
        case state::data_cr: {
          if (c != '\r') {
            // Invalid CR after chunk data!
            return fail(result, framer_error::invalid_chunk_crlf);
          }
          state_ = state::data_lf;
          break;
        }
        case state::data_lf: {
          if (c != '\n') {
            // Invalid LF after chunk data CR!
            return fail(result, framer_error::invalid_chunk_crlf);
          }
          chunk_remaining_ = 0;
          chunk_size_started_ = false;
          state_ = state::chunk_size;
          break;
        }
        // ---------------------------------------------------------------------
        // Trailer section
        // ---------------------------------------------------------------------
        case state::trailer_line_start: {
          if (helpers::is_token(c)) {
            state_ = state::trailer_name;
            break;
          }
          if (c == '\r') {
            state_ = state::trailer_end_lf;
            break;
          }
          return fail(result, framer_error::invalid_trailer);
        }
        case state::trailer_name: {
          if (helpers::is_token(c)) break;
          if (c == ':') {
            state_ = state::trailer_value;
            break;
          }
          return fail(result, framer_error::invalid_trailer);
        }
        case state::trailer_value: {
          if (c == '\r') {
            state_ = state::trailer_line_lf;
            break;
          }
          if (c == '\t' || c == ' ' || helpers::is_vchar(c) ||
              helpers::is_obs_text(c)) {
            break;
          }
          return fail(result, framer_error::invalid_trailer);
        }
        case state::trailer_line_lf: {
          if (c != '\n') {
            return fail(result, framer_error::invalid_trailer);
          }
          state_ = state::trailer_line_start;
          break;
        }
        case state::trailer_end_lf: {
          if (c != '\n') {
            return fail(result, framer_error::invalid_trailer);
          }
          if (!dst.write(input.subspan(i, 1))) {
            return fail(result, framer_error::io_error);
          }
          result.consumed += 1;
          state_ = state::complete;
          result.complete = true;
          return result;
        }
        default:
          break;
      }
      // Single-byte write for all non-data, non-complete paths.
      if (!dst.write(input.subspan(i, 1))) {
        // An I/O error occurred while writing to the destination!
        return fail(result, framer_error::io_error);
      }
      result.consumed += 1;
      i++;
    }
    result.complete = (state_ == state::complete);
    return result;
  }

 private:
  // +=========================================================================+
  // | [>] hex_digit                                               ( private ) |
  // +=========================================================================+
  static int hex_digit(char c) noexcept {
    if (c >= '0' && c <= '9') {
      return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
      return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
      return c - 'A' + 10;
    }
    return -1;
  }
  // +=========================================================================+
  // | [>] fail                                                    ( private ) |
  // +=========================================================================+
  framer_state fail(framer_state& result, framer_error err) {
    state_ = state::error;
    error_ = err;
    result.has_error = true;
    result.error = err;
    return result;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  state state_{state::chunk_size};
  framer_error error_{framer_error::none};
  std::size_t chunk_remaining_{0};
  std::size_t extension_size_{0};
  std::size_t trailer_size_{0};
  bool chunk_size_started_{false};
};
}  // namespace martianlabs::doba::protocol::http::v11::body

#endif
