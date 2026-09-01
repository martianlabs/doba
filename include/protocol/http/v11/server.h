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

#ifndef martianlabs_doba_protocol_http_v11_server_h
#define martianlabs_doba_protocol_http_v11_server_h

#include <memory>

#include "common/date_server.h"
#include "transport/server/tcpip.h"
#include "protocol/http/common/method_names.h"
#include "protocol/http/common/helpers.h"
#include "protocol/http/v11/request.h"
#include "protocol/http/v11/response.h"
#include "protocol/http/common/router.h"
#include "protocol/http/common/header_names.h"
#include "protocol/http/v11/decoder.h"
#include "protocol/http/v11/rejection_reason.h"
#include "protocol/http/v11/session.h"

namespace martianlabs::doba::protocol::http::v11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] server                                                      ( class ) |
// +---------------------------------------------------------------------------+
// | This class holds for the http 1.1 server implementation.                  |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   RQty - request being used (v11::request by default).                    |
// |   RSty - response being used (v11::response by default).                  |
// |   DEty - decoder being used (v11::decoder by default).                    |
// |   TRty - transport being used (tcp/ip by default).                        |
// |   ROty - router being used (v11::router by default).                      |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty = request, typename RSty = response,
          template <typename, typename> class DEty = decoder,
          template <typename, typename,
                    template <typename, typename> typename> class TRty =
              transport::server::tcpip,
          template <typename, typename> class ROty = router>
class server {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  server() = default;
  server(const server&) = delete;
  server(server&&) noexcept = delete;
  ~server() { stop(); }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  server& operator=(const server&) = delete;
  server& operator=(server&&) noexcept = delete;
  // +=========================================================================+
  // | [>] start                                                    ( public ) |
  // +=========================================================================+
  void start(const char port[]) {
    std::lock_guard<std::mutex> lock(locked_mutex_);
    common::date_server::get().start();
    transport_.set_on_request(
        [this](const std::shared_ptr<RQty>& req, RSty& res, auto&) {
          session_.dispatch(*req, res, router_);
        });
    transport_.set_on_bad_request(
        [](int code, std::string_view reason, RSty& res) {
          // The transport hands back the neutral reason recorded by the
          // decoder; only the HTTP layer knows how to translate it into a
          // status code (RFC 9110 semantics live here, not in the transport).
          switch (static_cast<rejection_reason>(code)) {
            case rejection_reason::kPayloadTooLarge:
              res.content_too_large_413().set_body(reason);
              break;
            case rejection_reason::kUnsupportedFeature:
              res.not_implemented_501().set_body(reason);
              break;
            case rejection_reason::kVersionNotSupported:
              res.http_version_not_supported_505().set_body(reason);
              break;
            case rejection_reason::kUriTooLong:
              res.uri_too_long_414().set_body(reason);
              break;
            case rejection_reason::kHeaderFieldsTooLarge:
              res.request_header_fields_too_large_431().set_body(reason);
              break;
            case rejection_reason::kHandlerError:
              res.internal_server_error_500().set_body(reason);
              break;
            case rejection_reason::kExpectationFailed:
              res.expectation_failed_417().set_body(reason);
              break;
            case rejection_reason::kSyntax:
            case rejection_reason::kNone:
            default:
              res.bad_request_400().set_body(reason);
              break;
          }
        });
    transport_.set_on_connection([this]() { connections_++; });
    transport_.set_on_disconnection([this]() { connections_--; });
    transport_.start(port);
    locked_ = true;
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +=========================================================================+
  void stop() {
    std::lock_guard<std::mutex> lock(locked_mutex_);
    transport_.stop();
    common::date_server::get().stop();
    locked_ = false;
  }
  // +=========================================================================+
  // | [>] add_route                                                ( public ) |
  // +=========================================================================+
  template <router_handler_lambda Hty>
  server& add_route(std::string_view method, std::string_view route,
                    Hty handler) {
    std::lock_guard<std::mutex> lock(locked_mutex_);
    if (locked_) {
      // If the server is running, we cannot add a route because the router is
      // locked and cannot be modified.
      throw std::runtime_error("Cannot add route when the server is running");
    }
    router_.add(method, route, std::move(handler));
    return *this;
  }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  std::atomic<uint32_t> connections_{0};
  TRty<RQty, RSty, DEty> transport_;
  std::mutex locked_mutex_;
  ROty<RQty, RSty> router_;
  session<RQty, RSty, ROty<RQty, RSty>> session_;
  bool locked_{false};
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
