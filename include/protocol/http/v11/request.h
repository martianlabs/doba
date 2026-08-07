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
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "platform.h"
#include "common/hash_map.h"
#include "protocol/http/v11/context.h"
#include "protocol/http/v11/limits.h"
#include "protocol/http/common/request_getter.h"
#include "protocol/http/common/query_parameter.h"
#include "protocol/http/common/method_names.h"
#include "protocol/http/common/header_names.h"
#include "protocol/http/common/header.h"
#include "protocol/http/common/helpers.h"
#include "protocol/http/v11/parsed_types.h"
#include "protocol/http/v11/policies.h"
#include "protocol/http/v11/verdict.h"
#include "protocol/http/common/target.h"
#include "protocol/http/v11/body/reader.h"
#include "protocol/http/v11/body/writer_chunked.h"
#include "protocol/http/v11/body/writer_raw.h"
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
  // +=========================================================================+
  request(const request&) = delete;
  request(request&&) noexcept = delete;
  ~request() { delete[] buffer_; }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  request& operator=(const request&) = delete;
  request& operator=(request&&) noexcept = delete;
  // +=========================================================================+
  // | [>] from                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // | full_buffer            | the entire request head buffer                 |
  // | method                 | the method substring within full_buffer        |
  // | abs_path               | the absolute path substring within full_buffer |
  // | target_form            | the request-target form (origin, absolute, ..) |
  // | headers                | vector of header_view representing the headers |
  // |                        | in the request                                 |
  // | query_parameters       | vector of query_parameter_view representing    |
  // |                        | the query parameters in the request            |
  // | host                   | optional host substring from the Host header   |
  // | port                   | optional port substring from the Host header   |
  // | type                   | optional host_type from the Host header        |
  // | target_authority_host  | optional host substring from the authority     |
  // | target_authority_port  | optional port substring from the authority     |
  // | target_authority_type  | optional host_type from the authority          |
  // +=========================================================================+
  static request_getter<request> from(
      std::string_view full_buffer, std::string_view method,
      std::string_view abs_path, target target_form,
      std::vector<header_view> headers,
      std::vector<query_parameter_view> query_parameters,
      std::optional<std::string_view> host,
      std::optional<std::string_view> port,
      std::optional<helpers::host_type> type,
      std::optional<std::string_view> target_authority_host,
      std::optional<std::string_view> target_authority_port,
      std::optional<helpers::host_type> target_authority_type,
      bool body_chunked_encoding = false, std::size_t body_content_length = 0,
      bool wants_connection_close = false) {
    std::shared_ptr<request> req = std::shared_ptr<request>(new request(
        full_buffer, method, abs_path, target_form, std::move(headers),
        std::move(query_parameters), host, port, type, target_authority_host,
        target_authority_port, target_authority_type, wants_connection_close));
    return [req, body_chunked_encoding, body_content_length](
               std::optional<common::byte_storage> byte_storage) -> auto {
      if (byte_storage) {
        common::reader source(std::move(*byte_storage));
        req->body_reader_ =
            body_chunked_encoding
                ? std::make_shared<body::reader>(
                      body::reader::chunked(std::move(source)))
                : std::make_shared<body::reader>(body::reader::raw(
                      std::move(source), body_content_length));
      }
      return req;
    };
  }
  // +=========================================================================+
  // | [>] GETTERs                                                  ( public ) |
  // +=========================================================================+
  auto get_method() const { return method_; }
  auto get_target() const { return target_; }
  auto get_absolute_path() const { return abs_path_; }
  auto get_header(std::size_t i) const { return headers_[i]; }
  auto get_header(std::string_view name) const {
    for (const auto& header : headers_) {
      if (helpers::iequals(header.first, name)) return header;
    }
    throw std::out_of_range("Header not found: " + std::string(name));
  }
  auto exist_header(std::string_view name) const {
    for (const auto& header : headers_) {
      if (helpers::iequals(header.first, name)) return true;
    }
    return false;
  }
  auto get_headers_length() const { return headers_.size(); }
  auto get_query_parameter(std::size_t i) const { return query_parameters_[i]; }
  auto get_query_parameter(std::string_view name) const
      -> std::optional<query_parameter_view> {
    for (const auto& param : query_parameters_) {
      if (param.first == name) return param;
    }
    return std::nullopt;
  }
  auto get_query_parameters_length() const { return query_parameters_.size(); }
  auto has_host() const { return !host_.empty(); }
  auto get_host() const { return host_; }
  auto get_host_port() const { return host_port_; }
  auto get_host_type() const { return host_type_; }
  auto has_target_authority() const { return !ta_host_.empty(); }
  auto get_target_authority_host() const { return ta_host_; }
  auto get_target_authority_port() const { return ta_port_; }
  auto get_target_authority_type() const { return ta_type_; }
  auto get_body_reader() const { return body_reader_; }
  auto has_body_reader() const { return body_reader_ != nullptr; }
  auto wants_connection_close() const { return wants_connection_close_; }
  // +=========================================================================+
  // | [>] get_cookie                                               ( public ) |
  // +=========================================================================+
  // | Looks up a single cookie-pair by name in the (unparsed) Cookie header,  |
  // | if present. Parsing is done on demand; nothing is cached, mirroring how |
  // | headers_ and query_parameters_ are already accessed by linear scan.     |
  // +=========================================================================+
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
  // +=========================================================================+
  // | Returns every cookie-pair present in the (unparsed) Cookie header,      |
  // | or an empty vector when the header is absent.                           |
  // +=========================================================================+
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

 private:
  // +=========================================================================+
  // | [>] find_cookie_header                                      ( private ) |
  // +=========================================================================+
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
  // +=========================================================================+
  // | Splits a Cookie header field-value on the exact "; " separator (RFC     |
  // | 6265 Ã‚Â§4.2.1) and invokes fn(name, value) for every cookie-pair found.   |
  // +=========================================================================+
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
  // +=========================================================================+
  template <typename FNty>
  static void emit_cookie_pair(std::string_view pair, FNty&& fn) {
    std::size_t eq = pair.find('=');
    if (eq == std::string_view::npos) return;
    fn(pair.substr(0, eq), pair.substr(eq + 1));
  }
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                ( private ) |
  // +=========================================================================+
  request(std::string_view full_buffer, std::string_view method,
          std::string_view abs_path, target target_form,
          std::vector<header_view> headers,
          std::vector<query_parameter_view> query_parameters,
          std::optional<std::string_view> host,
          std::optional<std::string_view> port,
          std::optional<helpers::host_type> type,
          std::optional<std::string_view> target_authority_host,
          std::optional<std::string_view> target_authority_port,
          std::optional<helpers::host_type> target_authority_type,
          bool wants_connection_close = false) {
    buffer_ = new char[full_buffer.size()];
    std::memcpy(buffer_, full_buffer.data(), full_buffer.size());
    char* method_at = buffer_ + (method.data() - full_buffer.data());
    method_ = std::string_view(method_at, method.size());
    char* abs_path_at = buffer_ + (abs_path.data() - full_buffer.data());
    abs_path_ = std::string_view(abs_path_at, abs_path.size());
    helpers::percent_decode_in_place(abs_path_);
    target_ = target_form;
    headers_ = std::move(headers);
    query_parameters_ = std::move(query_parameters);
    if (host) host_ = *host;
    if (port) host_port_ = *port;
    if (type) host_type_ = *type;
    if (target_authority_host) ta_host_ = *target_authority_host;
    if (target_authority_port) ta_port_ = *target_authority_port;
    if (target_authority_type) ta_type_ = *target_authority_type;
    wants_connection_close_ = wants_connection_close;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  char* buffer_ = nullptr;            // buffer holding the serialized request
  std::string_view method_;           // HTTP method (e.g., GET, POST, etc.)
  std::string_view abs_path_;         // absolute path from the request-target
  target target_ = target::kUnknown;  // request-target form
  std::string_view host_;             // host from the Host header
  std::string_view host_port_;        // port from the Host header
  helpers::host_type host_type_ = helpers::host_type::kUnknown;
  std::string_view ta_host_;  // target authority host
  std::string_view ta_port_;  // target authority port
  helpers::host_type ta_type_ = helpers::host_type::kUnknown;
  std::vector<header_view> headers_;                    // vector of headers
  std::vector<query_parameter_view> query_parameters_;  // query parameters
  std::shared_ptr<body::reader> body_reader_;           // body reader
  bool wants_connection_close_ = false;  // protocol decided to close channel
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
