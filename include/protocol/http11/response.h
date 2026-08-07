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

#ifndef martianlabs_doba_protocol_http11_response_h
#define martianlabs_doba_protocol_http11_response_h

#include <optional>
#include <stdexcept>
#include <utility>

#include "common/date_server.h"
#include "platform.h"
#include "protocol/http11/body/writer.h"
#include "protocol/http11/header_names.h"
#include "protocol/http11/limits.h"
#include "protocol/serialization.h"
#include "status_codes.h"
#include "status_lines.h"

namespace martianlabs::doba::protocol::http11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] response                                                    ( class ) |
// +---------------------------------------------------------------------------+
// | This class holds for the http 1.1 response implementation.                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class response {
 public:
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +=========================================================================+
  using serialized_type = protocol::serialization_result;
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
  // +=========================================================================+
  // | Finalizes and transfers the wire prefix plus an optional streaming body |
  // | to the transport. It never drains a streaming body; the receiver owns   |
  // | the reader and controls bounded reads after response is destroyed.      |
  // +=========================================================================+
  [[nodiscard]] std::unique_ptr<protocol::serialization_result> serialize() {
    if (!has_date_header_) {
      add_header(header_names::kDate, common::date_server::get().current());
    }
    // RFC 9110 S8.6/S15.3.5/S15.4.5: 1xx, 204 and 304 responses must never
    // carry a message body, regardless of what a handler may have set via
    // set_body(). 1xx/204 must not advertise any body framing at all, so
    // Content-Length/Transfer-Encoding are stripped; 304 may still describe
    // the resource via Content-Length (mirroring a hypothetical 200), but
    // Transfer-Encoding is meaningless without an actual chunked body.
    bool is_informational = status_code_ < SC_200_OK;
    bool must_omit_body = is_informational ||
                          status_code_ == SC_204_NO_CONTENT ||
                          status_code_ == SC_304_NOT_MODIFIED;
    if (must_omit_body) {
      remove_header(header_names::kTransferEncoding);
      if (is_informational || status_code_ == SC_204_NO_CONTENT) {
        remove_header(header_names::kContentLength);
      }
    }
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
    auto result = std::make_unique<protocol::serialization_result>();
    result->prefix.assign(memory_, sln_plus_hdr_len);
    if (!must_omit_body && bdy_len_ > 0) {
      result->prefix.append(&memory_[bdy_beg_], bdy_len_);
    }
    if (bdy_writer_.has_value()) {
      if (!must_omit_body) {
        // Finalizes the framing and transfers the accumulated bytes to the
        // transport; the writer is consumed, so this response no longer owns
        // a body afterwards.
        result->source.emplace(bdy_writer_->release());
      }
      bdy_writer_.reset();
    }
    return result;
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
      // Not enough space to add this header without overrunning the body
      // region!
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
    if (iequals(k, header_names::kDate)) has_date_header_ = true;
    return *this;
  }
  // +=========================================================================+
  // | [>] add_header                                               ( public ) |
  // +=========================================================================+
  template <typename T>
    requires std::is_arithmetic_v<T>
  response& add_header(std::string_view key, const T& val) {
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
        // Not enough space to grow this header without overrunning the body!
        throw std::out_of_range("not enough space to set header!");
      }
    }
    // Relocate the tail to make room for (or reclaim space from) the new value,
    // then write the new value in place. memmove tolerates overlap.
    std::memmove(&memory_[val_off + new_v_size], &memory_[tail_off], tail_len);
    std::memcpy(&memory_[val_off], v.data(), new_v_size);
    hdr_len_ = hdr_len_ - val_len + new_v_size;
    if (iequals(k, header_names::kDate)) has_date_header_ = true;
    return *this;
  }
  // +=========================================================================+
  // | [>] set_header                                               ( public ) |
  // +=========================================================================+
  template <typename T>
    requires std::is_arithmetic_v<T>
  response& set_header(std::string_view key, const T& val) {
    return set_header(key, std::to_string(val));
  }
  // +=========================================================================+
  // | [>] has_header                                               ( public ) |
  // +=========================================================================+
  // | Returns true if a header whose name case-insensitively matches 'k' is   |
  // | present. Lets callers probe for a header without incurring the cost (or |
  // | control flow) of catching the exception thrown by get_header.           |
  // +-------------------------------------------------------------------------+
  bool has_header(std::string_view k) const {
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
  // | compacts the header block. Removing an absent header is an intentional  |
  // | no-op (idempotent): unlike get_header, it does not throw when 'k' is    |
  // | not present.                                                            |
  // +-------------------------------------------------------------------------+
  response& remove_header(std::string_view k) {
    std::size_t line_off, val_off, val_len, line_len;
    if (!find_header(k, line_off, val_off, val_len, line_len)) return *this;
    std::size_t tail_off = line_off + line_len;
    std::size_t tail_len = (sln_len_ + hdr_len_) - tail_off;
    std::memmove(&memory_[line_off], &memory_[tail_off], tail_len);
    hdr_len_ -= line_len;
    if (iequals(k, header_names::kDate)) has_date_header_ = false;
    return *this;
  }
  // +=========================================================================+
  // | [>] set_body                                                 ( public ) |
  // +=========================================================================+
  // | Stores the supplied payload as the response body. Small payloads land   |
  // | in the in-memory body region (fast path); anything that does not fit is |
  // | transparently spilled into an internally created raw body::body_writer, |
  // | which serialize() hands over to the transport as a streaming source.    |
  // +-------------------------------------------------------------------------+
  response& set_body(std::string_view sv) {
    std::size_t body_size = sv.size();
    reset_body();
    if (body_size <= limits::kMaxResponseBodySizeInMemory) {
      std::memcpy(&memory_[bdy_beg_], sv.data(), body_size);
      bdy_len_ = body_size;
      set_header("Content-Length", body_size);
      return *this;
    }
    // Payload does not fit in the in-memory body region: spill it into a raw
    // (Content-Length framed) writer that may itself offload to disk.
    auto writer = body::body_writer::raw();
    if (!writer.write(sv)) {
      throw std::runtime_error("unable to write body!");
    }
    bdy_writer_.emplace(std::move(writer));
    apply_body_framing();
    return *this;
  }
  // +=========================================================================+
  // | [>] set_body                                                 ( public ) |
  // +=========================================================================+
  // | Adopts a caller-built body::body_writer (possibly already written to)   |
  // | as the response body, replacing any previously set body. Framing        |
  // | headers are derived from the writer: chunked writers get                |
  // | 'Transfer-Encoding: chunked', raw ones a 'Content-Length' matching the  |
  // | bytes written so far.                                                   |
  // +-------------------------------------------------------------------------+
  response& set_body(body::body_writer&& writer) {
    reset_body();
    bdy_writer_.emplace(std::move(writer));
    apply_body_framing();
    return *this;
  }
  // +=========================================================================+
  // | [>] clear_body                                               ( public ) |
  // +=========================================================================+
  // | Discards any body bytes previously set via set_body(), and removes the  |
  // | framing headers that described it (Content-Length/Transfer-Encoding),   |
  // | leaving the response as if set_body() had never been called. Symmetric  |
  // | counterpart to set_body(): it never leaves a framing header pointing    |
  // | to bytes that are no longer sent.                                       |
  // +-------------------------------------------------------------------------+
  response& clear_body() {
    bdy_len_ = 0;
    bdy_writer_.reset();
    remove_header(header_names::kContentLength);
    remove_header(header_names::kTransferEncoding);
    return *this;
  }
  // +=========================================================================+
  // | [>] STATUS-LINEs                                             ( public ) |
  // +=========================================================================+
  response& continue_100() {
    // 100_CONTINUE
    return sln(status_lines::k100, SC_100_CONTINUE);
  }
  response& switching_protocols_101() {
    // 101_SWITCHING_PROTOCOLS
    return sln(status_lines::k101, SC_101_SWITCHING_PROTOCOLS);
  }
  response& ok_200() {
    // 200_OK
    return sln(status_lines::k200, SC_200_OK);
  }
  response& created_201() {
    // 201_CREATED
    return sln(status_lines::k201, SC_201_CREATED);
  }
  response& accepted_202() {
    // 202_ACCEPTED
    return sln(status_lines::k202, SC_202_ACCEPTED);
  }
  response& non_authoritative_info_203() {
    // 203_NON_AUTHORITATIVE_INFORMATION
    return sln(status_lines::k203, SC_203_NON_AUTHORITATIVE_INFORMATION);
  }
  response& no_content_204() {
    // 204_NO_CONTENT
    return sln(status_lines::k204, SC_204_NO_CONTENT);
  }
  response& reset_content_205() {
    // 205_RESET_CONTENT
    return sln(status_lines::k205, SC_205_RESET_CONTENT);
  }
  response& partial_content_206() {
    // 206_PARTIAL_CONTENT
    return sln(status_lines::k206, SC_206_PARTIAL_CONTENT);
  }
  response& multiple_choices_300() {
    // 300_MULTIPLE_CHOICES
    return sln(status_lines::k300, SC_300_MULTIPLE_CHOICES);
  }
  response& moved_permanently_301() {
    // 301_MOVED_PERMANENTLY
    return sln(status_lines::k301, SC_301_MOVED_PERMANENTLY);
  }
  response& found_302() {
    // 302_FOUND
    return sln(status_lines::k302, SC_302_FOUND);
  }
  response& see_other_303() {
    // 303_SEE_OTHER
    return sln(status_lines::k303, SC_303_SEE_OTHER);
  }
  response& not_modified_304() {
    // 304_NOT_MODIFIED
    return sln(status_lines::k304, SC_304_NOT_MODIFIED);
  }
  response& use_proxy_305() {
    // 305_USE_PROXY
    return sln(status_lines::k305, SC_305_USE_PROXY);
  }
  response& unused_306() {
    // 306_UNUSED
    return sln(status_lines::k306, SC_306_UNUSED);
  }
  response& temporary_redirect_307() {
    // 307_TEMPORARY_REDIRECT
    return sln(status_lines::k307, SC_307_TEMPORARY_REDIRECT);
  }
  response& permanent_redirect_308() {
    // 308_PERMANENT_REDIRECT
    return sln(status_lines::k308, SC_308_PERMANENT_REDIRECT);
  }
  response& bad_request_400() {
    // 400_BAD_REQUEST
    return sln(status_lines::k400, SC_400_BAD_REQUEST);
  }
  response& unauthorized_401() {
    // 401_UNAUTHORIZED
    return sln(status_lines::k401, SC_401_UNAUTHORIZED);
  }
  response& payment_required_402() {
    // 402_PAYMENT_REQUIRED
    return sln(status_lines::k402, SC_402_PAYMENT_REQUIRED);
  }
  response& forbidden_403() {
    // 403_FORBIDDEN
    return sln(status_lines::k403, SC_403_FORBIDDEN);
  }
  response& not_found_404() {
    // 404_NOT_FOUND
    return sln(status_lines::k404, SC_404_NOT_FOUND);
  }
  response& method_not_allowed_405() {
    // 405_METHOD_NOT_ALLOWED
    return sln(status_lines::k405, SC_405_METHOD_NOT_ALLOWED);
  }
  response& not_acceptable_406() {
    // 406_NOT_ACCEPTABLE
    return sln(status_lines::k406, SC_406_NOT_ACCEPTABLE);
  }
  response& proxy_auth_required_407() {
    // 407_PROXY_AUTHENTICATION_REQUIRED
    return sln(status_lines::k407, SC_407_PROXY_AUTHENTICATION_REQUIRED);
  }
  response& request_timeout_408() {
    // 408_REQUEST_TIMEOUT
    return sln(status_lines::k408, SC_408_REQUEST_TIMEOUT);
  }
  response& conflict_409() {
    // 409_CONFLICT
    return sln(status_lines::k409, SC_409_CONFLICT);
  }
  response& gone_410() {
    // 410_GONE
    return sln(status_lines::k410, SC_410_GONE);
  }
  response& length_required_411() {
    // 411_LENGTH_REQUIRED
    return sln(status_lines::k411, SC_411_LENGTH_REQUIRED);
  }
  response& precondition_failed_412() {
    // 412_PRECONDITION_FAILED
    return sln(status_lines::k412, SC_412_PRECONDITION_FAILED);
  }
  response& content_too_large_413() {
    // 413_CONTENT_TOO_LARGE
    return sln(status_lines::k413, SC_413_CONTENT_TOO_LARGE);
  }
  response& uri_too_long_414() {
    // 414_URI_TOO_LONG
    return sln(status_lines::k414, SC_414_URI_TOO_LONG);
  }
  response& unsupported_media_type_415() {
    // 415_UNSUPPORTED_MEDIA_TYPE
    return sln(status_lines::k415, SC_415_UNSUPPORTED_MEDIA_TYPE);
  }
  response& range_not_satisfiable_416() {
    // 416_RANGE_NOT_SATISFIABLE
    return sln(status_lines::k416, SC_416_RANGE_NOT_SATISFIABLE);
  }
  response& expectation_failed_417() {
    // 417_EXPECTATION_FAILED
    return sln(status_lines::k417, SC_417_EXPECTATION_FAILED);
  }
  response& unused_418() {
    // 418_IM_A_TEAPOT
    return sln(status_lines::k418, SC_418_IM_A_TEAPOT);
  }
  response& misdirected_request_421() {
    // 421_MISDIRECTED_REQUEST
    return sln(status_lines::k421, SC_421_MISDIRECTED_REQUEST);
  }
  response& unprocessable_content_422() {
    // 422_UNPROCESSABLE_CONTENT
    return sln(status_lines::k422, SC_422_UNPROCESSABLE_CONTENT);
  }
  response& upgrade_required_426() {
    // 426_UPGRADE_REQUIRED
    return sln(status_lines::k426, SC_426_UPGRADE_REQUIRED);
  }
  response& request_header_fields_too_large_431() {
    // 431_REQUEST_HEADER_FIELDS_TOO_LARGE
    return sln(status_lines::k431, SC_431_REQUEST_HEADER_FIELDS_TOO_LARGE);
  }
  response& internal_server_error_500() {
    // 500_INTERNAL_SERVER_ERROR
    return sln(status_lines::k500, SC_500_INTERNAL_SERVER_ERROR);
  }
  response& not_implemented_501() {
    // 501_NOT_IMPLEMENTED
    return sln(status_lines::k501, SC_501_NOT_IMPLEMENTED);
  }
  response& bad_gateway_502() {
    // 502_BAD_GATEWAY
    return sln(status_lines::k502, SC_502_BAD_GATEWAY);
  }
  response& service_unavailable_503() {
    // 503_SERVICE_UNAVAILABLE
    return sln(status_lines::k503, SC_503_SERVICE_UNAVAILABLE);
  }
  response& gateway_timeout_504() {
    // 504_GATEWAY_TIMEOUT
    return sln(status_lines::k504, SC_504_GATEWAY_TIMEOUT);
  }
  response& http_version_not_supported_505() {
    // 505_HTTP_VERSION_NOT_SUPPORTED
    return sln(status_lines::k505, SC_505_HTTP_VERSION_NOT_SUPPORTED);
  }

 private:
  // +=========================================================================+
  // | [>] CONSTANTs                                               ( private ) |
  // +=========================================================================+
  // +=========================================================================+
  // | [>] reset_body                                              ( private ) |
  // +=========================================================================+
  // | Drops any previously set body (in-memory payload or adopted writer) and |
  // | clears the framing headers, so every set_body() call fully replaces the |
  // | former body instead of accumulating state.                              |
  // +-------------------------------------------------------------------------+
  void reset_body() {
    bdy_len_ = 0;
    bdy_writer_.reset();
    remove_header("Transfer-Encoding");
    remove_header("Content-Length");
  }
  // +=========================================================================+
  // | [>] apply_body_framing                                      ( private ) |
  // +=========================================================================+
  // | Emits the framing header matching the currently owned body writer:      |
  // | 'Transfer-Encoding: chunked' for chunked writers, otherwise a           |
  // | 'Content-Length' derived from the raw bytes written so far.             |
  // +-------------------------------------------------------------------------+
  void apply_body_framing() {
    if (bdy_writer_->is_chunked()) {
      set_header("Transfer-Encoding", "chunked");
    } else {
      set_header("Content-Length", bdy_writer_->bytes_written());
    }
  }
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
  // | [>] sln                                                     ( private ) |
  // +=========================================================================+
  response& sln(auto&& status_line, int status_code) {
    std::size_t len = strlen(status_line);
    // Reset framing state first, then copy the status line only if it fits
    // before the body region. This keeps the object in a coherent state even
    // if an oversized status line is ever supplied.
    hdr_len_ = 0;
    bdy_len_ = 0;
    has_date_header_ = false;
    bdy_writer_.reset();
    status_code_ = status_code;
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
  // | [>] ATTRIBUTES                                              ( private ) |
  // +=========================================================================+
  char memory_[limits::kMaxResponseSizeInMemory]{0};
  std::size_t sln_len_{0};
  std::size_t hdr_len_{0};
  std::size_t bdy_beg_{limits::kMaxResponseSizeInMemory -
                       limits::kMaxResponseBodySizeInMemory};
  std::size_t bdy_len_{0};
  int status_code_{SC_200_OK};
  bool has_date_header_{false};
  std::optional<body::body_writer> bdy_writer_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
