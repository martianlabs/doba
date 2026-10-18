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

#include <cstddef>
#include <span>
#include <string_view>
#include <type_traits>
#include <utility>

#include "common/reader.h"
#include "common/signaler.h"
#include "http_server.h"
#include "protocol/http/v11/contracts.h"

namespace {
using namespace martianlabs::doba::protocol::http::v11;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] custom_response                                             ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class custom_response : public response {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  custom_response(const custom_response&) = delete;
  custom_response(custom_response&&) noexcept = default;
  ~custom_response() = default;
  // +=========================================================================+
  // | [>] from                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  custom_response(std::size_t head_limit, std::size_t body_threshold)
      : response(head_limit, body_threshold) {}
  static custom_response from(std::size_t head_limit,
                              std::size_t body_threshold) {
    return custom_response(head_limit, body_threshold);
  }
  custom_response(std::size_t head_limit, std::size_t body_threshold,
                  std::string& head, std::string& body)
      : response(head_limit, body_threshold, head, body) {}
  static custom_response from(std::size_t head_limit,
                              std::size_t body_threshold,
                              std::string& head, std::string& body) {
    return custom_response(head_limit, body_threshold, head, body);
  }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  custom_response& operator=(const custom_response&) = delete;
  custom_response& operator=(custom_response&&) noexcept = default;
  // +=========================================================================+
  // | [>] add_header                                               ( public ) |
  // +-------------------------------------------------------------------------+
  custom_response& add_header(std::string_view key, std::string_view value) {
    response::add_header(key, value);
    return *this;
  }
  template <typename T>
    requires std::is_arithmetic_v<T>
  custom_response& add_header(std::string_view key, const T& value) {
    response::add_header(key, value);
    return *this;
  }
  // +=========================================================================+
  // | [>] set_header                                               ( public ) |
  // +-------------------------------------------------------------------------+
  custom_response& set_header(std::string_view key, std::string_view value) {
    response::set_header(key, value);
    return *this;
  }
  template <typename T>
    requires std::is_arithmetic_v<T>
  custom_response& set_header(std::string_view key, const T& value) {
    response::set_header(key, value);
    return *this;
  }
  // +=========================================================================+
  // | [>] remove_header                                            ( public ) |
  // +-------------------------------------------------------------------------+
  custom_response& remove_header(std::string_view key) {
    response::remove_header(key);
    return *this;
  }
  // +=========================================================================+
  // | [>] set_body                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  custom_response& set_body(std::string_view value) {
    response::set_body(value);
    return *this;
  }
  custom_response& set_body(body::reader* value) {
    response::set_body(value);
    return *this;
  }
  custom_response& set_body(body::body_writer&& value) {
    response::set_body(std::move(value));
    return *this;
  }
  template <typename T>
    requires std::is_arithmetic_v<T>
  custom_response& set_body(const T& value) {
    response::set_body(value);
    return *this;
  }
  custom_response& set_body(martianlabs::doba::common::reader&& value,
                            std::size_t length) {
    response::set_body(std::move(value), length);
    return *this;
  }
  // +=========================================================================+
  // | [>] clear_body                                               ( public ) |
  // +-------------------------------------------------------------------------+
  custom_response& clear_body() {
    response::clear_body();
    return *this;
  }
  // +=========================================================================+
  // | [>] suppress_body                                            ( public ) |
  // +-------------------------------------------------------------------------+
  custom_response& suppress_body() {
    response::suppress_body();
    return *this;
  }
  // +=========================================================================+
  // | [>] STATUS-LINEs                                             ( public ) |
  // +-------------------------------------------------------------------------+
  custom_response& continue_100() {
    response::continue_100();
    return *this;
  }
  custom_response& switching_protocols_101() {
    response::switching_protocols_101();
    return *this;
  }
  custom_response& ok_200() {
    response::ok_200();
    return *this;
  }
  custom_response& created_201() {
    response::created_201();
    return *this;
  }
  custom_response& accepted_202() {
    response::accepted_202();
    return *this;
  }
  custom_response& non_authoritative_info_203() {
    response::non_authoritative_info_203();
    return *this;
  }
  custom_response& no_content_204() {
    response::no_content_204();
    return *this;
  }
  custom_response& reset_content_205() {
    response::reset_content_205();
    return *this;
  }
  custom_response& partial_content_206() {
    response::partial_content_206();
    return *this;
  }
  custom_response& multiple_choices_300() {
    response::multiple_choices_300();
    return *this;
  }
  custom_response& moved_permanently_301() {
    response::moved_permanently_301();
    return *this;
  }
  custom_response& found_302() {
    response::found_302();
    return *this;
  }
  custom_response& see_other_303() {
    response::see_other_303();
    return *this;
  }
  custom_response& not_modified_304() {
    response::not_modified_304();
    return *this;
  }
  custom_response& use_proxy_305() {
    response::use_proxy_305();
    return *this;
  }
  custom_response& unused_306() {
    response::unused_306();
    return *this;
  }
  custom_response& temporary_redirect_307() {
    response::temporary_redirect_307();
    return *this;
  }
  custom_response& permanent_redirect_308() {
    response::permanent_redirect_308();
    return *this;
  }
  custom_response& bad_request_400() {
    response::bad_request_400();
    return *this;
  }
  custom_response& unauthorized_401() {
    response::unauthorized_401();
    return *this;
  }
  custom_response& payment_required_402() {
    response::payment_required_402();
    return *this;
  }
  custom_response& forbidden_403() {
    response::forbidden_403();
    return *this;
  }
  custom_response& not_found_404() {
    response::not_found_404();
    return *this;
  }
  custom_response& method_not_allowed_405() {
    response::method_not_allowed_405();
    return *this;
  }
  custom_response& not_acceptable_406() {
    response::not_acceptable_406();
    return *this;
  }
  custom_response& proxy_auth_required_407() {
    response::proxy_auth_required_407();
    return *this;
  }
  custom_response& request_timeout_408() {
    response::request_timeout_408();
    return *this;
  }
  custom_response& conflict_409() {
    response::conflict_409();
    return *this;
  }
  custom_response& gone_410() {
    response::gone_410();
    return *this;
  }
  custom_response& length_required_411() {
    response::length_required_411();
    return *this;
  }
  custom_response& precondition_failed_412() {
    response::precondition_failed_412();
    return *this;
  }
  custom_response& content_too_large_413() {
    response::content_too_large_413();
    return *this;
  }
  custom_response& uri_too_long_414() {
    response::uri_too_long_414();
    return *this;
  }
  custom_response& unsupported_media_type_415() {
    response::unsupported_media_type_415();
    return *this;
  }
  custom_response& range_not_satisfiable_416() {
    response::range_not_satisfiable_416();
    return *this;
  }
  custom_response& expectation_failed_417() {
    response::expectation_failed_417();
    return *this;
  }
  custom_response& unused_418() {
    response::unused_418();
    return *this;
  }
  custom_response& misdirected_request_421() {
    response::misdirected_request_421();
    return *this;
  }
  custom_response& unprocessable_content_422() {
    response::unprocessable_content_422();
    return *this;
  }
  custom_response& upgrade_required_426() {
    response::upgrade_required_426();
    return *this;
  }
  custom_response& request_header_fields_too_large_431() {
    response::request_header_fields_too_large_431();
    return *this;
  }
  custom_response& internal_server_error_500() {
    response::internal_server_error_500();
    return *this;
  }
  custom_response& not_implemented_501() {
    response::not_implemented_501();
    return *this;
  }
  custom_response& bad_gateway_502() {
    response::bad_gateway_502();
    return *this;
  }
  custom_response& service_unavailable_503() {
    response::service_unavailable_503();
    return *this;
  }
  custom_response& gateway_timeout_504() {
    response::gateway_timeout_504();
    return *this;
  }
  custom_response& http_version_not_supported_505() {
    response::http_version_not_supported_505();
    return *this;
  }
  // +=========================================================================+
  // | [>] json_200                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  custom_response& json_200(std::string_view value) {
    ok_200();
    set_header("Content-Type", "application/json");
    set_body(value);
    return *this;
  }
};

static_assert(contracts::response<custom_response>);
}  // namespace

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main() {
  martianlabs::doba::http_server<request, custom_response> server(
      {.ip = "0.0.0.0", .port = "8080"});
  server.add_route("GET", "/hello",
                   [](const request&, custom_response& res) {
                     res.json_200(R"({"message":"hello"})");
                   });
  server.start();
  martianlabs::doba::common::signaler::wait();
  server.stop();
  return 0;
}
