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

#include <string>
#include <string_view>
#include <utility>

#include "protocol/serialization.h"
#include "protocol/http/common/header_names.h"
#include "protocol/http/common/method_names.h"
#include "protocol/http/common/router.h"

namespace martianlabs::doba::protocol::http::v11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] engine_sync                                                ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty, typename ROty>
struct engine_sync {
  // +=========================================================================+
  // | [>] execute                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  static void execute(const ROty& router, const RQty& request, RSty& response,
                      bool& close,
                      const typename ROty::route_match* selected = nullptr) {
    try {
      switch (request.get_target()) {
        case target::kOriginForm:
        case target::kAbsoluteForm: {
          const std::string_view path = request.get_absolute_path();
          const typename ROty::route_match match =
              selected ? *selected : router.match(request.get_method(), path);
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
  // | [>] build_error_response                                     ( public ) |
  // +-------------------------------------------------------------------------+
  static void build_error_response(RSty& response) {
    response.internal_server_error_500();
    response.set_body("Internal Server Error");
  }
  // +=========================================================================+
  // | [>] serialize                                                ( public ) |
  // +-------------------------------------------------------------------------+
  static protocol::serialization_result serialize(RSty& response, bool close) {
    if (close) response.set_header(header_names::kConnection, "close");
    return response.serialize();
  }
  // +=========================================================================+
  // | [>] serialize                                                ( public ) |
  // +-------------------------------------------------------------------------+
  static protocol::serialization_result serialize(const RQty& request,
                                                  RSty& response, bool& close) {
    protocol::serialization_result serialized;
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
