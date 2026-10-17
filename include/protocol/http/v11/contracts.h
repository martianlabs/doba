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

#ifndef martianlabs_doba_protocol_http_v11_contracts_h
#define martianlabs_doba_protocol_http_v11_contracts_h

#include <concepts>
#include <cstddef>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "common/reader.h"
#include "common/task.h"
#include "protocol/http/common/header.h"
#include "protocol/http/common/helpers.h"
#include "protocol/http/common/query_parameter.h"
#include "protocol/http/common/router_match_result.h"
#include "protocol/http/common/target.h"
#include "protocol/http/v11/body/reader.h"
#include "protocol/http/v11/body/writer.h"
#include "protocol/contracts.h"
#include "protocol/http/v11/results.h"

namespace martianlabs::doba::protocol::http::v11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] make_engine_factory                                      ( function ) |
// +---------------------------------------------------------------------------+
// | Creates a factory function for engines.                                   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty, typename ROty>
auto make_engine_factory(typename ENty::policies_type configuration,
                         const ROty& routes) {
  return [configuration = std::move(configuration), &routes]() -> ENty {
    return ENty{configuration, routes};
  };
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] make_engine_factory                                      ( function ) |
// +---------------------------------------------------------------------------+
// | Creates a factory function for engines providing asynchronous callbacks.  |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty, typename ROty>
auto make_engine_factory(typename ENty::policies_type configuration,
                         const ROty& routes,
                         std::function<void()> on_async_started,
                         std::function<void()> on_async_finished) {
  return [configuration = std::move(configuration), &routes,
          on_async_started = std::move(on_async_started),
          on_async_finished = std::move(on_async_finished)]() -> ENty {
    return ENty{configuration, routes, on_async_started, on_async_finished};
  };
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] engine_factory                                               ( type ) |
// +---------------------------------------------------------------------------+
// | Type of the engine factory function.                                      |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty, typename ROty>
using engine_factory = decltype(make_engine_factory<ENty>(
    std::declval<typename ENty::policies_type>(), std::declval<const ROty&>()));

namespace contracts {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] request                                                     (concept) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty>
concept request =
    std::move_constructible<RQty> && std::assignable_from<RQty&, RQty> &&
    requires(const RQty& value, std::string_view text, std::size_t index,
             std::vector<http::header_view> headers,
             std::vector<http::query_parameter_view> query_parameters,
             std::optional<body::reader> body_reader) {
      {
        RQty::from(text, text, http::target::kOriginForm, std::move(headers),
                   std::move(query_parameters),
                   std::optional<std::string_view>{},
                   std::optional<std::string_view>{},
                   std::optional<http::helpers::host_type>{},
                   std::optional<std::string_view>{},
                   std::optional<std::string_view>{},
                   std::optional<http::helpers::host_type>{}, false,
                   std::move(body_reader))
      } -> std::same_as<RQty>;
      { value.get_method() } -> std::same_as<std::string_view>;
      { value.get_target() } -> std::same_as<http::target>;
      { value.get_absolute_path() } -> std::same_as<std::string_view>;
      { value.get_header(index) } -> std::same_as<http::header_view>;
      { value.get_header(text) } -> std::same_as<http::header_view>;
      { value.exist_header(text) } -> std::same_as<bool>;
      { value.get_headers_length() } -> std::same_as<std::size_t>;
      {
        value.get_query_parameter(index)
      } -> std::same_as<http::query_parameter_view>;
      {
        value.get_query_parameter(text)
      } -> std::same_as<std::optional<http::query_parameter_view>>;
      { value.get_query_parameters_length() } -> std::same_as<std::size_t>;
      { value.has_host() } -> std::same_as<bool>;
      { value.get_host() } -> std::same_as<std::string_view>;
      { value.get_host_port() } -> std::same_as<std::string_view>;
      { value.get_host_type() } -> std::same_as<http::helpers::host_type>;
      { value.has_target_authority() } -> std::same_as<bool>;
      { value.get_target_authority_host() } -> std::same_as<std::string_view>;
      { value.get_target_authority_port() } -> std::same_as<std::string_view>;
      {
        value.get_target_authority_type()
      } -> std::same_as<http::helpers::host_type>;
      { value.get_body_reader() } -> std::same_as<body::reader*>;
      { value.has_body_reader() } -> std::same_as<bool>;
      { value.wants_connection_close() } -> std::same_as<bool>;
      {
        value.get_cookie(text)
      } -> std::same_as<std::optional<std::string_view>>;
      {
        value.get_cookies()
      } -> std::same_as<
          std::vector<std::pair<std::string_view, std::string_view>>>;
    };

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] response                                                    (concept) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RSty>
concept response =
    std::move_constructible<RSty> && std::assignable_from<RSty&, RSty> &&
    std::same_as<typename RSty::serialized_type, serialization_result> &&
    requires(RSty& value, const RSty& constant, std::span<char> storage,
             std::vector<char>& head, std::vector<char>& body,
             std::size_t size, std::string_view text, body::reader* body_reader,
             body::body_writer body_writer, common::reader source) {
      { RSty::from(storage, size, size) } -> std::same_as<RSty>;
      { RSty::from(head, body, size, size) } -> std::same_as<RSty>;
      { value.serialize() } -> std::same_as<serialization_result>;
      { value.add_header(text, text) } -> std::same_as<RSty&>;
      { value.add_header(text, 1) } -> std::same_as<RSty&>;
      { value.add_header(text, 1.0) } -> std::same_as<RSty&>;
      { value.set_header(text, text) } -> std::same_as<RSty&>;
      { value.set_header(text, 1) } -> std::same_as<RSty&>;
      { value.set_header(text, 1.0) } -> std::same_as<RSty&>;
      { constant.has_header(text) } -> std::same_as<bool>;
      { constant.wants_connection_close() } -> std::same_as<bool>;
      { constant.is_continue_100() } -> std::same_as<bool>;
      {
        constant.get_header(text)
      } -> std::same_as<std::pair<std::string, std::string>>;
      {
        constant.get_header(size)
      } -> std::same_as<std::pair<std::string, std::string>>;
      { constant.get_headers_length() } -> std::same_as<std::size_t>;
      { value.remove_header(text) } -> std::same_as<RSty&>;
      { value.set_body(text) } -> std::same_as<RSty&>;
      { value.set_body(body_reader) } -> std::same_as<RSty&>;
      { value.set_body(std::move(body_writer)) } -> std::same_as<RSty&>;
      { value.set_body(1) } -> std::same_as<RSty&>;
      { value.set_body(1.0) } -> std::same_as<RSty&>;
      { value.set_body(std::move(source), size) } -> std::same_as<RSty&>;
      { value.clear_body() } -> std::same_as<RSty&>;
      { value.suppress_body() } -> std::same_as<RSty&>;
      { value.continue_100() } -> std::same_as<RSty&>;
      { value.switching_protocols_101() } -> std::same_as<RSty&>;
      { value.ok_200() } -> std::same_as<RSty&>;
      { value.created_201() } -> std::same_as<RSty&>;
      { value.accepted_202() } -> std::same_as<RSty&>;
      { value.non_authoritative_info_203() } -> std::same_as<RSty&>;
      { value.no_content_204() } -> std::same_as<RSty&>;
      { value.reset_content_205() } -> std::same_as<RSty&>;
      { value.partial_content_206() } -> std::same_as<RSty&>;
      { value.multiple_choices_300() } -> std::same_as<RSty&>;
      { value.moved_permanently_301() } -> std::same_as<RSty&>;
      { value.found_302() } -> std::same_as<RSty&>;
      { value.see_other_303() } -> std::same_as<RSty&>;
      { value.not_modified_304() } -> std::same_as<RSty&>;
      { value.use_proxy_305() } -> std::same_as<RSty&>;
      { value.unused_306() } -> std::same_as<RSty&>;
      { value.temporary_redirect_307() } -> std::same_as<RSty&>;
      { value.permanent_redirect_308() } -> std::same_as<RSty&>;
      { value.bad_request_400() } -> std::same_as<RSty&>;
      { value.unauthorized_401() } -> std::same_as<RSty&>;
      { value.payment_required_402() } -> std::same_as<RSty&>;
      { value.forbidden_403() } -> std::same_as<RSty&>;
      { value.not_found_404() } -> std::same_as<RSty&>;
      { value.method_not_allowed_405() } -> std::same_as<RSty&>;
      { value.not_acceptable_406() } -> std::same_as<RSty&>;
      { value.proxy_auth_required_407() } -> std::same_as<RSty&>;
      { value.request_timeout_408() } -> std::same_as<RSty&>;
      { value.conflict_409() } -> std::same_as<RSty&>;
      { value.gone_410() } -> std::same_as<RSty&>;
      { value.length_required_411() } -> std::same_as<RSty&>;
      { value.precondition_failed_412() } -> std::same_as<RSty&>;
      { value.content_too_large_413() } -> std::same_as<RSty&>;
      { value.uri_too_long_414() } -> std::same_as<RSty&>;
      { value.unsupported_media_type_415() } -> std::same_as<RSty&>;
      { value.range_not_satisfiable_416() } -> std::same_as<RSty&>;
      { value.expectation_failed_417() } -> std::same_as<RSty&>;
      { value.unused_418() } -> std::same_as<RSty&>;
      { value.misdirected_request_421() } -> std::same_as<RSty&>;
      { value.unprocessable_content_422() } -> std::same_as<RSty&>;
      { value.upgrade_required_426() } -> std::same_as<RSty&>;
      { value.request_header_fields_too_large_431() } -> std::same_as<RSty&>;
      { value.internal_server_error_500() } -> std::same_as<RSty&>;
      { value.not_implemented_501() } -> std::same_as<RSty&>;
      { value.bad_gateway_502() } -> std::same_as<RSty&>;
      { value.service_unavailable_503() } -> std::same_as<RSty&>;
      { value.gateway_timeout_504() } -> std::same_as<RSty&>;
      { value.http_version_not_supported_505() } -> std::same_as<RSty&>;
    };

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router                                                      (concept) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename ROty, typename RQty, typename RSty>
concept router =
    std::default_initializable<ROty> &&
    requires(ROty& routes, const ROty& constant, std::string_view text) {
      {
        routes.add(text, text, [](const RQty&, RSty&) {})
      } -> std::same_as<void>;
      {
        routes.add(text, text,
                   [](const RQty&, RSty&) -> common::task<void> { co_return; })
      } -> std::same_as<void>;
      { routes.template add_controller<RQty>() } -> std::same_as<void>;
      {
        constant.match(text, text)
      } -> std::same_as<http::router_match_result<RQty, RSty>>;
      { constant.allowed_methods(text) } -> std::same_as<std::string>;
    };

}  // namespace contracts
}  // namespace martianlabs::doba::protocol::http::v11

#endif
