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

#ifndef martianlabs_doba_protocol_http_v11_request_h
#define martianlabs_doba_protocol_http_v11_request_h

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "platform.h"
#include "common/byte_storage.h"
#include "common/hash_map.h"
#include "protocol/http/v11/context.h"
#include "protocol/http/common/query_parameter.h"
#include "protocol/http/common/method_names.h"
#include "protocol/http/common/header_names.h"
#include "protocol/http/common/header.h"
#include "protocol/http/common/helpers.h"
#include "protocol/http/v11/parsed_types.h"
#include "protocol/http/v11/body/reader.h"
#include "protocol/http/v11/policies.h"
#include "protocol/http/v11/verdict.h"
#include "protocol/http/common/target.h"
#include "protocol/http/common/headers/accept.h"
#include "protocol/http/common/headers/accept_charset.h"
#include "protocol/http/common/headers/accept_encoding.h"
#include "protocol/http/common/headers/accept_language.h"
#include "protocol/http/common/headers/accept_ranges.h"
#include "protocol/http/common/headers/access_control_allow_credentials.h"
#include "protocol/http/common/headers/access_control_allow_headers.h"
#include "protocol/http/common/headers/access_control_allow_methods.h"
#include "protocol/http/common/headers/access_control_allow_origin.h"
#include "protocol/http/common/headers/access_control_expose_headers.h"
#include "protocol/http/common/headers/access_control_max_age.h"
#include "protocol/http/common/headers/access_control_request_headers.h"
#include "protocol/http/common/headers/access_control_request_method.h"
#include "protocol/http/common/headers/age.h"
#include "protocol/http/common/headers/allow.h"
#include "protocol/http/common/headers/authentication_info.h"
#include "protocol/http/common/headers/authorization.h"
#include "protocol/http/common/headers/cache_control.h"
#include "protocol/http/v11/headers/connection.h"
#include "protocol/http/common/headers/content_encoding.h"
#include "protocol/http/common/headers/content_language.h"
#include "protocol/http/v11/headers/content_length.h"
#include "protocol/http/common/headers/content_location.h"
#include "protocol/http/common/headers/content_range.h"
#include "protocol/http/common/headers/content_type.h"
#include "protocol/http/common/headers/cookie.h"
#include "protocol/http/common/headers/date.h"
#include "protocol/http/common/headers/etag.h"
#include "protocol/http/v11/headers/expect.h"
#include "protocol/http/common/headers/expires.h"
#include "protocol/http/v11/headers/forwarded.h"
#include "protocol/http/common/headers/from.h"
#include "protocol/http/v11/headers/host.h"
#include "protocol/http/common/headers/if_match.h"
#include "protocol/http/common/headers/if_modified_since.h"
#include "protocol/http/common/headers/if_none_match.h"
#include "protocol/http/common/headers/if_range.h"
#include "protocol/http/common/headers/if_unmodified_since.h"
#include "protocol/http/common/headers/keep_alive.h"
#include "protocol/http/common/headers/last_modified.h"
#include "protocol/http/common/headers/location.h"
#include "protocol/http/v11/headers/max_forwards.h"
#include "protocol/http/common/headers/origin.h"
#include "protocol/http/common/headers/pragma.h"
#include "protocol/http/common/headers/proxy_connection.h"
#include "protocol/http/common/headers/range.h"
#include "protocol/http/common/headers/referer.h"
#include "protocol/http/common/headers/retry_after.h"
#include "protocol/http/common/headers/sec_websocket_accept.h"
#include "protocol/http/common/headers/sec_websocket_extensions.h"
#include "protocol/http/common/headers/sec_websocket_key.h"
#include "protocol/http/common/headers/sec_websocket_protocol.h"
#include "protocol/http/common/headers/sec_websocket_version.h"
#include "protocol/http/common/headers/server.h"
#include "protocol/http/common/headers/set_cookie.h"
#include "protocol/http/v11/headers/te.h"
#include "protocol/http/v11/headers/trailer.h"
#include "protocol/http/v11/headers/transfer_encoding.h"
#include "protocol/http/v11/headers/upgrade.h"
#include "protocol/http/common/headers/user_agent.h"
#include "protocol/http/common/headers/vary.h"
#include "protocol/http/v11/headers/via.h"
#include "protocol/http/common/headers/www_authenticate.h"
#include "protocol/http/v11/headers/x_forwarded_for.h"
#include "protocol/http/v11/headers/x_forwarded_host.h"
#include "protocol/http/v11/headers/x_forwarded_proto.h"
#include "protocol/http/v11/headers/rules/directives.h"
#include "protocol/http/v11/headers/rules/framing.h"
#include "protocol/http/v11/headers/rules/policy.h"
#include "protocol/http/v11/headers/rules/routing.h"

namespace martianlabs::doba::protocol::http::v11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] request                                                     ( class ) |
// +---------------------------------------------------------------------------+
// | This class holds for the http 1.1 request implementation.                 |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class request {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  request(const request&) = delete;
  request(request&&) noexcept = default;
  ~request() = default;
  // +=========================================================================+
  // | [>] from                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  static request from(
      std::string_view method, std::string_view abs_path, target target_form,
      std::vector<header_view> headers,
      std::vector<query_parameter_view> query_parameters,
      std::optional<std::string_view> host,
      std::optional<std::string_view> port,
      std::optional<helpers::host_type> type,
      std::optional<std::string_view> target_authority_host,
      std::optional<std::string_view> target_authority_port,
      std::optional<helpers::host_type> target_authority_type,
      bool wants_connection_close, std::optional<body::reader> body_reader,
      bool borrow_bytes = false) {
    return request(method, abs_path, target_form, std::move(headers),
                   std::move(query_parameters), host, port, type,
                   target_authority_host, target_authority_port,
                   target_authority_type, wants_connection_close,
                   std::move(body_reader), borrow_bytes);
  }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  request& operator=(const request&) = delete;
  request& operator=(request&&) noexcept = default;
  // +=========================================================================+
  // | [>] get_method                                               ( public ) |
  // +-------------------------------------------------------------------------+
  auto get_method() const { return method_; }
  // +=========================================================================+
  // | [>] get_target                                               ( public ) |
  // +-------------------------------------------------------------------------+
  auto get_target() const { return target_; }
  // +=========================================================================+
  // | [>] get_absolute_path                                        ( public ) |
  // +-------------------------------------------------------------------------+
  auto get_absolute_path() const { return abs_path_; }
  // +=========================================================================+
  // | [>] get_header                                               ( public ) |
  // +-------------------------------------------------------------------------+
  auto get_header(std::size_t i) const { return headers_[i]; }
  // +=========================================================================+
  // | [>] get_header                                               ( public ) |
  // +-------------------------------------------------------------------------+
  auto get_header(std::string_view name) const {
    for (const auto& header : headers_) {
      if (helpers::iequals(header.first, name)) return header;
    }
    throw std::out_of_range("Header not found: " + std::string(name));
  }
  // +=========================================================================+
  // | [>] exist_header                                             ( public ) |
  // +-------------------------------------------------------------------------+
  auto exist_header(std::string_view name) const {
    for (const auto& header : headers_) {
      if (helpers::iequals(header.first, name)) return true;
    }
    return false;
  }
  // +=========================================================================+
  // | [>] get_headers_length                                       ( public ) |
  // +-------------------------------------------------------------------------+
  auto get_headers_length() const { return headers_.size(); }
  // +=========================================================================+
  // | [>] get_query_parameter                                      ( public ) |
  // +-------------------------------------------------------------------------+
  auto get_query_parameter(std::size_t i) const { return query_parameters_[i]; }
  // +=========================================================================+
  // | [>] get_query_parameter                                      ( public ) |
  // +-------------------------------------------------------------------------+
  auto get_query_parameter(std::string_view name) const
      -> std::optional<query_parameter_view> {
    for (const auto& param : query_parameters_) {
      if (param.first == name) return param;
    }
    return std::nullopt;
  }
  // +=========================================================================+
  // | [>] get_query_parameters_length                              ( public ) |
  // +-------------------------------------------------------------------------+
  auto get_query_parameters_length() const { return query_parameters_.size(); }
  // +=========================================================================+
  // | [>] has_hosts                                                ( public ) |
  // +-------------------------------------------------------------------------+
  auto has_host() const { return !host_.empty(); }
  // +=========================================================================+
  // | [>] get_host                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  auto get_host() const { return host_; }
  // +=========================================================================+
  // | [>] get_host_port                                            ( public ) |
  // +-------------------------------------------------------------------------+
  auto get_host_port() const { return host_port_; }
  // +=========================================================================+
  // | [>] get_host_type                                            ( public ) |
  // +-------------------------------------------------------------------------+
  auto get_host_type() const { return host_type_; }
  // +=========================================================================+
  // | [>] has_target_authority                                     ( public ) |
  // +-------------------------------------------------------------------------+
  auto has_target_authority() const { return !ta_host_.empty(); }
  // +=========================================================================+
  // | [>] get_target_authority_host                                ( public ) |
  // +-------------------------------------------------------------------------+
  auto get_target_authority_host() const { return ta_host_; }
  // +=========================================================================+
  // | [>] get_target_authority_port                                ( public ) |
  // +-------------------------------------------------------------------------+
  auto get_target_authority_port() const { return ta_port_; }
  // +=========================================================================+
  // | [>] get_target_authority_type                                ( public ) |
  // +-------------------------------------------------------------------------+
  auto get_target_authority_type() const { return ta_type_; }
  // +=========================================================================+
  // | [>] get_body_reader                                          ( public ) |
  // +-------------------------------------------------------------------------+
  auto get_body_reader() const {
    return body_reader_ ? &*body_reader_ : nullptr;
  }
  // +=========================================================================+
  // | [>] has_body_reader                                          ( public ) |
  // +-------------------------------------------------------------------------+
  auto has_body_reader() const { return body_reader_.has_value(); }
  // +=========================================================================+
  // | [>] wants_connection_close                                   ( public ) |
  // +-------------------------------------------------------------------------+
  auto wants_connection_close() const { return wants_connection_close_; }
  // +=========================================================================+
  // | [>] get_cookie                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // | Looks up a single cookie-pair by name in the (unparsed) Cookie header,  |
  // | if present. Parsing is done on demand; nothing is cached, mirroring how |
  // | headers_ and query_parameters_ are already accessed by linear scan.     |
  // +-------------------------------------------------------------------------+
  std::optional<std::string_view> get_cookie(std::string_view name) const {
    std::string_view raw;
    if (!find_cookie_header(raw)) return std::nullopt;
    std::optional<std::string_view> found;
    for_each_cookie_pair(
        raw, [&](std::string_view cookie_name, std::string_view cookie_value) {
          if (found) return;
          if (cookie_name == name) found = cookie_value;
        });
    return found;
  }
  // +=========================================================================+
  // | [>] get_cookies                                              ( public ) |
  // +-------------------------------------------------------------------------+
  // | Returns every cookie-pair present in the (unparsed) Cookie header,      |
  // | or an empty vector when the header is absent.                           |
  // +-------------------------------------------------------------------------+
  std::vector<std::pair<std::string_view, std::string_view>> get_cookies()
      const {
    std::vector<std::pair<std::string_view, std::string_view>> cookies;
    std::string_view raw;
    if (!find_cookie_header(raw)) return cookies;
    for_each_cookie_pair(
        raw, [&](std::string_view cookie_name, std::string_view cookie_value) {
          cookies.emplace_back(cookie_name, cookie_value);
        });
    return cookies;
  }

  // Async execution must own its bytes before calling user code.
  bool borrows_bytes() const noexcept { return borrowed_; }
  void make_owned() {
    if (!borrowed_) return;
    std::size_t size = 0;
    auto count = [&](std::string_view text) {
      if (text.size() > std::numeric_limits<std::size_t>::max() - size) {
        throw std::length_error("request bytes are too large");
      }
      size += text.size();
    };
    count(method_);
    count(abs_path_);
    count(host_);
    count(host_port_);
    count(ta_host_);
    count(ta_port_);
    for (const auto& field : headers_) { count(field.first); count(field.second); }
    for (const auto& field : query_parameters_) { count(field.first); count(field.second); }
    count(body_bytes_);
    std::vector<char> owned(size);
    std::size_t offset = 0;
    auto copy = [&](std::string_view& text) {
      if (text.empty()) return;
      char* target = owned.data() + offset;
      std::memcpy(target, text.data(), text.size());
      offset += text.size();
      text = {target, text.size()};
    };
    copy(method_);
    copy(abs_path_);
    copy(host_);
    copy(host_port_);
    copy(ta_host_);
    copy(ta_port_);
    for (auto& field : headers_) { copy(field.first); copy(field.second); }
    for (auto& field : query_parameters_) { copy(field.first); copy(field.second); }
    copy(body_bytes_);
    if (body_reader_ && !body_bytes_.empty()) {
      body_reader_->rebind_borrowed(std::as_bytes(
          std::span(body_bytes_.data(), body_bytes_.size())));
    }
    head_bytes_ = std::move(owned);
    borrowed_ = false;
  }
  void rebase_bytes(std::uintptr_t previous, const char* current) noexcept {
    auto rebase = [&](std::string_view& text) {
      if (!text.empty()) {
        const auto offset = reinterpret_cast<std::uintptr_t>(text.data()) - previous;
        text = {current + offset, text.size()};
      }
    };
    rebase(method_);
    if (head_bytes_.empty()) rebase(abs_path_);
    rebase(host_);
    rebase(host_port_);
    rebase(ta_host_);
    rebase(ta_port_);
    for (auto& field : headers_) { rebase(field.first); rebase(field.second); }
    for (auto& field : query_parameters_) { rebase(field.first); rebase(field.second); }
    rebase(body_bytes_);
  }
  bool body_is_stored() const noexcept { return body_storage_.has_value(); }
  std::size_t body_wire_size() const noexcept { return body_wire_size_; }

  // Inline wire bytes stay in the decoder; larger bodies use existing storage.
  bool begin_body(std::size_t memory_threshold, std::size_t declared_size) {
    body_threshold_ = memory_threshold;
    body_wire_size_ = 0;
    body_bytes_ = {};
    if (borrowed_ && memory_threshold && declared_size <= memory_threshold) {
      return true;
    }
    body_storage_.emplace(common::byte_storage_options{memory_threshold, {}});
    return (memory_threshold && declared_size <= memory_threshold) ||
           body_storage_->spill();
  }
  bool append_body(std::string_view bytes) {
    if (bytes.empty()) return true;
    if (!body_storage_ && bytes.size() <= body_threshold_ - body_wire_size_) {
      if (body_bytes_.empty()) body_bytes_ = bytes;
      else body_bytes_ = {body_bytes_.data(), body_bytes_.size() + bytes.size()};
      body_wire_size_ += bytes.size();
      return true;
    }
    if (!body_storage_) {
      body_storage_.emplace(common::byte_storage_options{body_threshold_, {}});
      if (!body_storage_->spill() ||
          !body_storage_->write(body_bytes_.data(), body_bytes_.size())) return false;
      body_bytes_ = {};
    }
    if (!body_storage_->write(bytes.data(), bytes.size())) return false;
    body_wire_size_ += bytes.size();
    return true;
  }
  bool finish_body(bool chunked, std::size_t content_length,
                   std::size_t extension_limit, std::size_t trailer_limit) {
    if (!body_storage_) {
      const auto bytes = std::as_bytes(std::span(body_bytes_.data(), body_bytes_.size()));
      body_reader_ = chunked ? body::reader::chunked(bytes, extension_limit, trailer_limit)
                             : body::reader::raw(bytes, content_length);
      return true;
    }
    body_storage_->finish(body_wire_size_);
    if (!body_storage_->ok()) return false;
    body_storage_->unlink_file();
    body_reader_ =
        chunked ? body::reader::chunked(std::move(*body_storage_),
                                        extension_limit, trailer_limit)
                : body::reader::raw(std::move(*body_storage_), content_length);
    body_storage_.reset();
    return true;
  }

 private:
  // +=========================================================================+
  // | [>] find_cookie_header                                      ( private ) |
  // +-------------------------------------------------------------------------+
  bool find_cookie_header(std::string_view& out) const {
    for (const auto& header : headers_) {
      if (helpers::iequals(header.first, header_names::kCookie)) {
        out = header.second;
        return true;
      }
    }
    return false;
  }
  // +=========================================================================+
  // | [>] for_each_cookie_pair                                    ( private ) |
  // +-------------------------------------------------------------------------+
  // | Splits a Cookie header field-value on the exact "; " separator (RFC     |
  // | 6265 S4.2.1) and invokes fn(name, value) for every cookie-pair found.   |
  // +-------------------------------------------------------------------------+
  template <typename FNty>
  static void for_each_cookie_pair(std::string_view raw, FNty&& fn) {
    std::size_t start = 0;
    std::size_t i = 0;
    while (i < raw.size()) {
      if (raw[i] == ';' && i + 1 < raw.size() && raw[i + 1] == ' ') {
        emit_cookie_pair(raw.substr(start, i - start), fn);
        i += 2;
        start = i;
        continue;
      }
      i++;
    }
    emit_cookie_pair(raw.substr(start), fn);
  }
  // +=========================================================================+
  // | [>] emit_cookie_pair                                        ( private ) |
  // +-------------------------------------------------------------------------+
  template <typename FNty>
  static void emit_cookie_pair(std::string_view pair, FNty&& fn) {
    std::size_t eq = pair.find('=');
    if (eq == std::string_view::npos) return;
    fn(pair.substr(0, eq), pair.substr(eq + 1));
  }
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                ( private ) |
  // +-------------------------------------------------------------------------+
  request(std::string_view method, std::string_view abs_path,
          target target_form, std::vector<header_view> headers,
          std::vector<query_parameter_view> query_parameters,
          std::optional<std::string_view> host,
          std::optional<std::string_view> port,
          std::optional<helpers::host_type> type,
          std::optional<std::string_view> target_authority_host,
          std::optional<std::string_view> target_authority_port,
          std::optional<helpers::host_type> target_authority_type,
          bool wants_connection_close,
          std::optional<body::reader> body_reader, bool borrow_bytes) {
    std::size_t size = 0;
    auto count = [&](std::string_view text) {
      if (text.size() > std::numeric_limits<std::size_t>::max() - size) {
        throw std::length_error("request fields are too large");
      }
      size += text.size();
    };
    borrowed_ = borrow_bytes;
    if (!borrow_bytes) {
      count(method);
      count(abs_path);
      for (const auto& field : headers) {
        count(field.first);
        count(field.second);
      }
      for (const auto& field : query_parameters) {
        count(field.first);
        count(field.second);
      }
      for (const auto field : {host, port, target_authority_host,
                              target_authority_port}) {
        if (field) count(*field);
      }
      head_bytes_.resize(size);
    }
    std::size_t offset = 0;
    auto copy = [&](std::string_view text) {
      if (text.empty()) return std::string_view{};
      if (borrow_bytes) return text;
      char* destination = head_bytes_.data() + offset;
      std::memcpy(destination, text.data(), text.size());
      offset += text.size();
      return std::string_view(destination, text.size());
    };
    method_ = copy(method);
    abs_path_ = copy(abs_path);
    if (borrow_bytes && abs_path_.find('%') != std::string_view::npos) {
      head_bytes_.assign(abs_path_.begin(), abs_path_.end());
      abs_path_ = {head_bytes_.data(), head_bytes_.size()};
    }
    helpers::percent_decode_in_place(abs_path_);
    for (auto& field : headers) {
      field.first = copy(field.first);
      field.second = copy(field.second);
    }
    for (auto& field : query_parameters) {
      field.first = copy(field.first);
      field.second = copy(field.second);
    }
    target_ = target_form;
    headers_ = std::move(headers);
    query_parameters_ = std::move(query_parameters);
    if (host) host_ = copy(*host);
    if (port) host_port_ = copy(*port);
    if (type) host_type_ = *type;
    if (target_authority_host) ta_host_ = copy(*target_authority_host);
    if (target_authority_port) ta_port_ = copy(*target_authority_port);
    if (target_authority_type) ta_type_ = *target_authority_type;
    wants_connection_close_ = wants_connection_close;
    if (body_reader) {
      if (!begin_body(policies::kDefaultRequestBodyMemoryThreshold, 0)) {
        throw std::runtime_error("unable to create request body");
      }
      std::byte bytes[8192];
      for (;;) {
        const auto state = body_reader->read(bytes);
        if (state.has_error || (!state.produced && !state.complete) ||
            !append_body(std::string_view(
                reinterpret_cast<const char*>(bytes), state.produced))) {
          throw std::runtime_error("unable to copy request body");
        }
        if (state.complete) break;
      }
      if (!finish_body(false, body_wire_size_, 0, 0)) {
        throw std::runtime_error("unable to finish request body");
      }
    }
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  std::string_view method_;           // HTTP method (e.g., GET, POST, etc.)
  std::string_view abs_path_;         // absolute path from the request-target
  std::vector<char> head_bytes_;
  std::optional<common::byte_storage> body_storage_;
  std::size_t body_wire_size_{0};
  std::size_t body_threshold_{0};
  std::string_view body_bytes_;
  bool borrowed_{false};
  target target_ = target::kUnknown;  // request-target form
  std::string_view host_;             // host from the Host header
  std::string_view host_port_;        // port from the Host header
  helpers::host_type host_type_ = helpers::host_type::kUnknown;
  std::string_view ta_host_;  // target authority host
  std::string_view ta_port_;  // target authority port
  helpers::host_type ta_type_ = helpers::host_type::kUnknown;
  std::vector<header_view> headers_;                    // vector of headers
  std::vector<query_parameter_view> query_parameters_;  // query parameters
  bool wants_connection_close_ = false;  // protocol decided to close channel
  mutable std::optional<body::reader> body_reader_;
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
