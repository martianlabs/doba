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

#include "common/date_server.h"
#include "common/execution_policy.h"
#include "transport/server/tcpip.h"
#include "protocol/http/common/method_names.h"
#include "protocol/http/common/helpers.h"
#include "protocol/http/v11/request.h"
#include "protocol/http/v11/response.h"
#include "protocol/http/common/router.h"
#include "protocol/http/common/header_names.h"
#include "protocol/http/v11/decoder.h"
#include "protocol/http/v11/rejection_reason.h"

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
    router_.start();
    transport_.set_on_request(
        [this](std::shared_ptr<const RQty> req, std::shared_ptr<RSty> res,
               transport::server::types::on_send_delegate<RSty> on_send) {
          auto send = [req, on_send](std::shared_ptr<RSty> res) {
            if (req->wants_connection_close()) {
              res->set_header(header_names::kConnection, "close");
            }
            if (req->get_method() == method_names::kHead) {
              // RFC 9110 S9.3.2: a HEAD response must describe the same
              // headers a matching GET would have produced, but must never
              // carry a message body. clear_body() also drops the framing
              // headers, so they are captured beforehand and restored right
              // after, using only the response's already public API.
              bool had_cl = res->has_header(header_names::kContentLength);
              std::string cl =
                  had_cl ? res->get_header(header_names::kContentLength).second
                         : std::string();
              bool had_te = res->has_header(header_names::kTransferEncoding);
              std::string te =
                  had_te
                      ? res->get_header(header_names::kTransferEncoding).second
                      : std::string();
              res->clear_body();
              if (had_cl) res->set_header(header_names::kContentLength, cl);
              if (had_te) res->set_header(header_names::kTransferEncoding, te);
            }
            on_send(res);
          };
          switch (req->get_target()) {
            case target::kOriginForm:
            case target::kAbsoluteForm: {
              // The request is either in origin-form (RFC 9110 S9.3.5) or
              // absolute-form (RFC 9110 S9.3.4); in either case, the request is
              // routed to a handler based on the method and absolute path.
              std::string_view method = req->get_method();
              std::string_view abs_path = req->get_absolute_path();
              switch (router_.match(method, abs_path, req, res, send)) {
                case router_match_result::kMatched:
                  break;
                case router_match_result::kNotFound:
                  res->not_found_404();
                  send(res);
                  break;
                case router_match_result::kMethodNotAllowed:
                  res->method_not_allowed_405();
                  send(res);
                  break;
              }
              break;
            }
            case target::kAuthorityForm:
              // CONNECT tunnelling (RFC 9110 S9.3.6) is deliberately deferred
              // to Doba's future client (dial-out) module: it will open the
              // outbound connection to the target authority already owned by
              // the request (req->get_target_authority_host()/_port()) and
              // drive the raw-byte relay, keeping this server-side transport
              // agnostic of CONNECT. Until that module exists, the request
              // must not be left unanswered.
              res->not_implemented_501();
              send(res);
              return;
            case target::kAsteriskForm:
              // OPTIONS * (RFC 9110 S9.3.7) addresses the server in general
              // rather than a specific resource; acknowledge it without
              // routing to a handler.
              res->ok_200();
              send(res);
              return;
            default:
              res->bad_request_400();
              send(res);
              return;
          }
        });
    transport_.set_on_bad_request(
        [](int code, std::string_view reason, std::shared_ptr<RSty> res) {
          // The transport hands back the neutral reason recorded by the
          // decoder; only the HTTP layer knows how to translate it into a
          // status code (RFC 9110 semantics live here, not in the transport).
          switch (static_cast<rejection_reason>(code)) {
            case rejection_reason::kPayloadTooLarge:
              res->content_too_large_413().set_body(reason);
              break;
            case rejection_reason::kUnsupportedFeature:
              res->not_implemented_501().set_body(reason);
              break;
            case rejection_reason::kVersionNotSupported:
              res->http_version_not_supported_505().set_body(reason);
              break;
            case rejection_reason::kUriTooLong:
              res->uri_too_long_414().set_body(reason);
              break;
            case rejection_reason::kHeaderFieldsTooLarge:
              res->request_header_fields_too_large_431().set_body(reason);
              break;
            case rejection_reason::kHandlerError:
              res->internal_server_error_500().set_body(reason);
              break;
            case rejection_reason::kExpectationFailed:
              res->expectation_failed_417().set_body(reason);
              break;
            case rejection_reason::kSyntax:
            case rejection_reason::kNone:
            default:
              res->bad_request_400().set_body(reason);
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
    router_.stop();
    transport_.stop();
    common::date_server::get().stop();
    locked_ = false;
  }
  // +=========================================================================+
  // | [>] add_route                                                ( public ) |
  // +=========================================================================+
  template <router_handler_lambda Hty>
  server& add_route(std::string_view method, std::string_view route,
                    Hty handler,
                    common::execution_policy policy =
                        common::execution_policy::kSynchronous) {
    std::lock_guard<std::mutex> lock(locked_mutex_);
    if (locked_) {
      // If the server is running, we cannot add a route because the router is
      // locked and cannot be modified.
      throw std::runtime_error("Cannot add route when the server is running");
    }
    router_.add(method, route, std::move(handler), policy);
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
  bool locked_{false};
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
