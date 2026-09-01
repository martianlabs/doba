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

#ifndef martianlabs_doba_protocol_http_v11_body_reader_chunked_h
#define martianlabs_doba_protocol_http_v11_body_reader_chunked_h

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>

#include "common/reader.h"
#include "protocol/http/common/helpers.h"
#include "protocol/http/v11/body/reader_state.h"
#include "protocol/http/v11/limits.h"

namespace martianlabs::doba::protocol::http::v11::body {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] reader_chunked                                              ( class ) |
// +---------------------------------------------------------------------------+
// | Decodes a chunked Transfer-Encoding body (as accumulated verbatim by      |
// | framer_chunked) by pulling wire bytes on demand from a common::reader     |
// | source and filling the caller-supplied output span.                       |
// |                                                                           |
// | The caller pulls decoded payload via read(). Each call walks the same     |
// | chunked framing grammar as framer_chunked, but chunk-size lines,          |
// | extensions, CRLFs and trailers are wire framing, not payload: they are    |
// | consumed from src and discarded, never written to output.                 |
// |                                                                           |
// | reader_state::produced reports the exact number of decoded payload bytes  |
// | written into output during this call. If src is only temporarily          |
// | exhausted (more bytes may still arrive), read() returns early with        |
// | complete=false and no error; the caller may invoke it again once more     |
// | wire bytes are available in src. If src reaches definitive eof() before   |
// | the body reaches state::complete, this is a protocol error reported as    |
// | reader_error::chunked_incomplete.                                         |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class reader_chunked {
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
  reader_chunked() = default;
  // +=========================================================================+
  // | [>] read                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // | Pulls wire bytes from src, walking the chunked framing, and writes only |
  // | chunk-data bytes into output. Returns the number of decoded payload     |
  // | bytes produced and whether the body is complete (last-chunk +           |
  // | terminating CRLF seen).                                                 |
  // +=========================================================================+
  reader_state read(common::reader& src, std::span<std::byte> output) {
    reader_state result;
    if (state_ == state::complete) {
      result.complete = true;
      return result;
    }
    if (state_ == state::error) {
      result.has_error = true;
      result.error = error_;
      return result;
    }
    std::size_t out_pos = 0;
    while (true) {
      // -----------------------------------------------------------------------
      // Chunk-data: bulk-copy payload bytes directly from src into output
      // -----------------------------------------------------------------------
      if (state_ == state::data) {
        std::size_t room = output.size() - out_pos;
        if (room == 0) break;  // output full - stop here for this call
        std::size_t to_take = std::min(room, chunk_remaining_);
        if (to_take > 0) {
          std::size_t got = src.read(output.subspan(out_pos, to_take));
          if (got == 0) {
            if (src.failed()) {
              result.produced = out_pos;
              return fail(result, reader_error::io_error);
            }
            if (src.eof()) {
              result.produced = out_pos;
              return fail(result, reader_error::chunked_incomplete);
            }
            break;  // src exhausted for now - resume on next call
          }
          out_pos += got;
          chunk_remaining_ -= got;
          if (chunk_remaining_ == 0) {
            // All chunk data consumed; expect post-data CRLF next.
            state_ = state::data_cr;
          }
          continue;
        }
        // chunk_remaining_ == 0: move on to the post-data CRLF immediately.
        state_ = state::data_cr;
        continue;
      }
      // ---------------------------------------------------------------------
      // All other states consume a single wire byte from src (framing only)
      // ---------------------------------------------------------------------
      std::byte b;
      if (!src.fetch(b)) {
        if (src.failed()) {
          result.produced = out_pos;
          return fail(result, reader_error::io_error);
        }
        if (src.eof()) {
          result.produced = out_pos;
          return fail(result, reader_error::chunked_incomplete);
        }
        break;  // src exhausted for now - resume on next call
      }
      const char c = static_cast<char>(b);
      if (state_ >= state::extension_before_semicolon &&
          state_ <= state::extension_after_value &&
          ++extension_size_ > limits::kMaxChunkedExtensionSize) {
        result.produced = out_pos;
        return fail(result, reader_error::chunk_extension_size_limit_exceeded);
      }
      if (state_ >= state::trailer_line_start &&
          state_ <= state::trailer_end_lf &&
          ++trailer_size_ > limits::kMaxChunkedTrailerSize) {
        result.produced = out_pos;
        return fail(result, reader_error::trailer_size_limit_exceeded);
      }
      switch (state_) {
        // ---------------------------------------------------------------------
        // Chunk-size line: accumulate hex digits, handle extension and CR
        // ---------------------------------------------------------------------
        case state::chunk_size: {
          if (c == ';') {
            if (!chunk_size_started_) {
              result.produced = out_pos;
              return fail(result, reader_error::invalid_chunk_size);
            }
            state_ = state::extension_before_name;
            break;
          }
          if (c == '\r') {
            if (!chunk_size_started_) {
              result.produced = out_pos;
              return fail(result, reader_error::invalid_chunk_size);
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
            result.produced = out_pos;
            return fail(result, reader_error::invalid_chunk_size);
          }
          if (chunk_remaining_ >
              (std::numeric_limits<std::size_t>::max() >> 4)) {
            // Overflow in chunk size accumulation!
            result.produced = out_pos;
            return fail(result, reader_error::chunk_size_overflow);
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
          result.produced = out_pos;
          return fail(result, reader_error::invalid_chunk_size);
        }
        case state::extension_before_name: {
          if (c == ' ' || c == '\t') break;
          if (helpers::is_token(c)) {
            state_ = state::extension_name;
            break;
          }
          result.produced = out_pos;
          return fail(result, reader_error::invalid_chunk_size);
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
          result.produced = out_pos;
          return fail(result, reader_error::invalid_chunk_size);
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
          result.produced = out_pos;
          return fail(result, reader_error::invalid_chunk_size);
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
          result.produced = out_pos;
          return fail(result, reader_error::invalid_chunk_size);
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
          result.produced = out_pos;
          return fail(result, reader_error::invalid_chunk_size);
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
          result.produced = out_pos;
          return fail(result, reader_error::invalid_chunk_size);
        }
        case state::extension_quoted_pair: {
          if (c == '\t' || c == ' ' || helpers::is_vchar(c) ||
              helpers::is_obs_text(c)) {
            state_ = state::extension_quoted_value;
            break;
          }
          result.produced = out_pos;
          return fail(result, reader_error::invalid_chunk_size);
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
          result.produced = out_pos;
          return fail(result, reader_error::invalid_chunk_size);
        }
        // ---------------------------------------------------------------------
        // LF after chunk-size CR: decide data vs last-chunk
        // ---------------------------------------------------------------------
        case state::size_lf: {
          if (c != '\n') {
            // Invalid LF after chunk-size CR!
            result.produced = out_pos;
            return fail(result, reader_error::invalid_chunk_crlf);
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
        // Post-data CRLF
        // ---------------------------------------------------------------------
        case state::data_cr: {
          if (c != '\r') {
            // Invalid CR after chunk data!
            result.produced = out_pos;
            return fail(result, reader_error::invalid_chunk_crlf);
          }
          state_ = state::data_lf;
          break;
        }
        case state::data_lf: {
          if (c != '\n') {
            // Invalid LF after chunk data CR!
            result.produced = out_pos;
            return fail(result, reader_error::invalid_chunk_crlf);
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
          result.produced = out_pos;
          return fail(result, reader_error::invalid_trailer);
        }
        case state::trailer_name: {
          if (helpers::is_token(c)) break;
          if (c == ':') {
            state_ = state::trailer_value;
            break;
          }
          result.produced = out_pos;
          return fail(result, reader_error::invalid_trailer);
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
          result.produced = out_pos;
          return fail(result, reader_error::invalid_trailer);
        }
        case state::trailer_line_lf: {
          if (c != '\n') {
            result.produced = out_pos;
            return fail(result, reader_error::invalid_trailer);
          }
          state_ = state::trailer_line_start;
          break;
        }
        case state::trailer_end_lf: {
          if (c != '\n') {
            result.produced = out_pos;
            return fail(result, reader_error::invalid_trailer);
          }
          state_ = state::complete;
          result.produced = out_pos;
          result.complete = true;
          return result;
        }
        default:
          break;
      }
    }
    result.produced = out_pos;
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
  reader_state fail(reader_state& result, reader_error err) {
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
  reader_error error_{reader_error::none};
  std::size_t chunk_remaining_{0};
  std::size_t extension_size_{0};
  std::size_t trailer_size_{0};
  bool chunk_size_started_{false};
};
}  // namespace martianlabs::doba::protocol::http::v11::body

#endif
