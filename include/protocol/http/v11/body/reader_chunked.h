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
#include "protocol/http/v11/policies.h"

namespace martianlabs::doba::protocol::http::v11::body {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] reader_chunked                                              ( class ) |
// +---------------------------------------------------------------------------+
// | Pull chunked wire bytes from common::reader and decode them into the      |
// | caller's output.                                                          |
// |                                                                           |
// | Give callers payload only. Consume and discard chunk sizes, extensions,   |
// | CRLFs and trailers; they aren't body data.                                |
// |                                                                           |
// | produced counts decoded bytes written this call. If the source is         |
// | temporarily empty, return incomplete without an error and try again       |
// | later. Final EOF before completion is chunked_incomplete.                 |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class reader_chunked {
  // +=========================================================================+
  // | [>] TYPEs                                                   ( private ) |
  // +-------------------------------------------------------------------------+
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
  // +-------------------------------------------------------------------------+
  static constexpr std::size_t kDefaultMaxChunkExtensionSize =
      policies::kDefaultMaxChunkExtensionSize;
  static constexpr std::size_t kDefaultMaxTrailerSectionSize =
      policies::kDefaultMaxTrailerSectionSize;
  // +=========================================================================+
  // | [>] CONSTRUCTORs                                             ( public ) |
  // +-------------------------------------------------------------------------+
  reader_chunked(std::size_t extension_limit = kDefaultMaxChunkExtensionSize,
                 std::size_t trailer_limit = kDefaultMaxTrailerSectionSize)
      : extension_limit_(extension_limit), trailer_limit_(trailer_limit) {}
  // +=========================================================================+
  // | [>] read                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // | Read chunk framing and copy only payload to output. Report produced     |
  // | bytes; complete means the final chunk and CRLF arrived.                 |
  // +-------------------------------------------------------------------------+
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
      // Copy chunk payload straight into output.
      // -----------------------------------------------------------------------
      if (state_ == state::data) {
        std::size_t room = output.size() - out_pos;
        if (room == 0) break;  // Output is full; stop this read.
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
            break;  // Nothing available yet; try on the next call.
          }
          out_pos += got;
          chunk_remaining_ -= got;
          if (chunk_remaining_ == 0) {
            // Chunk data is done; expect CRLF next.
            state_ = state::data_cr;
          }
          continue;
        }
        // No chunk data left; move straight to its CRLF.
        state_ = state::data_cr;
        continue;
      }
      // ---------------------------------------------------------------------
      // Other states consume one framing byte at a time.
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
        break;  // Nothing available yet; try on the next call.
      }
      const char c = static_cast<char>(b);
      if (state_ >= state::extension_before_semicolon &&
          state_ <= state::extension_after_value &&
          extension_limit_ && ++extension_size_ > extension_limit_) {
        result.produced = out_pos;
        return fail(result, reader_error::chunk_extension_size_limit_exceeded);
      }
      if (state_ >= state::trailer_line_start &&
          state_ <= state::trailer_end_lf &&
          trailer_limit_ && ++trailer_size_ > trailer_limit_) {
        result.produced = out_pos;
        return fail(result, reader_error::trailer_size_limit_exceeded);
      }
      switch (state_) {
        // +-------------------------------------------------------------------+
        // | Read hex size digits, then an optional extension and CR.          |
        // +-------------------------------------------------------------------+
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
          int digit = helpers::hex_value(c);
          if (digit < 0) {
            // Not a hex digit in the chunk size.
            result.produced = out_pos;
            return fail(result, reader_error::invalid_chunk_size);
          }
          if (chunk_remaining_ >
              (std::numeric_limits<std::size_t>::max() >> 4)) {
            // The chunk size won't fit.
            result.produced = out_pos;
            return fail(result, reader_error::chunk_size_overflow);
          }
          chunk_remaining_ =
              (chunk_remaining_ << 4) | static_cast<std::size_t>(digit);
          chunk_size_started_ = true;
          break;
        }
        // +-------------------------------------------------------------------+
        // | extension_before_semicolon                                        |
        // +-------------------------------------------------------------------+
        case state::extension_before_semicolon: {
          if (c == ' ' || c == '\t') break;
          if (c == ';') {
            state_ = state::extension_before_name;
            break;
          }
          result.produced = out_pos;
          return fail(result, reader_error::invalid_chunk_size);
        }
        // +-------------------------------------------------------------------+
        // | extension_before_name                                             |
        // +-------------------------------------------------------------------+
        case state::extension_before_name: {
          if (c == ' ' || c == '\t') break;
          if (helpers::is_token(c)) {
            state_ = state::extension_name;
            break;
          }
          result.produced = out_pos;
          return fail(result, reader_error::invalid_chunk_size);
        }
        // +-------------------------------------------------------------------+
        // | extension_name                                                    |
        // +-------------------------------------------------------------------+
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
        // +-------------------------------------------------------------------+
        // | extension_after_name                                              |
        // +-------------------------------------------------------------------+
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
        // +-------------------------------------------------------------------+
        // | extension_before_value                                            |
        // +-------------------------------------------------------------------+
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
        // +-------------------------------------------------------------------+
        // | extension_token_value                                             |
        // +-------------------------------------------------------------------+
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
        // +-------------------------------------------------------------------+
        // | extension_quoted_value                                            |
        // +-------------------------------------------------------------------+
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
        // +-------------------------------------------------------------------+
        // | extension_quoted_pair                                             |
        // +-------------------------------------------------------------------+
        case state::extension_quoted_pair: {
          if (c == '\t' || c == ' ' || helpers::is_vchar(c) ||
              helpers::is_obs_text(c)) {
            state_ = state::extension_quoted_value;
            break;
          }
          result.produced = out_pos;
          return fail(result, reader_error::invalid_chunk_size);
        }
        // +-------------------------------------------------------------------+
        // | extension_after_value                                             |
        // +-------------------------------------------------------------------+
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
        // +-------------------------------------------------------------------+
        // | After the size's LF, choose data or the final chunk.              |
        // +-------------------------------------------------------------------+
        case state::size_lf: {
          if (c != '\n') {
            // Expected LF after the chunk-size CR.
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
        // +-------------------------------------------------------------------+
        // | Expect CR after chunk data.                                       |
        // +-------------------------------------------------------------------+
        case state::data_cr: {
          if (c != '\r') {
            // Expected CR after chunk data.
            result.produced = out_pos;
            return fail(result, reader_error::invalid_chunk_crlf);
          }
          state_ = state::data_lf;
          break;
        }
        // +-------------------------------------------------------------------+
        // | Expect LF after the chunk-data CR.                                |
        // +-------------------------------------------------------------------+
        case state::data_lf: {
          if (c != '\n') {
            // Expected LF after the chunk-data CR.
            result.produced = out_pos;
            return fail(result, reader_error::invalid_chunk_crlf);
          }
          chunk_remaining_ = 0;
          chunk_size_started_ = false;
          state_ = state::chunk_size;
          break;
        }
        // +-------------------------------------------------------------------+
        // | trailer_line_start                                                |
        // +-------------------------------------------------------------------+
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
        // +-------------------------------------------------------------------+
        // | trailer_name                                                      |
        // +-------------------------------------------------------------------+
        case state::trailer_name: {
          if (helpers::is_token(c)) break;
          if (c == ':') {
            state_ = state::trailer_value;
            break;
          }
          result.produced = out_pos;
          return fail(result, reader_error::invalid_trailer);
        }
        // +-------------------------------------------------------------------+
        // | trailer_value                                                     |
        // +-------------------------------------------------------------------+
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
        // +-------------------------------------------------------------------+
        // | trailer_line_lf                                                   |
        // +-------------------------------------------------------------------+
        case state::trailer_line_lf: {
          if (c != '\n') {
            result.produced = out_pos;
            return fail(result, reader_error::invalid_trailer);
          }
          state_ = state::trailer_line_start;
          break;
        }
        // +-------------------------------------------------------------------+
        // | trailer_end_lf                                                    |
        // +-------------------------------------------------------------------+
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
  // | [>] fail                                                    ( private ) |
  // +-------------------------------------------------------------------------+
  reader_state fail(reader_state& result, reader_error err) {
    state_ = state::error;
    error_ = err;
    result.has_error = true;
    result.error = err;
    return result;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  state state_{state::chunk_size};
  reader_error error_{reader_error::none};
  std::size_t chunk_remaining_{0};
  std::size_t extension_limit_{kDefaultMaxChunkExtensionSize};
  std::size_t trailer_limit_{kDefaultMaxTrailerSectionSize};
  std::size_t extension_size_{0};
  std::size_t trailer_size_{0};
  bool chunk_size_started_{false};
};
}  // namespace martianlabs::doba::protocol::http::v11::body

#endif
