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

#ifndef martianlabs_doba_protocol_http11_request_h
#define martianlabs_doba_protocol_http11_request_h

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
#include "protocol/http11/context.h"
#include "protocol/http11/request_getter.h"
#include "protocol/http11/query_parameter.h"
#include "protocol/http11/method_names.h"
#include "protocol/http11/header_names.h"
#include "protocol/http11/header.h"
#include "protocol/http11/helpers.h"
#include "protocol/http11/parsed_types.h"
#include "protocol/http11/policies.h"
#include "protocol/http11/verdict.h"
#include "protocol/http11/target.h"
#include "protocol/http11/body/writer_chunked.h"
#include "protocol/http11/body/writer_raw.h"
#include "protocol/http11/headers/accept.h"
#include "protocol/http11/headers/accept_charset.h"
#include "protocol/http11/headers/accept_encoding.h"
#include "protocol/http11/headers/accept_language.h"
#include "protocol/http11/headers/accept_ranges.h"
#include "protocol/http11/headers/access_control_allow_credentials.h"
#include "protocol/http11/headers/access_control_allow_headers.h"
#include "protocol/http11/headers/access_control_allow_methods.h"
#include "protocol/http11/headers/access_control_allow_origin.h"
#include "protocol/http11/headers/access_control_expose_headers.h"
#include "protocol/http11/headers/access_control_max_age.h"
#include "protocol/http11/headers/access_control_request_headers.h"
#include "protocol/http11/headers/access_control_request_method.h"
#include "protocol/http11/headers/age.h"
#include "protocol/http11/headers/allow.h"
#include "protocol/http11/headers/authentication_info.h"
#include "protocol/http11/headers/authorization.h"
#include "protocol/http11/headers/cache_control.h"
#include "protocol/http11/headers/connection.h"
#include "protocol/http11/headers/content_encoding.h"
#include "protocol/http11/headers/content_language.h"
#include "protocol/http11/headers/content_length.h"
#include "protocol/http11/headers/content_location.h"
#include "protocol/http11/headers/content_range.h"
#include "protocol/http11/headers/content_type.h"
#include "protocol/http11/headers/cookie.h"
#include "protocol/http11/headers/date.h"
#include "protocol/http11/headers/etag.h"
#include "protocol/http11/headers/expect.h"
#include "protocol/http11/headers/expires.h"
#include "protocol/http11/headers/forwarded.h"
#include "protocol/http11/headers/from.h"
#include "protocol/http11/headers/host.h"
#include "protocol/http11/headers/if_match.h"
#include "protocol/http11/headers/if_modified_since.h"
#include "protocol/http11/headers/if_none_match.h"
#include "protocol/http11/headers/if_range.h"
#include "protocol/http11/headers/if_unmodified_since.h"
#include "protocol/http11/headers/keep_alive.h"
#include "protocol/http11/headers/last_modified.h"
#include "protocol/http11/headers/location.h"
#include "protocol/http11/headers/max_forwards.h"
#include "protocol/http11/headers/origin.h"
#include "protocol/http11/headers/pragma.h"
#include "protocol/http11/headers/proxy_connection.h"
#include "protocol/http11/headers/range.h"
#include "protocol/http11/headers/referer.h"
#include "protocol/http11/headers/retry_after.h"
#include "protocol/http11/headers/sec_websocket_accept.h"
#include "protocol/http11/headers/sec_websocket_extensions.h"
#include "protocol/http11/headers/sec_websocket_key.h"
#include "protocol/http11/headers/sec_websocket_protocol.h"
#include "protocol/http11/headers/sec_websocket_version.h"
#include "protocol/http11/headers/server.h"
#include "protocol/http11/headers/set_cookie.h"
#include "protocol/http11/headers/te.h"
#include "protocol/http11/headers/trailer.h"
#include "protocol/http11/headers/transfer_encoding.h"
#include "protocol/http11/headers/upgrade.h"
#include "protocol/http11/headers/user_agent.h"
#include "protocol/http11/headers/vary.h"
#include "protocol/http11/headers/via.h"
#include "protocol/http11/headers/www_authenticate.h"
#include "protocol/http11/headers/x_forwarded_for.h"
#include "protocol/http11/headers/x_forwarded_host.h"
#include "protocol/http11/headers/x_forwarded_proto.h"
#include "protocol/http11/headers/rules/directives.h"
#include "protocol/http11/headers/rules/framing.h"
#include "protocol/http11/headers/rules/policy.h"
#include "protocol/http11/headers/rules/routing.h"

namespace martianlabs::doba::protocol::http11 {
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
  ~request() = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  request& operator=(const request&) = delete;
  request& operator=(request&&) noexcept = delete;
  // +=========================================================================+
  // | [>] CONSTANTs                                                ( public ) |
  // +=========================================================================+
  static constexpr std::size_t kMaxHeadSize = 4096;
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
      std::optional<helpers::host_type> target_authority_type) {
    std::shared_ptr<request> req = std::shared_ptr<request>(new request());
    std::memcpy(req->buffer_, full_buffer.data(), full_buffer.size());
    char* method_at = req->buffer_ + (method.data() - full_buffer.data());
    req->method_ = std::string_view(method_at, method.size());
    char* abs_path_at = req->buffer_ + (abs_path.data() - full_buffer.data());
    req->abs_path_ = std::string_view(abs_path_at, abs_path.size());
    req->target_ = target_form;
    req->headers_ = std::move(headers);
    req->query_parameters_ = std::move(query_parameters);
    if (host) req->host_ = *host;
    if (port) req->host_port_ = *port;
    if (type) req->host_type_ = *type;
    if (target_authority_host) req->ta_host_ = *target_authority_host;
    if (target_authority_port) req->ta_port_ = *target_authority_port;
    if (target_authority_type) req->ta_type_ = *target_authority_type;
    return [req](std::optional<common::byte_storage> byte_storage)
               -> std::shared_ptr<request> {
      if (byte_storage) {
        req->body_reader_ = std::shared_ptr<common::reader>(
            new common::reader(std::move(*byte_storage)));
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
  auto get_headers_length() const { return headers_.size(); }
  auto get_query_parameter(std::size_t i) const { return query_parameters_[i]; }
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

 private:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                ( private ) |
  // +=========================================================================+
  request() = default;
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  char buffer_[kMaxHeadSize];         // buffer holding the serialized request
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
  std::shared_ptr<common::reader> body_reader_;         // body reader
};
}  // namespace martianlabs::doba::protocol::http11

#endif
