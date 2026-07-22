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
// | Validates and accumulates a chunked Transfer-Encoding body into a         |
// | common::writer.                                                           |
// |                                                                           |
// | The caller pushes incoming transport spans via write(). Each call         |
// | validates the chunked framing and writes ALL wire bytes â€” including     |
// | chunk-size lines, extensions, trailers and terminating CRLF â€” verbatim  |
// | into dst. No decoding is performed here; decoding is deferred to a        |
// | reader pass over the completed buffer.                                    |
// |                                                                           |
// | writer_state::consumed reports the exact number of bytes belonging to     |
// | this body that were taken from the input span. Any remaining bytes in the |
// | span belong to the next request.                                          |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class writer_chunked {
  // +=========================================================================+
  // | [>] TYPEs                                                   ( private ) |
  // +=========================================================================+
  enum class state : std::uint8_t {
    chunk_size,  // reading hex digits of the chunk-size field
    extension,   // skipping chunk-extension after ';' until CR
    size_lf,     // expecting LF after the CR of the chunk-size line
    data,        // consuming chunk-data bytes (bulk)
    data_cr,     // expecting CR after chunk-data
    data_lf,     // expecting LF after the post-data CR
    trailer,     // reading trailer section until empty CRLF line
    complete,    // last-chunk and terminating CRLF fully consumed
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
  // | Validates the chunked framing in input and writes every wire byte into  |
  // | dst unchanged. Returns the number of bytes consumed from input and      |
  // | whether the body is complete (last-chunk + terminating CRLF seen).      |
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
    std::size_t i = 0;
    while (i < input.size()) {
      const char c = static_cast<char>(input[i]);
      switch (state_) {
        // --------------------------------------------------------------------
        // Chunk-size line: accumulate hex digits, handle extension and CR
        // --------------------------------------------------------------------
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
          if (digit < 0) return fail(result, writer_error::invalid_chunk_size);
          if (chunk_remaining_ >
              (std::numeric_limits<std::size_t>::max() >> 4)) {
            return fail(result, writer_error::chunk_size_overflow);
          }
          chunk_remaining_ =
              (chunk_remaining_ << 4) | static_cast<std::size_t>(digit);
          break;
        }
        // --------------------------------------------------------------------
        // Chunk-extension: discard until CR
        // --------------------------------------------------------------------
        case state::extension: {
          if (c == '\r') state_ = state::size_lf;
          break;
        }
        // --------------------------------------------------------------------
        // LF after chunk-size CR: decide data vs last-chunk
        // --------------------------------------------------------------------
        case state::size_lf: {
          if (c != '\n') return fail(result, writer_error::invalid_chunk_crlf);
          if (chunk_remaining_ == 0) {
            state_ = state::trailer;
            trailer_line_start_ = true;
            trailer_cr_seen_ = false;
          } else {
            state_ = state::data;
          }
          break;
        }
        // --------------------------------------------------------------------
        // Chunk data: bulk-write min(remaining, available) bytes
        // --------------------------------------------------------------------
        case state::data: {
          std::size_t to_take = std::min(chunk_remaining_, input.size() - i);
          if (!dst.write(input.subspan(i, to_take))) {
            return fail(result, writer_error::io_error);
          }
          result.consumed += to_take;
          i += to_take;
          chunk_remaining_ -= to_take;
          if (chunk_remaining_ == 0) state_ = state::data_cr;
          continue;  // i already advanced â€” skip the single-byte path below
        }
        // --------------------------------------------------------------------
        // Post-data CRLF
        // --------------------------------------------------------------------
        case state::data_cr: {
          if (c != '\r') return fail(result, writer_error::invalid_chunk_crlf);
          state_ = state::data_lf;
          break;
        }
        case state::data_lf: {
          if (c != '\n') return fail(result, writer_error::invalid_chunk_crlf);
          chunk_remaining_ = 0;
          state_ = state::chunk_size;
          break;
        }
        // --------------------------------------------------------------------
        // Trailer section: validate lines, detect terminating empty CRLF
        // --------------------------------------------------------------------
        case state::trailer: {
          if (c == '\r') {
            trailer_cr_seen_ = true;
          } else if (c == '\n') {
            if (trailer_cr_seen_ && trailer_line_start_) {
              // Terminating empty CRLF â€” body complete. Write final byte
              // and return with the exact consumed count.
              if (!dst.write(input.subspan(i, 1))) {
                return fail(result, writer_error::io_error);
              }
              result.consumed += 1;
              state_ = state::complete;
              result.complete = true;
              return result;
            }
            if (trailer_cr_seen_) {
              // End of a non-empty trailer field line.
              trailer_line_start_ = true;
            }
            trailer_cr_seen_ = false;
          } else {
            // Non-CR/LF byte: we are inside a trailer field value.
            trailer_cr_seen_ = false;
            trailer_line_start_ = false;
          }
          break;
        }
        default:
          break;
      }
      // Single-byte write for all non-data, non-complete paths.
      if (!dst.write(input.subspan(i, 1))) {
        return fail(result, writer_error::io_error);
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
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
  }
  // +=========================================================================+
  // | [>] fail                                                    ( private ) |
  // +=========================================================================+
  writer_state fail(writer_state& result, writer_error err) {
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
  writer_error error_{writer_error::none};
  std::size_t chunk_remaining_{0};
  bool trailer_cr_seen_{false};
  bool trailer_line_start_{false};
};
}  // namespace martianlabs::doba::protocol::http11::body

#endif
