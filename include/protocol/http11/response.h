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

#ifndef martianlabs_doba_protocol_http11_response_h
#define martianlabs_doba_protocol_http11_response_h

#include <optional>
#include <stdexcept>
#include <utility>

#include "platform.h"
#include "protocol/http11/body_reader.h"
#include "protocol/http11/body_serializer.h"
#include "status_codes.h"
#include "status_lines.h"

namespace martianlabs::doba::protocol::http11 {
// +---------------------------------------------------------------------------+
// | [>] response                                                    ( class ) |
// +---------------------------------------------------------------------------+
// | This class holds for the http 1.1 response implementation.                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class response {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  response() = default;
  response(const response&) = delete;
  response(response&& in) noexcept = delete;
  ~response() = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  response& operator=(const response&) = delete;
  response& operator=(response&& in) noexcept = delete;
  // +=========================================================================+
  // | [>] serialize                                                ( public ) |
  // +---------------------------------------------------------------------------
  // | Serializes the response by invoking sink(std::string_view) one or more  |
  // | times, in wire order: | |   1. The header block (status-line + headers +
  // CRLF terminator).        | |   2. The body, if any: | |      - inline body
  // (set_body(string_view)): single zero-copy sink call  | |        over the
  // fixed internal buffer region.                            | |      -
  // streaming body (set_body(body_reader&&)): repeated sink |        calls, one
  // per kMaxBodySizeInMemory-sized chunk until eof(). The  | |        inline
  // body region of memory_[] is reused as the read buffer      | | (inline and
  // streaming bodies are mutually exclusive).             | |   If no body has
  // been set, only the header block is delivered.           | | SNfn must be
  // callable as void(std::string_view).                        |
  // +=========================================================================+
  template <typename SNfn>
  void serialize(SNfn&& sink) {
    std::size_t sln_plus_hdr_len = sln_len_ + hdr_len_;
    // Write the header-terminating CRLF (plus an extra CRLF when there are no
    // headers). The core section must always stay within [0, bdy_beg_).
    std::size_t crlf_bytes = hdr_len_ ? 2 : 4;
    if (sln_plus_hdr_len + crlf_bytes > bdy_beg_) {
      throw std::out_of_range("not enough space to serialize response!");
    }
    memory_[sln_plus_hdr_len++] = '\r';
    memory_[sln_plus_hdr_len++] = '\n';
    if (!hdr_len_) {
      memory_[sln_plus_hdr_len++] = '\r';
      memory_[sln_plus_hdr_len++] = '\n';
    }
    // 1. Header block — always a single zero-copy call into memory_.
    sink(std::string_view(memory_, sln_plus_hdr_len));
    // 2a. Inline body — single zero-copy call over memory_[bdy_beg_].
    if (bdy_len_ > 0) {
      sink(std::string_view(&memory_[bdy_beg_], bdy_len_));
      return;
    }
    // 2b. Streaming body — chunk-by-chunk until the source is exhausted.
    // memory_[bdy_beg_] is unused in this path (inline and streaming bodies
    // are mutually exclusive), so it serves as the read buffer at zero extra
    // cost: no stack allocation, no heap allocation.
    if (body_reader_) {
      while (!body_reader_->eof() && !body_reader_->failed()) {
        std::size_t n = body_reader_->read(std::span<std::byte>(
            reinterpret_cast<std::byte*>(&memory_[bdy_beg_]),
            kMaxBodySizeInMemory));
        if (n > 0) sink(std::string_view(&memory_[bdy_beg_], n));
      }
    }
  }
  // +=========================================================================+
  // | [>] add_header                                               ( public ) |
  // +=========================================================================+
  response& add_header(std::string_view k, std::string_view v) {
    std::size_t k_size = k.size();
    std::size_t v_size = v.size();
    std::size_t space_left = bdy_beg_ - sln_len_ - hdr_len_;
    // Bytes written by this call: key + ':' + ' ' + value + '\r' + '\n' = k + v
    // + 4. Reserve, in addition, the 2 bytes of the header-terminating CRLF
    // that serialize() appends, so we never overrun into the body region.
    if (k_size + v_size + 4 + 2 > space_left) {
      throw std::out_of_range("not enough space to add header!");
    }
    std::memcpy(&memory_[sln_len_ + hdr_len_], k.data(), k.size());
    hdr_len_ += k.size();
    memory_[sln_len_ + hdr_len_] = ':';
    hdr_len_++;
    memory_[sln_len_ + hdr_len_] = ' ';
    hdr_len_++;
    std::memcpy(&memory_[sln_len_ + hdr_len_], v.data(), v.size());
    hdr_len_ += v.size();
    memory_[sln_len_ + hdr_len_] = '\r';
    hdr_len_++;
    memory_[sln_len_ + hdr_len_] = '\n';
    hdr_len_++;
    return *this;
  }
  // +=========================================================================+
  // | [>] add_header                                               ( public ) |
  // +=========================================================================+
  template <typename T>
    requires std::is_arithmetic_v<T>
  INLINE response& add_header(std::string_view key, const T& val) {
    return add_header(key, std::to_string(val));
  }
  // +=========================================================================+
  // | [>] set_header                                               ( public ) |
  // +=========================================================================+
  response& set_header(std::string_view k, std::string_view v) {
    std::size_t line_off, val_off, val_len, line_len;
    if (!find_header(k, line_off, val_off, val_len, line_len)) {
      // Header not present: reuse add_header to append it.
      return add_header(k, v);
    }
    std::size_t new_v_size = v.size();
    // Bytes after this header's value inside the header block (subsequent
    // headers) that must be relocated when the value length changes.
    std::size_t tail_off = val_off + val_len;
    std::size_t tail_len = (sln_len_ + hdr_len_) - tail_off;
    if (new_v_size > val_len) {
      // Growing: ensure the extra bytes still fit before the body region,
      // keeping room for the header-terminating CRLF that serialize() adds.
      std::size_t grow = new_v_size - val_len;
      std::size_t space_left = bdy_beg_ - sln_len_ - hdr_len_;
      if (grow + 2 > space_left) {
        throw std::out_of_range("not enough space to set header!");
      }
    }
    // Relocate the tail to make room for (or reclaim space from) the new value,
    // then write the new value in place. memmove tolerates overlap.
    std::memmove(&memory_[val_off + new_v_size], &memory_[tail_off], tail_len);
    std::memcpy(&memory_[val_off], v.data(), new_v_size);
    hdr_len_ = hdr_len_ - val_len + new_v_size;
    return *this;
  }
  // +=========================================================================+
  // | [>] set_header                                               ( public ) |
  // +=========================================================================+
  template <typename T>
    requires std::is_arithmetic_v<T>
  INLINE response& set_header(std::string_view key, const T& val) {
    return set_header(key, std::to_string(val));
  }
  // +=========================================================================+
  // | [>] has_header                                               ( public ) |
  // +=========================================================================+
  // | Returns true if a header whose name case-insensitively matches 'k' is   |
  // | present. Lets callers probe for a header without incurring the cost (or |
  // | control flow) of catching the exception thrown by get_header.           |
  // +-------------------------------------------------------------------------+
  INLINE bool has_header(std::string_view k) const {
    std::size_t line_off, val_off, val_len, line_len;
    return find_header(k, line_off, val_off, val_len, line_len);
  }
  // +=========================================================================+
  // | [>] get_header                                               ( public ) |
  // +=========================================================================+
  // | Returns the (key, value) pair for the first header whose name           |
  // | case-insensitively matches 'k'. Throws std::runtime_error if the header |
  // | is not present. The returned strings are owned copies: they do NOT      |
  // | alias memory_ and stay valid across later mutations of this response.   |
  // +-------------------------------------------------------------------------+
  std::pair<std::string, std::string> get_header(std::string_view k) const {
    std::size_t line_off, val_off, val_len, line_len;
    if (!find_header(k, line_off, val_off, val_len, line_len)) {
      throw std::runtime_error("header not found!");
    }
    return {std::string(k), std::string(&memory_[val_off], val_len)};
  }
  // +=========================================================================+
  // | [>] get_header                                               ( public ) |
  // +=========================================================================+
  // | Returns the (key, value) pair for the header at position 'index' (in    |
  // | insertion order). Throws std::out_of_range if 'index' is beyond the     |
  // | number of headers. The returned strings are owned copies (see above).   |
  // +-------------------------------------------------------------------------+
  std::pair<std::string, std::string> get_header(std::size_t index) const {
    std::size_t pos = sln_len_;
    std::size_t end = sln_len_ + hdr_len_;
    std::size_t current = 0;
    while (pos < end) {
      std::size_t name_beg = pos;
      std::size_t colon = pos;
      while (colon < end && memory_[colon] != ':') colon++;
      if (colon >= end) break;
      std::size_t val_beg = colon + 2;
      std::size_t cr = val_beg;
      while (cr < end && memory_[cr] != '\r') cr++;
      if (cr >= end) break;
      if (current == index) {
        // Copy into caller-owned std::strings (see rationale above).
        return {std::string(&memory_[name_beg], colon - name_beg),
                std::string(&memory_[val_beg], cr - val_beg)};
      }
      current++;
      // Advance past CRLF (\r\n) to the next header line.
      pos = cr + 2;
    }
    throw std::out_of_range("header index out of range!");
  }
  // +=========================================================================+
  // | [>] get_headers_length                                       ( public ) |
  // +=========================================================================+
  // | Returns the number of headers currently stored. Enables safe indexed    |
  // | iteration via get_header(index) without relying on catching exceptions. |
  // +-------------------------------------------------------------------------+
  std::size_t get_headers_length() const {
    std::size_t pos = sln_len_;
    std::size_t end = sln_len_ + hdr_len_;
    std::size_t count = 0;
    while (pos < end) {
      std::size_t colon = pos;
      while (colon < end && memory_[colon] != ':') colon++;
      if (colon >= end) break;
      std::size_t cr = colon + 1;
      while (cr < end && memory_[cr] != '\r') cr++;
      if (cr >= end) break;
      count++;
      // Advance past CRLF (\r\n) to the next header line.
      pos = cr + 2;
    }
    return count;
  }
  // +=========================================================================+
  // | [>] remove_header                                            ( public ) |
  // +=========================================================================+
  // | Removes the first header whose name case-insensitively matches 'k' and  |
  // | compacts the header block. Removing an absent header is an intentional |
  // | no-op (idempotent): unlike get_header, it does not throw when 'k' is not
  // | | present. |
  // +-------------------------------------------------------------------------+
  response& remove_header(std::string_view k) {
    std::size_t line_off, val_off, val_len, line_len;
    if (!find_header(k, line_off, val_off, val_len, line_len)) return *this;
    std::size_t tail_off = line_off + line_len;
    std::size_t tail_len = (sln_len_ + hdr_len_) - tail_off;
    std::memmove(&memory_[line_off], &memory_[tail_off], tail_len);
    hdr_len_ -= line_len;
    return *this;
  }
  // +=========================================================================+
  // | [>] set_body                                                 ( public ) |
  // +=========================================================================+
  response& set_body(std::string_view sv) {
    std::size_t body_size = sv.size();
    if (body_size > kMaxBodySizeInMemory) {
      throw std::out_of_range("not enough space to set body!");
    }
    std::memcpy(&memory_[bdy_beg_], sv.data(), body_size);
    bdy_len_ = body_size;
    body_reader_.reset();
    // Keep Content-Length in sync with the actual body size.
    // both overwrites the bodiless default set by sln() and updates any
    // value the caller may have set explicitly beforehand.
    set_header("Content-Length", body_size);
    return *this;
  }
  // +=========================================================================+
  // | [>] set_body                                                 ( public ) |
  // +=========================================================================+
  template <typename T>
    requires std::is_arithmetic_v<T>
  response& set_body(T&& val) {
    return set_body(std::to_string(val));
  }
  // +=========================================================================+
  // | [>] set_body                                                 ( public ) |
  // +---------------------------------------------------------------------------
  // | Assigns a streaming body from a body_reader (obtained via              |
  // | body_serializer::release_reader()). The inline body buffer is cleared. |
  // | If the reader reports a known size, Content-Length is set; otherwise   |
  // | Transfer-Encoding: chunked is used (RFC 9112 §6.1).                    |
  // +=========================================================================+
  response& set_body(body_reader&& src) {
    if (src.failed()) {
      // Reader has a storage I/O error; do not corrupt the response wire
      // format.
      return *this;
    }
    bdy_len_ = 0;
    body_reader_ = std::move(src);
    if (auto sz = body_reader_->size()) {
      remove_header("Transfer-Encoding");
      set_header("Content-Length", *sz);
    } else {
      remove_header("Content-Length");
      set_header("Transfer-Encoding", "chunked");
    }
    return *this;
  }
  // +=========================================================================+
  // | [>] STATUS-LINE-METHODs                                      ( public ) |
  // +=========================================================================+
  INLINE response& continue_100() {
    return sln(status_lines::k100,  // RFC 9112 §6.2.1
               SC_100_CONTINUE);
  }
  INLINE response& switching_protocols_101() {
    return sln(status_lines::k101,  // RFC 9112 §6.2.2
               SC_101_SWITCHING_PROTOCOLS);
  }
  INLINE response& ok_200() {
    return sln(status_lines::k200,  // RFC 9112 §6.3.1
               SC_200_OK);
  }
  INLINE response& created_201() {
    return sln(status_lines::k201,  // RFC 9112 §6.3.2
               SC_201_CREATED);
  }
  INLINE response& accepted_202() {
    return sln(status_lines::k202,  // RFC 9112 §6.3.3
               SC_202_ACCEPTED);
  }
  INLINE response& non_authoritative_info_203() {
    return sln(status_lines::k203,  // RFC 9112 §6.3.4
               SC_203_NON_AUTHORITATIVE_INFORMATION);
  }
  INLINE response& no_content_204() {
    return sln(status_lines::k204,  // RFC 9112 §6.3.5
               SC_204_NO_CONTENT);
  }
  INLINE response& reset_content_205() {
    return sln(status_lines::k205,  // RFC 9112 §6.3.6
               SC_205_RESET_CONTENT);
  }
  INLINE response& partial_content_206() {
    return sln(status_lines::k206,  // RFC 9112 §6.3.7
               SC_206_PARTIAL_CONTENT);
  }
  INLINE response& multiple_choices_300() {
    return sln(status_lines::k300,  // RFC 9112 §6.4.1
               SC_300_MULTIPLE_CHOICES);
  }
  INLINE response& moved_permanently_301() {
    return sln(status_lines::k301,  // RFC 9112 §6.4.2
               SC_301_MOVED_PERMANENTLY);
  }
  INLINE response& found_302() {
    return sln(status_lines::k302,  // RFC 9112 §6.4.3
               SC_302_FOUND);
  }
  INLINE response& see_other_303() {
    return sln(status_lines::k303,  // RFC 9112 §6.4.4
               SC_303_SEE_OTHER);
  }
  INLINE response& not_modified_304() {
    return sln(status_lines::k304,  // RFC 9112 §6.4.5
               SC_304_NOT_MODIFIED);
  }
  INLINE response& use_proxy_305() {
    return sln(status_lines::k305,  // RFC 9112 §6.4.6
               SC_305_USE_PROXY);
  }
  INLINE response& unused_306() {
    return sln(status_lines::k306,  // RFC 9112 §6.4.7
               SC_306_UNUSED);
  }
  INLINE response& temporary_redirect_307() {
    return sln(status_lines::k307,  // RFC 9112 §6.4.8
               SC_307_TEMPORARY_REDIRECT);
  }
  INLINE response& permanent_redirect_308() {
    return sln(status_lines::k308,  // RFC 9112 §6.4.9
               SC_308_PERMANENT_REDIRECT);
  }
  INLINE response& bad_request_400() {
    return sln(status_lines::k400,  // RFC 9112 §6.5.1
               SC_400_BAD_REQUEST);
  }
  INLINE response& unauthorized_401() {
    return sln(status_lines::k401,  // RFC 9112 §6.5.2
               SC_401_UNAUTHORIZED);
  }
  INLINE response& payment_required_402() {
    return sln(status_lines::k402,  // RFC 9112 §6.5.3
               SC_402_PAYMENT_REQUIRED);
  }
  INLINE response& forbidden_403() {
    return sln(status_lines::k403,  // RFC 9112 §6.5.4
               SC_403_FORBIDDEN);
  }
  INLINE response& not_found_404() {
    return sln(status_lines::k404,  // RFC 9112 §6.5.5
               SC_404_NOT_FOUND);
  }
  INLINE response& method_not_allowed_405() {
    return sln(status_lines::k405,  // RFC 9112 §6.5.6
               SC_405_METHOD_NOT_ALLOWED);
  }
  INLINE response& not_acceptable_406() {
    return sln(status_lines::k406,  // RFC 9112 §6.5.7
               SC_406_NOT_ACCEPTABLE);
  }
  INLINE response& proxy_auth_required_407() {
    return sln(status_lines::k407,  // RFC 9112 §6.5.8
               SC_407_PROXY_AUTHENTICATION_REQUIRED);
  }
  INLINE response& request_timeout_408() {
    return sln(status_lines::k408,  // RFC 9112 §6.5.9
               SC_408_REQUEST_TIMEOUT);
  }
  INLINE response& conflict_409() {
    return sln(status_lines::k409,  // RFC 9112 §6.5.10
               SC_409_CONFLICT);
  }
  INLINE response& gone_410() {
    return sln(status_lines::k410,  // RFC 9112 §6.5.11
               SC_410_GONE);
  }
  INLINE response& length_required_411() {
    return sln(status_lines::k411,  // RFC 9112 §6.5.12
               SC_411_LENGTH_REQUIRED);
  }
  INLINE response& precondition_failed_412() {
    return sln(status_lines::k412,  // RFC 9112 §6.5.13
               SC_412_PRECONDITION_FAILED);
  }
  INLINE response& content_too_large_413() {
    return sln(status_lines::k413,  // RFC 9112 §6.5.14
               SC_413_CONTENT_TOO_LARGE);
  }
  INLINE response& uri_too_long_414() {
    return sln(status_lines::k414,  // RFC 9112 §6.5.15
               SC_414_URI_TOO_LONG);
  }
  INLINE response& unsupported_media_type_415() {
    return sln(status_lines::k415,  // RFC 9112 §6.5.16
               SC_415_UNSUPPORTED_MEDIA_TYPE);
  }
  INLINE response& range_not_satisfiable_416() {
    return sln(status_lines::k416,  // RFC 9112 §6.5.17
               SC_416_RANGE_NOT_SATISFIABLE);
  }
  INLINE response& expectation_failed_417() {
    return sln(status_lines::k417,  // RFC 9112 §6.5.18
               SC_417_EXPECTATION_FAILED);
  }
  INLINE response& unused_418() {
    return sln(status_lines::k418,  // RFC 9112 §6.5.19
               SC_418_IM_A_TEAPOT);
  }
  INLINE response& misdirected_request_421() {
    return sln(status_lines::k421,  // RFC 9112 §6.5.20
               SC_421_MISDIRECTED_REQUEST);
  }
  INLINE response& unprocessable_content_422() {
    return sln(status_lines::k422,  // RFC 9112 §6.5.21
               SC_422_UNPROCESSABLE_CONTENT);
  }
  INLINE response& upgrade_required_426() {
    return sln(status_lines::k426,  // RFC 9112 §6.5.22
               SC_426_UPGRADE_REQUIRED);
  }
  INLINE response& internal_server_error_500() {
    return sln(status_lines::k500,  // RFC 9112 §6.6.1
               SC_500_INTERNAL_SERVER_ERROR);
  }
  INLINE response& not_implemented_501() {
    return sln(status_lines::k501,  // RFC 9112 §6.6.2
               SC_501_NOT_IMPLEMENTED);
  }
  INLINE response& bad_gateway_502() {
    return sln(status_lines::k502,  // RFC 9112 §6.6.3
               SC_502_BAD_GATEWAY);
  }
  INLINE response& service_unavailable_503() {
    return sln(status_lines::k503,  // RFC 9112 §6.6.4
               SC_503_SERVICE_UNAVAILABLE);
  }
  INLINE response& gateway_timeout_504() {
    return sln(status_lines::k504,  // RFC 9112 §6.6.5
               SC_504_GATEWAY_TIMEOUT);
  }
  INLINE response& http_version_not_supported_505() {
    return sln(status_lines::k505,  // RFC 9112 §6.6.6
               SC_505_HTTP_VERSION_NOT_SUPPORTED);
  }

 private:
  // +=========================================================================+
  // | [>] CONSTANTs                                                ( public ) |
  // +=========================================================================+
  static constexpr std::size_t kMaxSizeInMemory = 4096;
  static constexpr std::size_t kMaxBodySizeInMemory = 2048;
  // +=========================================================================+
  // | [>] tolower_ascii                                           ( private ) |
  // +=========================================================================+
  static constexpr char tolower_ascii(char c) noexcept {
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c;
  }
  // +=========================================================================+
  // | [>] iequals                                                 ( private ) |
  // +=========================================================================+
  static constexpr bool iequals(std::string_view a, std::string_view b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); i++) {
      if (tolower_ascii(a[i]) != tolower_ascii(b[i])) return false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] find_header                                             ( private ) |
  // +=========================================================================+
  // | Scans the serialized header block [sln_len_, sln_len_ + hdr_len_) for   |
  // | the first header whose name case-insensitively matches 'k'. On success  |
  // | reports the line start, value start, value length and full line length  |
  // | (CRLF included). Reused by get_header/set_header/remove_header.         |
  // +-------------------------------------------------------------------------+
  bool find_header(std::string_view k, std::size_t& line_off,
                   std::size_t& val_off, std::size_t& val_len,
                   std::size_t& line_len) const {
    std::size_t pos = sln_len_;
    std::size_t end = sln_len_ + hdr_len_;
    while (pos < end) {
      std::size_t name_beg = pos;
      std::size_t colon = pos;
      while (colon < end && memory_[colon] != ':') colon++;
      if (colon >= end) break;
      std::size_t val_beg = colon + 2;
      std::size_t cr = val_beg;
      while (cr < end && memory_[cr] != '\r') cr++;
      if (cr >= end) break;
      std::string_view name(&memory_[name_beg], colon - name_beg);
      if (iequals(name, k)) {
        line_off = name_beg;
        val_off = val_beg;
        val_len = cr - val_beg;
        line_len = (cr + 2) - name_beg;
        return true;
      }
      // Advance past CRLF (\r\n) to the next header line.
      pos = cr + 2;
    }
    return false;
  }
  // +=========================================================================+
  // | [>] sln                                                      ( public ) |
  // +=========================================================================+
  response& sln(auto&& status_line, int status_code) {
    std::size_t len = strlen(status_line);
    // Reset framing state first, then copy the status line only if it fits
    // before the body region. This keeps the object in a coherent state even
    // if an oversized status line is ever supplied.
    hdr_len_ = 0;
    bdy_len_ = 0;
    body_reader_.reset();
    if (len > bdy_beg_) {
      sln_len_ = 0;
      throw std::out_of_range("not enough space to set status line!");
    }
    sln_len_ = len;
    std::memcpy(memory_, status_line, sln_len_);
    // Default Content-Length to 0 for this (still bodiless) status line; a
    // later set_body() call will overwrite it with the real size. Skipped for
    // 1xx and 204, where Content-Length is forbidden (RFC 9110 S8.6), and for
    // 304, whose value must mirror a hypothetical 200 response this class has
    // no way to know.
    bool is_informational = status_code < SC_200_OK;
    if (!is_informational && status_code != SC_204_NO_CONTENT &&
        status_code != SC_304_NOT_MODIFIED) {
      set_header("Content-Length", 0);
    }
    return *this;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTES                                               ( public ) |
  // +=========================================================================+
  char memory_[kMaxSizeInMemory]{0};
  std::size_t sln_len_{0};
  std::size_t hdr_len_{0};
  std::size_t bdy_beg_{kMaxSizeInMemory - kMaxBodySizeInMemory};
  std::size_t bdy_len_{0};
  std::optional<body_reader> body_reader_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
