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

#ifndef martianlabs_doba_protocol_http_v11_engine_sync_h
#define martianlabs_doba_protocol_http_v11_engine_sync_h

#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "protocol/contracts.h"
#include "protocol/http/v11/decoder.h"
#include "protocol/http/v11/results.h"
#include "protocol/http/common/header_names.h"
#include "protocol/http/common/method_names.h"
#include "protocol/http/common/headers/www_authenticate.h"
#include "protocol/http/common/router.h"
#include "protocol/http/common/router_match_result.h"

namespace martianlabs::doba::protocol::http::v11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] engine_sync                                                ( struct ) |
// +---------------------------------------------------------------------------+
// | The synchronous engine handles the processing of a single request and     |
// | response in a blocking manner. It is designed for environments where      |
// | asynchronous processing is not required or desired. It provides methods   |
// | for executing the request against the router, building error responses,   |
// | and serializing the response.                                             |
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty, typename ROty>
struct engine_sync {
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  engine_sync() = default;
  engine_sync(const engine_sync&) = delete;
  engine_sync(engine_sync&&) noexcept = delete;
  ~engine_sync() = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  engine_sync& operator=(const engine_sync&) = delete;
  engine_sync& operator=(engine_sync&&) noexcept = delete;
  // +=========================================================================+
  // | [>] handle_request                                           ( public ) |
  // +-------------------------------------------------------------------------+
  // | Handles an incoming request synchronously. It executes the request      |
  // | against the router, builds the response, serializes it, and submits     |
  // | it to the transport. It returns true if the connection should be        |
  // | closed after the response is sent.                                      |
  // +-------------------------------------------------------------------------+
  static bool handle_request(decoder<RQty, RSty>& owner, const ROty& router,
                             const RQty& request,
                             const router_match_result<RQty, RSty>& match,
                             const protocol::send_delegate& on_send) {
    bool close = request.wants_connection_close();
    RSty response = owner.make_response();
    execute(router, request, response, close, &match);
    auto serialized = serialize(request, response, close);
    on_send(serialized.head, serialized.body, std::move(serialized.source));
    return close;
  }
  // +=========================================================================+
  // | [>] handle_response                                          ( public ) |
  // +-------------------------------------------------------------------------+
  // | Handles a response synchronously. It serializes the response and        |
  // | submits it to the transport. It returns true if the connection should   |
  // | be closed after the response is sent.                                   |
  // +-------------------------------------------------------------------------+
  static void handle_response(RSty& response, bool close,
                              const protocol::send_delegate& on_send) {
    auto serialized = serialize(response, close);
    on_send(serialized.head, serialized.body, std::move(serialized.source));
  }
  // +=========================================================================+
  // | [>] execute                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  // | Executes the request against the router and builds the response. It     |
  // | handles different request targets and sets the appropriate response.    |
  // | If an exception occurs during execution, it builds an error response.   |
  // +-------------------------------------------------------------------------+
  static void execute(
      const ROty& router, const RQty& request, RSty& response, bool& close,
      const router_match_result<RQty, RSty>* selected = nullptr) {
    try {
      switch (request.get_target()) {
        case target::kOriginForm:
        case target::kAbsoluteForm: {
          const std::string_view path = request.get_absolute_path();
          const router_match_result<RQty, RSty> match =
              selected ? *selected : router.match(request.get_method(), path);
          if (match && !authenticate(match, request, response)) return;
          if (match.handler) {
            (*match.handler)(request, response);
            return;
          }
          if (match.parametrized_handler) {
            match.parametrized_handler->invoke(request, response, path);
            return;
          }
          const std::string allowed = router.allowed_methods(path);
          if (allowed.empty()) {
            response.not_found_404();
            return;
          }
          // RFC 9110 S15.5.6: a 405 response must advertise allowed methods.
          response.method_not_allowed_405();
          response.set_header(header_names::kAllow, allowed);
          return;
        }
        case target::kAuthorityForm:
          response.not_implemented_501();
          return;
        case target::kAsteriskForm:
          response.ok_200();
          return;
        default:
          response.bad_request_400();
          return;
      }
    } catch (...) {
      close = true;
      build_error_response(response);
    }
  }
  // +=========================================================================+
  // | [>] authenticate                                             ( public ) |
  // +-------------------------------------------------------------------------+
  static bool authenticate(const router_match_result<RQty, RSty>& match,
                           const RQty& request, RSty& response) {
    if (!match.authenticate || !*match.authenticate) return true;
    const auto result = (*match.authenticate)(request);
    if (result.status == auth::authentication_status::kAccepted) return true;
    if ((!result.challenge.empty() &&
         !http::headers::www_authenticate::check(result.challenge)) ||
        (result.status == auth::authentication_status::kUnauthenticated &&
         result.challenge.find_first_not_of(" \t,") ==
             std::string::npos)) {
      throw std::invalid_argument("Invalid authentication challenge");
    }
    switch (result.status) {
      case auth::authentication_status::kInvalidRequest:
        response.bad_request_400();
        break;
      case auth::authentication_status::kUnauthenticated:
        response.unauthorized_401();
        break;
      case auth::authentication_status::kForbidden:
        response.forbidden_403();
        break;
      default:
        throw std::invalid_argument("Invalid authentication result");
    }
    if (!result.challenge.empty()) {
      response.set_header(header_names::kWwwAuthenticate, result.challenge);
    }
    return false;
  }
  // +=========================================================================+
  // | [>] build_error_response                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // | Builds a generic error response with a 500 Internal Server Error        |
  // | status. It sets the response body to "Internal Server Error".           |
  // | This is used when an exception occurs during request processing.        |
  // +-------------------------------------------------------------------------+
  static void build_error_response(RSty& response) {
    response.internal_server_error_500();
    response.set_body("Internal Server Error");
  }
  // +=========================================================================+
  // | [>] serialize                                                ( public ) |
  // +-------------------------------------------------------------------------+
  // | Serializes the response. If the connection should be closed,            |
  // | it sets the "Connection: close" header. It returns the                  |
  // | serialized result.                                                      |
  // +-------------------------------------------------------------------------+
  static serialization_result serialize(RSty& response, bool close) {
    if (close) response.set_header(header_names::kConnection, "close");
    return response.serialize();
  }
  // +=========================================================================+
  // | [>] serialize                                                ( public ) |
  // +-------------------------------------------------------------------------+
  // | Serializes the response based on the request. If the connection should  |
  // | be closed, it sets the "Connection: close" header. It returns the       |
  // | serialized result.                                                      |
  // +-------------------------------------------------------------------------+
  static serialization_result serialize(const RQty& request, RSty& response,
                                        bool& close) {
    serialization_result serialized;
    try {
      if (response.is_continue_100()) {
        close = true;
        build_error_response(response);
      }
      if (close) {
        response.set_header(header_names::kConnection, "close");
      } else if (response.wants_connection_close()) {
        close = true;
      }
      // RFC 9110 S9.3.2: HEAD preserves framing but never sends body bytes.
      if (request.get_method() == method_names::kHead) response.suppress_body();
      serialized = response.serialize();
    } catch (...) {
      close = true;
      build_error_response(response);
      response.set_header(header_names::kConnection, "close");
      if (request.get_method() == method_names::kHead) response.suppress_body();
      serialized = response.serialize();
    }
    return serialized;
  }
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
