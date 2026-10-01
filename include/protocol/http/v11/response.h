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

#ifndef martianlabs_doba_protocol_http_v11_response_h
#define martianlabs_doba_protocol_http_v11_response_h

#include <charconv>
#include <cstring>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>

#include "common/date_server.h"
#include "platform.h"
#include "protocol/http/common/helpers.h"
#include "protocol/http/v11/body/writer.h"
#include "protocol/http/v11/policies.h"
#include "protocol/http/common/header_names.h"
#include "protocol/serialization.h"
#include "protocol/http/common/status_codes.h"
#include "status_lines.h"

namespace martianlabs::doba::protocol::http::v11 {
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
  // +-------------------------------------------------------------------------+
  using serialized_type = protocol::serialization_result;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  response(const response&) = delete;
  explicit response(std::span<char> storage) {
    if (storage.size() < policies::kMaxResponseHeadSizeInMemory +
                             policies::kMaxResponseBodySizeInMemory) {
      throw std::invalid_argument("response storage is too small!");
    }
    head_ = storage.first(policies::kMaxResponseHeadSizeInMemory);
    body_ = storage.subspan(policies::kMaxResponseHeadSizeInMemory,
                            policies::kMaxResponseBodySizeInMemory);
  }
  response(response&& in) noexcept
      : head_(in.head_),
        body_(in.body_),
        head_size_(in.head_size_),
        body_size_(in.body_size_),
        sln_len_(in.sln_len_),
        hdr_len_(in.hdr_len_),
        status_code_(in.status_code_),
        has_date_header_(in.has_date_header_),
        has_content_length_header_(in.has_content_length_header_),
        has_transfer_encoding_header_(in.has_transfer_encoding_header_),
        connection_close_count_(in.connection_close_count_),
        content_length_(in.content_length_),
        chunked_(in.chunked_),
        bdy_writer_(std::move(in.bdy_writer_)),
        bdy_reader_(std::move(in.bdy_reader_)) {
    in.sln_len_ = 0;
    in.hdr_len_ = 0;
    in.head_ = {};
    in.body_ = {};
    in.head_size_ = 0;
    in.body_size_ = 0;
    in.status_code_ = 0;
    in.has_date_header_ = false;
    in.connection_close_count_ = 0;
    in.bdy_writer_.reset();
    in.bdy_reader_.reset();
  }
  ~response() = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  response& operator=(const response&) = delete;
  response& operator=(response&& in) noexcept {
    if (this == &in) return *this;
    head_ = in.head_;
    body_ = in.body_;
    head_size_ = in.head_size_;
    body_size_ = in.body_size_;
    sln_len_ = in.sln_len_;
    hdr_len_ = in.hdr_len_;
    status_code_ = in.status_code_;
    has_date_header_ = in.has_date_header_;
    has_content_length_header_ = in.has_content_length_header_;
    has_transfer_encoding_header_ = in.has_transfer_encoding_header_;
    connection_close_count_ = in.connection_close_count_;
    content_length_ = in.content_length_;
    chunked_ = in.chunked_;
    bdy_writer_ = std::move(in.bdy_writer_);
    bdy_reader_ = std::move(in.bdy_reader_);
    in.sln_len_ = 0;
    in.hdr_len_ = 0;
    in.head_ = {};
    in.body_ = {};
    in.head_size_ = 0;
    in.body_size_ = 0;
    in.status_code_ = 0;
    in.has_date_header_ = false;
    in.connection_close_count_ = 0;
    in.bdy_writer_.reset();
    in.bdy_reader_.reset();
    return *this;
  }
  // +=========================================================================+
  // | [>] serialize                                                ( public ) |
  // +-------------------------------------------------------------------------+
  // | Borrows head and body until on_send copies them; moves the source.      |
  // +-------------------------------------------------------------------------+
  [[nodiscard]] protocol::serialization_result serialize() {
    if (!head_size_) throw std::logic_error("response has no status!");
    // +-----------------------------------------------------------------------+
    // | RFC 9110 S8.6/S15.3.5/S15.3.6/S15.4.5: 1xx, 204, 205 and 304          |
    // | responses must never carry a message body, regardless of what a       |
    // | handler may have set via set_body(). 1xx/204 must not advertise any   |
    // | body framing at all, while 205 uses Content-Length: 0. 304 may still  |
    // | describe the resource via Content-Length (mirroring a                 |
    // | hypothetical 200).                                                    |
    // +-----------------------------------------------------------------------+
    bool is_informational = status_code_ < SC_200_OK;
    bool must_omit_body = is_informational ||
                          status_code_ == SC_204_NO_CONTENT ||
                          status_code_ == SC_205_RESET_CONTENT ||
                          status_code_ == SC_304_NOT_MODIFIED;
    if (must_omit_body) {
      while (has_transfer_encoding_header_) {
        remove_header(header_names::kTransferEncoding);
      }
      chunked_ = false;
      if (is_informational || status_code_ == SC_204_NO_CONTENT) {
        while (has_content_length_header_) {
          remove_header(header_names::kContentLength);
        }
        content_length_.reset();
      } else if (status_code_ == SC_205_RESET_CONTENT) {
        while (has_content_length_header_) {
          remove_header(header_names::kContentLength);
        }
        content_length_ = 0;
      }
    }
    if (must_omit_body) body_size_ = 0;
    apply_body_framing();
    if (!has_date_header_) {
      add_date_header();
    }
    std::size_t sln_plus_hdr_len = sln_len_ + hdr_len_;
    std::size_t crlf_bytes = hdr_len_ ? 2 : 4;
    if (sln_plus_hdr_len + crlf_bytes >
        policies::kMaxResponseHeadSizeInMemory) {
      throw std::out_of_range("not enough space to serialize response!");
    }
    protocol::serialization_result result;
    if (bdy_writer_.has_value()) {
      if (!must_omit_body) {
        // Finalizes the framing and transfers the accumulated bytes to the
        // transport; the writer is consumed, so this response no longer owns
        // a body afterwards.
        result.source = std::make_unique<common::reader>();
        *result.source = common::reader(bdy_writer_->release());
      }
      bdy_writer_.reset();
    }
    if (bdy_reader_) {
      if (!must_omit_body) {
        result.source = std::make_unique<common::reader>();
        *result.source = std::move(*bdy_reader_);
      }
      bdy_reader_.reset();
    }
    append_head(hdr_len_ ? "\r\n" : "\r\n\r\n");
    result.head = std::string_view(head_.data(), head_size_);
    result.body = std::string_view(body_.data(), body_size_);
    head_size_ = 0;
    return result;
  }
  // +=========================================================================+
  // | [>] add_header                                               ( public ) |
  // +-------------------------------------------------------------------------+
  response& add_header(std::string_view k, std::string_view v) {
    if (!helpers::is_token(k)) {
      throw std::invalid_argument("invalid header name!");
    }
    for (const char c : v) {
      if (c != '\t' && c != ' ' && !helpers::is_vchar(c) &&
          !helpers::is_obs_text(c)) {
        throw std::invalid_argument("invalid header value!");
      }
    }
    std::size_t k_size = k.size();
    std::size_t v_size = v.size();
    std::size_t space_left =
        policies::kMaxResponseHeadSizeInMemory - sln_len_ - hdr_len_;
    // Bytes written by this call: key + ':' + ' ' + value + '\r' + '\n' = k + v
    // + 4. Reserve the 2 bytes of the header-terminating CRLF.
    if (k_size + v_size + 4 + 2 > space_left) {
      throw std::out_of_range("not enough space to add header!");
    }
    append_head(k);
    append_head(": ");
    append_head(v);
    append_head("\r\n");
    hdr_len_ += k_size + v_size + 4;
    if (iequals(k, header_names::kDate)) has_date_header_ = true;
    if (iequals(k, header_names::kContentLength)) {
      has_content_length_header_ = true;
    }
    if (iequals(k, header_names::kTransferEncoding)) {
      has_transfer_encoding_header_ = true;
    }
    if (iequals(k, header_names::kConnection) && has_close_option(v)) {
      connection_close_count_++;
    }
    return *this;
  }
  // +=========================================================================+
  // | [>] add_header                                               ( public ) |
  // +-------------------------------------------------------------------------+
  template <typename T>
    requires std::is_arithmetic_v<T>
  response& add_header(std::string_view key, const T& val) {
    return add_header(key, std::to_string(val));
  }
  // +=========================================================================+
  // | [>] set_header                                               ( public ) |
  // +-------------------------------------------------------------------------+
  response& set_header(std::string_view k, std::string_view v) {
    if (!helpers::is_token(k)) {
      throw std::invalid_argument("invalid header name!");
    }
    for (const char c : v) {
      if (c != '\t' && c != ' ' && !helpers::is_vchar(c) &&
          !helpers::is_obs_text(c)) {
        throw std::invalid_argument("invalid header value!");
      }
    }
    std::size_t line_off, val_off, val_len, line_len;
    if (!find_header(k, line_off, val_off, val_len, line_len)) {
      // Header not present: reuse add_header to append it.
      return add_header(k, v);
    }
    std::size_t new_v_size = v.size();
    if (new_v_size > val_len) {
      std::size_t grow = new_v_size - val_len;
      std::size_t space_left =
          policies::kMaxResponseHeadSizeInMemory - sln_len_ - hdr_len_;
      if (grow + 2 > space_left) {
        throw std::out_of_range("not enough space to set header!");
      }
    }
    bool had_close_option =
        iequals(k, header_names::kConnection) &&
        has_close_option(std::string_view(&head_[val_off], val_len));
    replace_head(val_off, val_len, v);
    if (had_close_option) connection_close_count_--;
    hdr_len_ = hdr_len_ - val_len + new_v_size;
    if (iequals(k, header_names::kDate)) has_date_header_ = true;
    if (iequals(k, header_names::kConnection) && has_close_option(v)) {
      connection_close_count_++;
    }
    return *this;
  }
  // +=========================================================================+
  // | [>] set_header                                               ( public ) |
  // +-------------------------------------------------------------------------+
  template <typename T>
    requires std::is_arithmetic_v<T>
  response& set_header(std::string_view key, const T& val) {
    return set_header(key, std::to_string(val));
  }
  // +=========================================================================+
  // | [>] has_header                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // | Returns true if a header whose name case-insensitively matches 'k' is   |
  // | present. Lets callers probe for a header without incurring the cost (or |
  // | control flow) of catching the exception thrown by get_header.           |
  // +-------------------------------------------------------------------------+
  bool has_header(std::string_view k) const {
    // Automatic framing headers are emitted only by serialize().
    std::size_t line_off, val_off, val_len, line_len;
    return find_header(k, line_off, val_off, val_len, line_len);
  }
  // +=========================================================================+
  // | [>] wants_connection_close                                   ( public ) |
  // +-------------------------------------------------------------------------+
  bool wants_connection_close() const { return connection_close_count_ != 0; }
  // +=========================================================================+
  // | [>] is_continue_100                                          ( public ) |
  // +-------------------------------------------------------------------------+
  bool is_continue_100() const { return status_code_ == SC_100_CONTINUE; }
  // +=========================================================================+
  // | [>] get_header                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // | Returns the (key, value) pair for the first header whose name           |
  // | case-insensitively matches 'k'. Throws std::runtime_error if the header |
  // | is not present. The returned strings are owned copies: they do NOT      |
  // | alias head_ and stay valid across later mutations of this response.     |
  // +-------------------------------------------------------------------------+
  std::pair<std::string, std::string> get_header(std::string_view k) const {
    std::size_t line_off, val_off, val_len, line_len;
    if (!find_header(k, line_off, val_off, val_len, line_len)) {
      throw std::runtime_error("header not found!");
    }
    return {std::string(k), std::string(&head_[val_off], val_len)};
  }
  // +=========================================================================+
  // | [>] get_header                                               ( public ) |
  // +-------------------------------------------------------------------------+
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
      while (colon < end && head_[colon] != ':') colon++;
      if (colon >= end) break;
      std::size_t val_beg = colon + 2;
      std::size_t cr = val_beg;
      while (cr < end && head_[cr] != '\r') cr++;
      if (cr >= end) break;
      if (current == index) {
        // Copy into caller-owned std::strings (see rationale above).
        return {std::string(&head_[name_beg], colon - name_beg),
                std::string(&head_[val_beg], cr - val_beg)};
      }
      current++;
      // Advance past CRLF (\r\n) to the next header line.
      pos = cr + 2;
    }
    throw std::out_of_range("header index out of range!");
  }
  // +=========================================================================+
  // | [>] get_headers_length                                       ( public ) |
  // +-------------------------------------------------------------------------+
  // | Returns the number of headers currently stored. Enables safe indexed    |
  // | iteration via get_header(index) without relying on catching exceptions. |
  // +-------------------------------------------------------------------------+
  std::size_t get_headers_length() const {
    std::size_t pos = sln_len_;
    std::size_t end = sln_len_ + hdr_len_;
    std::size_t count = 0;
    while (pos < end) {
      std::size_t colon = pos;
      while (colon < end && head_[colon] != ':') colon++;
      if (colon >= end) break;
      std::size_t cr = colon + 1;
      while (cr < end && head_[cr] != '\r') cr++;
      if (cr >= end) break;
      count++;
      // Advance past CRLF (\r\n) to the next header line.
      pos = cr + 2;
    }
    return count;
  }
  // +=========================================================================+
  // | [>] remove_header                                            ( public ) |
  // +-------------------------------------------------------------------------+
  // | Removes the first header whose name case-insensitively matches 'k' and  |
  // | compacts the header block. Removing an absent header is an intentional  |
  // | no-op (idempotent): unlike get_header, it does not throw when 'k' is    |
  // | not present.                                                            |
  // +-------------------------------------------------------------------------+
  response& remove_header(std::string_view k) {
    std::size_t line_off, val_off, val_len, line_len;
    if (!find_header(k, line_off, val_off, val_len, line_len)) return *this;
    if (iequals(k, header_names::kConnection) &&
        has_close_option(std::string_view(&head_[val_off], val_len))) {
      connection_close_count_--;
    }
    erase_head(line_off, line_len);
    hdr_len_ -= line_len;
    if (iequals(k, header_names::kDate)) has_date_header_ = has_header(k);
    if (iequals(k, header_names::kContentLength)) {
      has_content_length_header_ = has_header(k);
    }
    if (iequals(k, header_names::kTransferEncoding)) {
      has_transfer_encoding_header_ = has_header(k);
    }
    return *this;
  }
  // +=========================================================================+
  // | [>] set_body                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  // | Stores small payloads in body_; larger ones use a raw writer.           |
  // +-------------------------------------------------------------------------+
  response& set_body(std::string_view sv) {
    std::size_t body_size = sv.size();
    reset_body();
    if (body_size <= policies::kMaxResponseBodySizeInMemory) {
      if (body_size) std::memcpy(body_.data(), sv.data(), body_size);
      body_size_ = body_size;
      content_length_ = body_size;
      return *this;
    }
    // The raw writer may offload the body to disk.
    auto writer = body::body_writer::raw();
    if (!writer.write(sv)) {
      throw std::runtime_error("unable to write body!");
    }
    bdy_writer_.emplace(std::move(writer));
    content_length_ = bdy_writer_->bytes_written();
    return *this;
  }
  // +=========================================================================+
  // | [>] set_body                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  // | Adopts a caller-built body::body_writer (possibly already written to)   |
  // | as the response body, replacing any previously set body. Framing        |
  // | headers are emitted at serialization from the writer's mode and size.   |
  // +-------------------------------------------------------------------------+
  response& set_body(body::body_writer&& writer) {
    reset_body();
    bdy_writer_.emplace(std::move(writer));
    chunked_ = bdy_writer_->is_chunked();
    if (!chunked_) content_length_ = bdy_writer_->bytes_written();
    return *this;
  }
  // +=========================================================================+
  // | [>] set_body                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  // | Convenience overload for numeric types: converts the value to a string  |
  // | and stores it as the response body with deferred framing.               |
  // +-------------------------------------------------------------------------+
  template <typename T>
    requires std::is_arithmetic_v<T>
  response& set_body(const T& val) {
    return set_body(std::to_string(val));
  }
  // +=========================================================================+
  // | [>] set_body                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  // length is the exact number of bytes remaining in source.
  response& set_body(common::reader&& source, std::size_t length) {
    reset_body();
    bdy_reader_.emplace(std::move(source));
    content_length_ = length;
    return *this;
  }
  // +=========================================================================+
  // | [>] clear_body                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // | Discards the body and restores empty-response framing.                  |
  // +-------------------------------------------------------------------------+
  response& clear_body() {
    suppress_body();
    content_length_ = 0;
    bool is_informational = status_code_ < SC_200_OK;
    if (is_informational || status_code_ == SC_204_NO_CONTENT ||
        status_code_ == SC_304_NOT_MODIFIED) {
      content_length_.reset();
    }
    chunked_ = false;
    while (has_content_length_header_) {
      remove_header(header_names::kContentLength);
    }
    while (has_transfer_encoding_header_) {
      remove_header(header_names::kTransferEncoding);
    }
    return *this;
  }
  // +=========================================================================+
  // | [>] suppress_body                                            ( public ) |
  // +-------------------------------------------------------------------------+
  // | Discards body bytes while retaining the advertised framing for HEAD.    |
  // +-------------------------------------------------------------------------+
  response& suppress_body() {
    body_size_ = 0;
    bdy_reader_.reset();
    bdy_writer_.reset();
    return *this;
  }
  // +=========================================================================+
  // | [>] STATUS-LINEs                                             ( public ) |
  // +-------------------------------------------------------------------------+
  response& continue_100() {
    // 100_CONTINUE
    return set_status_line(status_lines::k100, SC_100_CONTINUE);
  }
  response& switching_protocols_101() {
    // 101_SWITCHING_PROTOCOLS
    return set_status_line(status_lines::k101, SC_101_SWITCHING_PROTOCOLS);
  }
  response& ok_200() {
    // 200_OK
    return set_status_line(status_lines::k200, SC_200_OK);
  }
  response& created_201() {
    // 201_CREATED
    return set_status_line(status_lines::k201, SC_201_CREATED);
  }
  response& accepted_202() {
    // 202_ACCEPTED
    return set_status_line(status_lines::k202, SC_202_ACCEPTED);
  }
  response& non_authoritative_info_203() {
    // 203_NON_AUTHORITATIVE_INFORMATION
    return set_status_line(status_lines::k203,
                           SC_203_NON_AUTHORITATIVE_INFORMATION);
  }
  response& no_content_204() {
    // 204_NO_CONTENT
    return set_status_line(status_lines::k204, SC_204_NO_CONTENT);
  }
  response& reset_content_205() {
    // 205_RESET_CONTENT
    return set_status_line(status_lines::k205, SC_205_RESET_CONTENT);
  }
  response& partial_content_206() {
    // 206_PARTIAL_CONTENT
    return set_status_line(status_lines::k206, SC_206_PARTIAL_CONTENT);
  }
  response& multiple_choices_300() {
    // 300_MULTIPLE_CHOICES
    return set_status_line(status_lines::k300, SC_300_MULTIPLE_CHOICES);
  }
  response& moved_permanently_301() {
    // 301_MOVED_PERMANENTLY
    return set_status_line(status_lines::k301, SC_301_MOVED_PERMANENTLY);
  }
  response& found_302() {
    // 302_FOUND
    return set_status_line(status_lines::k302, SC_302_FOUND);
  }
  response& see_other_303() {
    // 303_SEE_OTHER
    return set_status_line(status_lines::k303, SC_303_SEE_OTHER);
  }
  response& not_modified_304() {
    // 304_NOT_MODIFIED
    return set_status_line(status_lines::k304, SC_304_NOT_MODIFIED);
  }
  response& use_proxy_305() {
    // 305_USE_PROXY
    return set_status_line(status_lines::k305, SC_305_USE_PROXY);
  }
  response& unused_306() {
    // 306_UNUSED
    return set_status_line(status_lines::k306, SC_306_UNUSED);
  }
  response& temporary_redirect_307() {
    // 307_TEMPORARY_REDIRECT
    return set_status_line(status_lines::k307, SC_307_TEMPORARY_REDIRECT);
  }
  response& permanent_redirect_308() {
    // 308_PERMANENT_REDIRECT
    return set_status_line(status_lines::k308, SC_308_PERMANENT_REDIRECT);
  }
  response& bad_request_400() {
    // 400_BAD_REQUEST
    return set_status_line(status_lines::k400, SC_400_BAD_REQUEST);
  }
  response& unauthorized_401() {
    // 401_UNAUTHORIZED
    return set_status_line(status_lines::k401, SC_401_UNAUTHORIZED);
  }
  response& payment_required_402() {
    // 402_PAYMENT_REQUIRED
    return set_status_line(status_lines::k402, SC_402_PAYMENT_REQUIRED);
  }
  response& forbidden_403() {
    // 403_FORBIDDEN
    return set_status_line(status_lines::k403, SC_403_FORBIDDEN);
  }
  response& not_found_404() {
    // 404_NOT_FOUND
    return set_status_line(status_lines::k404, SC_404_NOT_FOUND);
  }
  response& method_not_allowed_405() {
    // 405_METHOD_NOT_ALLOWED
    return set_status_line(status_lines::k405, SC_405_METHOD_NOT_ALLOWED);
  }
  response& not_acceptable_406() {
    // 406_NOT_ACCEPTABLE
    return set_status_line(status_lines::k406, SC_406_NOT_ACCEPTABLE);
  }
  response& proxy_auth_required_407() {
    // 407_PROXY_AUTHENTICATION_REQUIRED
    return set_status_line(status_lines::k407,
                           SC_407_PROXY_AUTHENTICATION_REQUIRED);
  }
  response& request_timeout_408() {
    // 408_REQUEST_TIMEOUT
    return set_status_line(status_lines::k408, SC_408_REQUEST_TIMEOUT);
  }
  response& conflict_409() {
    // 409_CONFLICT
    return set_status_line(status_lines::k409, SC_409_CONFLICT);
  }
  response& gone_410() {
    // 410_GONE
    return set_status_line(status_lines::k410, SC_410_GONE);
  }
  response& length_required_411() {
    // 411_LENGTH_REQUIRED
    return set_status_line(status_lines::k411, SC_411_LENGTH_REQUIRED);
  }
  response& precondition_failed_412() {
    // 412_PRECONDITION_FAILED
    return set_status_line(status_lines::k412, SC_412_PRECONDITION_FAILED);
  }
  response& content_too_large_413() {
    // 413_CONTENT_TOO_LARGE
    return set_status_line(status_lines::k413, SC_413_CONTENT_TOO_LARGE);
  }
  response& uri_too_long_414() {
    // 414_URI_TOO_LONG
    return set_status_line(status_lines::k414, SC_414_URI_TOO_LONG);
  }
  response& unsupported_media_type_415() {
    // 415_UNSUPPORTED_MEDIA_TYPE
    return set_status_line(status_lines::k415, SC_415_UNSUPPORTED_MEDIA_TYPE);
  }
  response& range_not_satisfiable_416() {
    // 416_RANGE_NOT_SATISFIABLE
    return set_status_line(status_lines::k416, SC_416_RANGE_NOT_SATISFIABLE);
  }
  response& expectation_failed_417() {
    // 417_EXPECTATION_FAILED
    return set_status_line(status_lines::k417, SC_417_EXPECTATION_FAILED);
  }
  response& unused_418() {
    // 418_IM_A_TEAPOT
    return set_status_line(status_lines::k418, SC_418_IM_A_TEAPOT);
  }
  response& misdirected_request_421() {
    // 421_MISDIRECTED_REQUEST
    return set_status_line(status_lines::k421, SC_421_MISDIRECTED_REQUEST);
  }
  response& unprocessable_content_422() {
    // 422_UNPROCESSABLE_CONTENT
    return set_status_line(status_lines::k422, SC_422_UNPROCESSABLE_CONTENT);
  }
  response& upgrade_required_426() {
    // 426_UPGRADE_REQUIRED
    return set_status_line(status_lines::k426, SC_426_UPGRADE_REQUIRED);
  }
  response& request_header_fields_too_large_431() {
    // 431_REQUEST_HEADER_FIELDS_TOO_LARGE
    return set_status_line(status_lines::k431,
                           SC_431_REQUEST_HEADER_FIELDS_TOO_LARGE);
  }
  response& internal_server_error_500() {
    // 500_INTERNAL_SERVER_ERROR
    return set_status_line(status_lines::k500, SC_500_INTERNAL_SERVER_ERROR);
  }
  response& not_implemented_501() {
    // 501_NOT_IMPLEMENTED
    return set_status_line(status_lines::k501, SC_501_NOT_IMPLEMENTED);
  }
  response& bad_gateway_502() {
    // 502_BAD_GATEWAY
    return set_status_line(status_lines::k502, SC_502_BAD_GATEWAY);
  }
  response& service_unavailable_503() {
    // 503_SERVICE_UNAVAILABLE
    return set_status_line(status_lines::k503, SC_503_SERVICE_UNAVAILABLE);
  }
  response& gateway_timeout_504() {
    // 504_GATEWAY_TIMEOUT
    return set_status_line(status_lines::k504, SC_504_GATEWAY_TIMEOUT);
  }
  response& http_version_not_supported_505() {
    // 505_HTTP_VERSION_NOT_SUPPORTED
    return set_status_line(status_lines::k505,
                           SC_505_HTTP_VERSION_NOT_SUPPORTED);
  }

 private:
  // +=========================================================================+
  // | [>] CONSTANTs                                               ( private ) |
  // +-------------------------------------------------------------------------+
  static constexpr std::string_view kDatePrefix = "Date: ";
  static constexpr std::size_t kDateLength = 29;
  static constexpr std::size_t kDateLineLength =
      kDatePrefix.size() + kDateLength + 2;
  void append_head(std::string_view value) {
    if (!value.empty()) {
      std::memcpy(head_.data() + head_size_, value.data(), value.size());
    }
    head_size_ += value.size();
  }
  void replace_head(std::size_t offset, std::size_t length,
                    std::string_view value) {
    const std::size_t tail = head_size_ - offset - length;
    std::memmove(head_.data() + offset + value.size(),
                 head_.data() + offset + length, tail);
    if (!value.empty()) {
      std::memcpy(head_.data() + offset, value.data(), value.size());
    }
    head_size_ = head_size_ - length + value.size();
  }
  void erase_head(std::size_t offset, std::size_t length) {
    std::memmove(head_.data() + offset, head_.data() + offset + length,
                 head_size_ - offset - length);
    head_size_ -= length;
  }
  response& set_status_line(std::string_view line, int code) {
    if (line.size() > head_.size()) {
      throw std::out_of_range("not enough space to set status line!");
    }
    head_size_ = 0;
    body_size_ = 0;
    sln_len_ = line.size();
    hdr_len_ = 0;
    status_code_ = code;
    has_date_header_ = false;
    has_content_length_header_ = false;
    has_transfer_encoding_header_ = false;
    connection_close_count_ = 0;
    content_length_ = 0;
    chunked_ = false;
    bdy_writer_.reset();
    bdy_reader_.reset();
    append_head(line);
    if (code < SC_200_OK || code == SC_204_NO_CONTENT ||
        code == SC_304_NOT_MODIFIED) {
      content_length_.reset();
    }
    return *this;
  }
  // +=========================================================================+
  // | [>] add_date_header                                         ( private ) |
  // +-------------------------------------------------------------------------+
  void add_date_header() {
    std::size_t space_left =
        policies::kMaxResponseHeadSizeInMemory - sln_len_ - hdr_len_;
    if (kDateLineLength + 2 > space_left) {
      throw std::out_of_range("not enough space to add header!");
    }
    const auto current = common::date_server::get().current();
    append_head(kDatePrefix);
    append_head(std::string_view(current.data(), kDateLength));
    append_head("\r\n");
    hdr_len_ += kDateLineLength;
    has_date_header_ = true;
  }
  // +=========================================================================+
  // | [>] reset_body                                              ( private ) |
  // +-------------------------------------------------------------------------+
  // | Drops any previously set body (in-memory payload or adopted writer) and |
  // | clears the framing headers, so every set_body() call fully replaces the |
  // | former body instead of accumulating state.                              |
  // +-------------------------------------------------------------------------+
  void reset_body() { clear_body(); }
  // +=========================================================================+
  // | [>] apply_body_framing                                      ( private ) |
  // +-------------------------------------------------------------------------+
  // | Emits deferred framing unless an explicit header already supplies it.   |
  // +-------------------------------------------------------------------------+
  void apply_body_framing() {
    // RFC 9112 S6.2: a sender must not combine Content-Length and TE.
    if (has_content_length_header_ &&
        (has_transfer_encoding_header_ || chunked_)) {
      throw std::invalid_argument("conflicting response framing headers!");
    }
    if (has_content_length_header_ || has_transfer_encoding_header_) return;
    std::size_t space_left =
        policies::kMaxResponseHeadSizeInMemory - sln_len_ - hdr_len_;
    if (chunked_) {
      constexpr std::string_view line = "Transfer-Encoding: chunked\r\n";
      if (line.size() + 2 > space_left) {
        throw std::out_of_range("not enough space to serialize response!");
      }
      append_head(line);
      hdr_len_ += line.size();
      has_transfer_encoding_header_ = true;
    } else if (content_length_) {
      constexpr std::string_view prefix = "Content-Length: ";
      char digits[std::numeric_limits<std::size_t>::digits10 + 1];
      const auto converted =
          std::to_chars(digits, digits + sizeof(digits), *content_length_);
      std::size_t digit_count = converted.ptr - digits;
      if (converted.ec != std::errc() ||
          prefix.size() + digit_count + 4 > space_left) {
        throw std::out_of_range("not enough space to serialize response!");
      }
      append_head(prefix);
      append_head(std::string_view(digits, digit_count));
      append_head("\r\n");
      hdr_len_ += prefix.size() + digit_count + 2;
      has_content_length_header_ = true;
    }
  }
  // +=========================================================================+
  // | [>] tolower_ascii                                           ( private ) |
  // +-------------------------------------------------------------------------+
  static constexpr char tolower_ascii(char c) noexcept {
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c;
  }
  // +=========================================================================+
  // | [>] iequals                                                 ( private ) |
  // +-------------------------------------------------------------------------+
  static constexpr bool iequals(std::string_view a, std::string_view b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); i++) {
      if (tolower_ascii(a[i]) != tolower_ascii(b[i])) return false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] has_close_option                                        ( private ) |
  // +-------------------------------------------------------------------------+
  static bool has_close_option(std::string_view value) {
    bool close = false;
    helpers::for_each_list_element(value, [&close](std::string_view option) {
      helpers::ows_ltrim(option);
      helpers::ows_rtrim(option);
      if (helpers::iequals(option, "close")) close = true;
      return true;
    });
    return close;
  }
  // +=========================================================================+
  // | [>] find_header                                             ( private ) |
  // +-------------------------------------------------------------------------+
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
      while (colon < end && head_[colon] != ':') colon++;
      if (colon >= end) break;
      std::size_t val_beg = colon + 2;
      std::size_t cr = val_beg;
      while (cr < end && head_[cr] != '\r') cr++;
      if (cr >= end) break;
      std::string_view name(&head_[name_beg], colon - name_beg);
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
  // | [>] ATTRIBUTES                                              ( private ) |
  // +-------------------------------------------------------------------------+
  std::span<char> head_;
  std::span<char> body_;
  std::size_t head_size_{0};
  std::size_t body_size_{0};
  std::size_t sln_len_{0};
  std::size_t hdr_len_{0};
  int status_code_{0};
  bool has_date_header_{false};
  bool has_content_length_header_{false};
  bool has_transfer_encoding_header_{false};
  std::size_t connection_close_count_{0};
  std::optional<std::size_t> content_length_{0};
  bool chunked_{false};
  std::optional<body::body_writer> bdy_writer_;
  std::optional<common::reader> bdy_reader_;
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
