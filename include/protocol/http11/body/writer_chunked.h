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

#ifndef martianlabs_doba_protocol_http11_body_writer_chunked_h
#define martianlabs_doba_protocol_http11_body_writer_chunked_h

#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>

#include "common/writer.h"
#include "protocol/http11/body/writer_state.h"

namespace martianlabs::doba::protocol::http11::body {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] writer_chunked                                              ( class ) |
// +---------------------------------------------------------------------------+
// | Accumulates a chunked Transfer-Encoding body into a common::writer.       |
// |                                                                           |
// | The caller pushes incoming transport spans via write(); each call returns |
// | a feed_result with the number of wire bytes consumed (framing included)   |
// | and whether the body is complete. Only logical data bytes are written to  |
// | dst — chunk-size lines, extensions, and trailers are discarded.           |
// |                                                                           |
// | The state machine is purely push-based: no internal buffer is kept;       |
// | state is a single enum value plus a chunk-data counter.                   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class writer_chunked {
  // +=========================================================================+
  // | [>] TYPEs                                                   ( private ) |
  // +=========================================================================+
  enum class state : std::uint8_t {
    chunk_size,  // reading hex digits of the chunk-size field
    extension,   // skipping chunk-extension after ';' until CR
    size_cr,     // expecting CR of the chunk-size line (after digits/ext)
    size_lf,     // expecting LF of the chunk-size line
    data,        // reading chunk-data bytes
    data_cr,     // expecting CR after chunk-data
    data_lf,     // expecting LF after chunk-data
    trailer,     // reading trailer lines until empty line (CRLF CRLF)
    trailer_lf,  // expecting LF after the CR that ended a trailer line
    complete,    // last-chunk and terminating CRLF consumed
    error        // unrecoverable parse error
  };

 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs                                             ( public ) |
  // +=========================================================================+
  writer_chunked() = default;
  // +=========================================================================+
  // | [>] write                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // | Advances the state machine over input, writing logical body bytes into  |
  // | dst. Returns consumed (wire bytes processed from input) and complete    |
  // | once the terminating empty chunk has been fully parsed.                 |
  // +=========================================================================+
  writer_state write(std::span<const std::byte> input, common::writer& dst) {
    writer_state result;
    if (state_ == state::complete) {
      result.complete = true;
      return result;
    }
    if (state_ == state::error) {
      result.has_error = true;
      result.error = error_;
      return result;
    }
    for (std::size_t i = 0; i < input.size(); ++i) {
      const char c = static_cast<char>(input[i]);
      switch (state_) {
        // ---------------------------------------------------------------------
        // Chunk-size line: hex digits, optional extension, CRLF
        // ---------------------------------------------------------------------
        case state::chunk_size: {
          if (c == ';') {
            state_ = state::extension;
            break;
          }
          if (c == '\r') {
            state_ = state::size_lf;
            break;
          }
          int digit = hex_digit(c);
          if (digit < 0) {
            return fail(result, i, writer_error::invalid_chunk_size);
          }
          if (chunk_remaining_ >
              (std::numeric_limits<std::size_t>::max() >> 4)) {
            return fail(result, i, writer_error::chunk_size_overflow);
          }
          chunk_remaining_ =
              (chunk_remaining_ << 4) | static_cast<std::size_t>(digit);
          break;
        }
        // --------------------------------------------------------------------
        // Chunk-extension: skip until CR
        // --------------------------------------------------------------------
        case state::extension: {
          // Skip everything until CR
          if (c == '\r') state_ = state::size_lf;
          break;
        }
        // --------------------------------------------------------------------
        // Chunk-size line CRLF: expect CR then LF
        // --------------------------------------------------------------------
        case state::size_cr: {
          // Unused: CR is consumed inline in chunk_size/extension states.
          // Kept for completeness; fall through to size_lf handling.
          if (c != '\r')
            return fail(result, i, writer_error::invalid_chunk_crlf);
          state_ = state::size_lf;
          break;
        }
        // --------------------------------------------------------------------
        // Chunk-size line CRLF: expect LF after CR
        // --------------------------------------------------------------------
        case state::size_lf: {
          if (c != '\n')
            return fail(result, i, writer_error::invalid_chunk_crlf);
          if (chunk_remaining_ == 0) {
            // last-chunk — move to trailer parsing
            state_ = state::trailer;
            trailer_cr_seen_ = false;
            trailer_empty_line_ = false;
          } else {
            state_ = state::data;
          }
          break;
        }
        // --------------------------------------------------------------------
        // Chunk-data: read chunk_remaining_ bytes, then expect CRLF
        // --------------------------------------------------------------------
        case state::data: {
          // Bulk-copy as many data bytes as possible in one shot.
          std::size_t available = input.size() - i;
          std::size_t to_write =
              (chunk_remaining_ <= available) ? chunk_remaining_ : available;
          dst.write(input.subspan(i, to_write));
          chunk_remaining_ -= to_write;
          // Advance i by (to_write - 1) because the loop will add 1.
          i += to_write - 1;
          if (chunk_remaining_ == 0) state_ = state::data_cr;
          break;
        }
        // --------------------------------------------------------------------
        // Chunk-data CRLF: expect CR then LF
        // --------------------------------------------------------------------
        case state::data_cr: {
          if (c != '\r')
            return fail(result, i, writer_error::invalid_chunk_crlf);
          state_ = state::data_lf;
          break;
        }
        // --------------------------------------------------------------------
        // Chunk-data CRLF: expect LF after CR
        // --------------------------------------------------------------------
        case state::data_lf: {
          if (c != '\n')
            return fail(result, i, writer_error::invalid_chunk_crlf);
          // Ready for the next chunk-size line.
          chunk_remaining_ = 0;
          state_ = state::chunk_size;
          break;
        }
        // --------------------------------------------------------------------
        // Trailer lines: skip until empty line (CRLF CRLF)
        // --------------------------------------------------------------------
        case state::trailer: {
          // Trailers are discarded. We look for the empty line that signals
          // end-of-trailers: a bare CRLF at the start of a line.
          if (c == '\r') {
            trailer_cr_seen_ = true;
          } else if (c == '\n' && trailer_cr_seen_) {
            if (trailer_empty_line_) {
              // Second CRLF: the empty line after any trailers — done.
              result.consumed = i + 1;
              result.complete = true;
              state_ = state::complete;
              return result;
            }
            // First LF: either end of a real trailer line or the LF of the
            // empty line introduced by the last-chunk's CRLF. Track it.
            trailer_empty_line_ = trailer_line_start_;
            trailer_line_start_ = true;
            trailer_cr_seen_ = false;
          } else {
            trailer_cr_seen_ = false;
            trailer_line_start_ = false;
            trailer_empty_line_ = false;
          }
          break;
        }
        // --------------------------------------------------------------------
        // Complete or error states: no further processing
        // --------------------------------------------------------------------
        default:
          break;
      }
    }
    result.consumed = input.size();
    result.complete = (state_ == state::complete);
    return result;
  }

 private:
  // +=========================================================================+
  // | [>] hex_digit                                               ( private ) |
  // +=========================================================================+
  static int hex_digit(char c) noexcept {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
  }
  // +=========================================================================+
  // | [>] fail                                                    ( private ) |
  // +=========================================================================+
  writer_state fail(writer_state& result, std::size_t consumed,
                    writer_error err) {
    state_ = state::error;
    error_ = err;
    result.consumed = consumed;
    result.has_error = true;
    result.error = err;
    return result;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  state state_{state::chunk_size};
  writer_error error_{writer_error::none};
  std::size_t chunk_remaining_{0};
  bool trailer_cr_seen_{false};
  bool trailer_empty_line_{false};
  bool trailer_line_start_{true};
};
}  // namespace martianlabs::doba::protocol::http11::body

#endif
