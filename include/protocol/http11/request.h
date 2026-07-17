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
#include "protocol/http11/method_names.h"
#include "protocol/http11/header_names.h"
#include "protocol/http11/header.h"
#include "protocol/http11/query_parameter.h"
#include "protocol/http11/helpers.h"
#include "protocol/http11/parsed_types.h"
#include "protocol/http11/policies.h"
#include "protocol/http11/verdict.h"
#include "protocol/http11/target.h"

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
  request() = default;
  request(const request&) = delete;
  request(request&&) noexcept = delete;
  ~request() = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  request& operator=(const request&) = delete;
  request& operator=(request&&) noexcept = delete;
  // +=========================================================================+
  // | [>] SETTERs                                                  ( public ) |
  // +=========================================================================+
  void set_method(std::string_view method) { method_ = method; }
  void set_target(target target) { target_ = target; }
  void set_absolute_path(std::string_view abs_path) { abs_path_ = abs_path; }
  void set_headers(std::vector<header_view> headers) {
    for (auto const& header : headers) {
      headers_.emplace_back(header.first, header.second);
    }
  }
  void set_query_parameters(std::vector<query_parameter_view> parameters) {
    for (auto const& parameter : parameters) {
      query_parameters_.emplace_back(parameter.first, parameter.second);
    }
  }
  void set_host(std::string_view host) { host_ = host; }
  void set_host_port(std::string_view port) { host_port_ = port; }
  void set_host_type(helpers::host_type type) { host_type_ = type; }
  void set_target_authority(std::string_view host) { ta_host_ = host; }
  void set_target_authority_port(std::string_view port) { ta_port_ = port; }
  void set_target_authority_type(helpers::host_type type) { ta_type_ = type; }
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

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  std::string method_;  // HTTP method (e.g., GET, POST, etc.)
  std::string abs_path_;  // absolute path from the request-target
  target target_ = target::kUnknown;
  std::string host_;  // host from the Host header
  std::string host_port_;  // port from the Host header
  helpers::host_type host_type_ = helpers::host_type::kUnknown;
  std::string ta_host_;  // target authority host
  std::string ta_port_; // target authority port
  helpers::host_type ta_type_ = helpers::host_type::kUnknown;
  std::vector<header> headers_;  // vector of headers
  std::vector<query_parameter> query_parameters_;  // query parameters
};
}  // namespace martianlabs::doba::protocol::http11

#endif
