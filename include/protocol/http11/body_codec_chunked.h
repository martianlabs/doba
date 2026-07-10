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
//        --- martianLabs Anti-AI Usage and Model-Training Addendum ---
//
// TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION
//
// Copyright 2025 martianLabs
//
// Except as otherwise stated in this Addendum, this software is licensed
// under the Apache License, Version 2.0 (the "License"); you may not use
// this file except in compliance with the License.
//
// The following additional terms are hereby added to the Apache License for
// the purpose of restricting the use of this software by Artificial
// Intelligence systems, machine learning models, data-scraping bots, and
// automated systems.
//
// 1.  MACHINE LEARNING AND AI RESTRICTIONS
//     1.1. No entity, organization, or individual may use this software,
//          its source code, object code, or any derivative work for the
//          purpose of training, fine-tuning, evaluating, or improving any
//          machine learning model, artificial intelligence system, large
//          language model, or similar automated system.
//     1.2. No automated system may copy, parse, analyze, index, or
//          otherwise process this software for any AI-related purpose.
//     1.3. Use of this software as input, prompt material, reference
//          material, or evaluation data for AI systems is expressly
//          prohibited.
//
// 2.  SCRAPING AND AUTOMATED ACCESS RESTRICTIONS
//     2.1. No automated crawler, training pipeline, or data-extraction
//          system may collect, store, or incorporate any portion of this
//          software in any dataset used for machine learning or AI
//          training.
//     2.2. Any automated access must comply with this License and with
//          applicable copyright law.
//
// 3.  PROHIBITION ON DERIVATIVE DATASETS
//     3.1. You may not create datasets, corpora, embeddings, vector
//          stores, or similar derivative data intended for use by
//          automated systems, AI models, or machine learning algorithms.
//
// 4.  NO WAIVER OF RIGHTS
//     4.1. These restrictions apply in addition to, and do not limit,
//          the rights and protections provided to the copyright holder
//          under the Apache License Version 2.0 and applicable law.
//
// 5.  ACCEPTANCE
//     5.1. Any use of this software constitutes acceptance of both the
//          Apache License Version 2.0 and this Anti-AI Addendum.
//
// You may obtain a copy of the Apache License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.  See the License for the specific language governing
// permissions and limitations under the Apache License Version 2.0.

#ifndef martianlabs_doba_protocol_http11_body_codec_chunked_h
#define martianlabs_doba_protocol_http11_body_codec_chunked_h

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>

#include "protocol/http11/body_common.h"

namespace martianlabs::doba::protocol::http11 {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// | [>] body_codec_chunked                                           ( class ) |
// +---------------------------------------------------------------------------+
// | HTTP/1.1 chunked transfer-coding body codec. Fulfils the BODY CODEC        |
// | CONTRACT declared in body_common.h so it can be plugged into               |
// | body_writer<body_codec_chunked> and body_reader<body_codec_chunked>.        |
// |                                                                           |
// | encode : each encode() call frames the input as one complete chunk         |
// |          "<hex-size>\r\n<data>\r\n"; finish() appends the last-chunk        |
// |          terminator "0\r\n\r\n" exactly once.                              |
// | decode : owns the chunked decoder state machine (chunk size, extensions,  |
// |          data, trailers) and returns only the logical payload bytes.      |
// |                                                                           |
// | The chunk-extension and trailer size limits are intrinsic to the coding   |
// | and are supplied at construction time (0 means unlimited).                |
// +===========================================================================+
// /////////////////////////////////////////////////////////////////////////////
class body_codec_chunked {
  // +=========================================================================+
  // | [>] TYPEs                                                   ( private ) |
  // +=========================================================================+
  // Chunked decoder state machine.
  enum class chunked_state : std::uint8_t {
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
  // | [>] CONSTRUCTORs                                             ( public ) |
  // +=========================================================================+
  body_codec_chunked() = default;
  body_codec_chunked(std::uint64_t max_chunk_extension_size,
                     std::uint64_t max_trailer_size)
      : max_chunk_extension_size_(max_chunk_extension_size),
        max_trailer_size_(max_trailer_size) {}
  // +=========================================================================+
  // | [>] encode                                                   ( public ) |
  // | ------------------------------------------------------------------------+
  // | Frames len bytes as one chunk "<hex-size>\r\n<data>\r\n" into the sink. |
  // | Reports the total bytes stored (framing overhead + data).               |
  // +=========================================================================+
  template <typename BDty>
  bool encode(BDty& sink, const char* data, std::size_t len,
              encode_result& out) {
    out.stored = 0;
    if (len == 0) return true;
    char size_line[24];
    int size_line_len = format_chunk_size(size_line, len);
    // size_line + CRLF + data + CRLF
    std::uint64_t frame_overhead =
        static_cast<std::uint64_t>(size_line_len) + 4;
    if (len > UINT64_MAX - frame_overhead) {
      out.has_error = true;
      out.error = body_error::chunk_size_overflow;
      return false;
    }
    std::uint64_t chunk_bytes = frame_overhead + static_cast<std::uint64_t>(len);
    if (!sink.write(size_line, static_cast<std::size_t>(size_line_len))) {
      return false;
    }
    if (!sink.write("\r\n", 2)) return false;
    if (!sink.write(data, len)) return false;
    if (!sink.write("\r\n", 2)) return false;
    out.stored = chunk_bytes;
    return true;
  }
  // +=========================================================================+
  // | [>] finish                                                   ( public ) |
  // | ------------------------------------------------------------------------+
  // | Appends the last-chunk terminator "0\r\n\r\n". Reports its size.        |
  // +=========================================================================+
  template <typename BDty>
  bool finish(BDty& sink, encode_result& out) {
    out.stored = 0;
    if (!sink.write("0\r\n\r\n", 5)) return false;
    out.stored = 5;
    return true;
  }
  // +=========================================================================+
  // | [>] decode                                                   ( public ) |
  // | ------------------------------------------------------------------------+
  // | Decodes chunk framing from the source into output, advancing the        |
  // | internal state machine. Returns the logical bytes produced plus         |
  // | completion/error state.                                                 |
  // +=========================================================================+
  template <typename BDty>
  decode_result decode(BDty& src, std::span<std::byte> output,
                       std::uint64_t& source_consumed) {
    decode_result r;
    if (cstate_ == chunked_state::complete) {
      r.complete = true;
      return r;
    }
    if (cstate_ == chunked_state::error) {
      r.has_error = true;
      r.error = decode_error_;
      return r;
    }
    std::size_t out_pos = 0;
    bool keep_going = true;
    while (keep_going && cstate_ != chunked_state::complete &&
           cstate_ != chunked_state::error) {
      switch (cstate_) {
        case chunked_state::reading_chunk_size:
          keep_going = step_chunk_size(src, source_consumed);
          break;
        case chunked_state::reading_chunk_extension:
          keep_going = step_chunk_extension(src, source_consumed);
          break;
        case chunked_state::reading_chunk_size_lf:
          keep_going = step_chunk_size_lf(src, source_consumed);
          break;
        case chunked_state::reading_chunk_data:
          keep_going = step_chunk_data(src, source_consumed, output, out_pos);
          break;
        case chunked_state::reading_chunk_data_cr:
          keep_going = step_chunk_data_cr(src, source_consumed);
          break;
        case chunked_state::reading_chunk_data_lf:
          keep_going = step_chunk_data_lf(src, source_consumed);
          break;
        case chunked_state::reading_trailers:
          keep_going = step_trailers(src, source_consumed);
          break;
        default:
          keep_going = false;
          break;
      }
    }
    if (cstate_ == chunked_state::error) {
      r.has_error = true;
      r.error = decode_error_;
      return r;
    }
    r.produced = out_pos;
    if (cstate_ == chunked_state::complete) r.complete = true;
    return r;
  }

 private:
  // +=========================================================================+
  // | [>] fail                                                    ( private ) |
  // +=========================================================================+
  void fail(body_error e) {
    decode_error_ = e;
    cstate_ = chunked_state::error;
  }
  // +=========================================================================+
  // | [>] fetch_byte                                              ( private ) |
  // | ------------------------------------------------------------------------+
  // | Pulls one byte from the backend for the chunked decoder, updating the   |
  // | consumed counter and mapping an early end-of-source to the appropriate  |
  // | error (io_error when the backend faulted, chunked_incomplete otherwise).|
  // +=========================================================================+
  template <typename BDty>
  bool fetch_byte(BDty& src, std::uint64_t& source_consumed, std::byte& b) {
    if (!src.fetch(b)) {
      if (cstate_ != chunked_state::complete &&
          cstate_ != chunked_state::error) {
        fail(src.ok() ? body_error::chunked_incomplete : body_error::io_error);
      }
      return false;
    }
    source_consumed++;
    return true;
  }
  // +=========================================================================+
  // | [>] step_chunk_size                                         ( private ) |
  // +=========================================================================+
  template <typename BDty>
  bool step_chunk_size(BDty& src, std::uint64_t& source_consumed) {
    std::byte b;
    if (!fetch_byte(src, source_consumed, b)) return false;
    char c = static_cast<char>(b);
    if (c == ';') {
      cext_size_ = 0;
      cstate_ = chunked_state::reading_chunk_extension;
      return true;
    }
    if (c == '\r') {
      cstate_ = chunked_state::reading_chunk_size_lf;
      return true;
    }
    int digit = hex_digit(c);
    if (digit < 0) {
      fail(body_error::invalid_chunk_size);
      return false;
    }
    // overflow check: chunk_size_ > (UINT64_MAX - digit) / 16
    if (chunk_size_ > (UINT64_MAX - static_cast<std::uint64_t>(digit)) / 16) {
      fail(body_error::chunk_size_overflow);
      return false;
    }
    chunk_size_ = chunk_size_ * 16 + static_cast<std::uint64_t>(digit);
    return true;
  }
  // +=========================================================================+
  // | [>] step_chunk_extension                                    ( private ) |
  // +=========================================================================+
  template <typename BDty>
  bool step_chunk_extension(BDty& src, std::uint64_t& source_consumed) {
    std::byte b;
    if (!fetch_byte(src, source_consumed, b)) return false;
    char c = static_cast<char>(b);
    if (c == '\r') {
      cstate_ = chunked_state::reading_chunk_size_lf;
      return true;
    }
    cext_size_++;
    if (max_chunk_extension_size_ > 0 &&
        cext_size_ > max_chunk_extension_size_) {
      fail(body_error::chunk_extension_size_limit_exceeded);
      return false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] step_chunk_size_lf                                      ( private ) |
  // +=========================================================================+
  template <typename BDty>
  bool step_chunk_size_lf(BDty& src, std::uint64_t& source_consumed) {
    std::byte b;
    if (!fetch_byte(src, source_consumed, b)) return false;
    if (static_cast<char>(b) != '\n') {
      fail(body_error::invalid_chunk_crlf);
      return false;
    }
    if (chunk_size_ == 0) {
      trailer_size_ = 0;
      trailer_eol_count_ = 1;  // last-chunk's own CRLF counts as first
      trailer_cr_seen_ = false;
      cstate_ = chunked_state::reading_trailers;
    } else {
      chunk_remaining_ = chunk_size_;
      chunk_size_ = 0;
      cstate_ = chunked_state::reading_chunk_data;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] step_chunk_data                                         ( private ) |
  // +=========================================================================+
  template <typename BDty>
  bool step_chunk_data(BDty& src, std::uint64_t& source_consumed,
                       std::span<std::byte> output, std::size_t& out_pos) {
    std::size_t want = static_cast<std::size_t>(std::min(
        chunk_remaining_, static_cast<std::uint64_t>(output.size() - out_pos)));
    if (want == 0) return false;  // output full
    std::size_t got = src.read(output.subspan(out_pos, want));
    if (got == 0) {
      // Source returned nothing: either EOF (incomplete) or I/O error.
      fail(src.ok() ? body_error::chunked_incomplete : body_error::io_error);
      return false;
    }
    source_consumed += got;
    out_pos += got;
    chunk_remaining_ -= static_cast<std::uint64_t>(got);
    if (chunk_remaining_ == 0) cstate_ = chunked_state::reading_chunk_data_cr;
    return true;
  }
  // +=========================================================================+
  // | [>] step_chunk_data_cr                                      ( private ) |
  // +=========================================================================+
  template <typename BDty>
  bool step_chunk_data_cr(BDty& src, std::uint64_t& source_consumed) {
    std::byte b;
    if (!fetch_byte(src, source_consumed, b)) return false;
    if (static_cast<char>(b) != '\r') {
      fail(body_error::invalid_chunk_crlf);
      return false;
    }
    cstate_ = chunked_state::reading_chunk_data_lf;
    return true;
  }
  // +=========================================================================+
  // | [>] step_chunk_data_lf                                      ( private ) |
  // +=========================================================================+
  template <typename BDty>
  bool step_chunk_data_lf(BDty& src, std::uint64_t& source_consumed) {
    std::byte b;
    if (!fetch_byte(src, source_consumed, b)) return false;
    if (static_cast<char>(b) != '\n') {
      fail(body_error::invalid_chunk_crlf);
      return false;
    }
    chunk_size_ = 0;
    cext_size_ = 0;
    cstate_ = chunked_state::reading_chunk_size;
    return true;
  }
  // +=========================================================================+
  // | [>] step_trailers                                           ( private ) |
  // +=========================================================================+
  template <typename BDty>
  bool step_trailers(BDty& src, std::uint64_t& source_consumed) {
    // RFC 9112: trailer-section = *( field-line CRLF ) CRLF
    // trailer_eol_count_ starts at 1 (last-chunk's CRLF already seen).
    // Reaching 2 means the blank-line terminator has been consumed.
    std::byte b;
    if (!fetch_byte(src, source_consumed, b)) return false;
    char c = static_cast<char>(b);
    trailer_size_++;
    if (max_trailer_size_ > 0 && trailer_size_ > max_trailer_size_) {
      fail(body_error::trailer_size_limit_exceeded);
      return false;
    }
    if (c == '\r') {
      trailer_cr_seen_ = true;
    } else if (c == '\n' && trailer_cr_seen_) {
      trailer_cr_seen_ = false;
      if (++trailer_eol_count_ >= 2) cstate_ = chunked_state::complete;
    } else {
      trailer_cr_seen_ = false;
      trailer_eol_count_ = 0;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] format_chunk_size                                       ( private ) |
  // +=========================================================================+
  static int format_chunk_size(char* buf, std::size_t n) {
    char tmp[16];
    int i = 0;
    if (n == 0) {
      buf[0] = '0';
      return 1;
    }
    while (n > 0) {
      tmp[i++] = "0123456789abcdef"[n & 0xF];
      n >>= 4;
    }
    for (int j = 0; j < i; j++) buf[j] = tmp[i - 1 - j];
    return i;
  }
  // +=========================================================================+
  // | [>] hex_digit                                               ( private ) |
  // +=========================================================================+
  static int hex_digit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  std::uint64_t max_chunk_extension_size_{0};
  std::uint64_t max_trailer_size_{0};
  body_error decode_error_{body_error::none};
  // Chunked decoder state.
  chunked_state cstate_{chunked_state::reading_chunk_size};
  std::uint64_t chunk_size_{0};
  std::uint64_t chunk_remaining_{0};
  std::uint64_t cext_size_{0};
  std::uint64_t trailer_size_{0};
  bool trailer_cr_seen_{false};
  int trailer_eol_count_{0};
};
}  // namespace martianlabs::doba::protocol::http11

#endif
