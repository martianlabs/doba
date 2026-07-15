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

#ifndef martianlabs_doba_protocol_http11_server_h
#define martianlabs_doba_protocol_http11_server_h

#include "common/execution_policy.h"
#include "common/thread_pool.h"
#include "common/date_server.h"
#include "transport/server/tcpip.h"
#include "protocol/http11/method_names.h"
#include "protocol/http11/helpers.h"
#include "protocol/http11/request.h"
#include "protocol/http11/response.h"
#include "protocol/http11/router.h"
#include "protocol/http11/header_names.h"
#include "protocol/http11/decoder.h"

namespace martianlabs::doba::protocol::http11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] server                                                      ( class ) |
// +---------------------------------------------------------------------------+
// | This class holds for the http 1.1 server implementation.                  |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   RQty - request being used (http11::request by default).                 |
// |   RSty - response being used (http11::response by default).               |
// |   DEty - decoder being used (http11::decoder by default).                 |
// |   TRty - transport being used (tcp/ip by default).                        |
// |   ROty - router being used (http11::router by default).                   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty = request, typename RSty = response,
          template <typename, typename> class DEty = decoder,
          template <typename, typename,
                    template <typename, typename> typename> class TRty =
              transport::server::tcpip,
          typename FNty = std::function<void(const RQty&, RSty&)>,
          template <typename> class ROty = router>
class server {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  server() {
    transport_.on_request =
        [this](std::shared_ptr<const RQty> req, std::shared_ptr<RSty> res,
               transport::server::types::on_send_delegate<RSty> on_send) {
          switch (req->get_target()) {
            case target::kOriginForm:
            case target::kAbsoluteForm: {
              // The request is either in origin-form (RFC 9110 §9.3.5) or
              // absolute-form (RFC 9110 §9.3.4); in either case, the request is
              // routed to a handler based on the method and absolute path.
              std::optional<typename ROty<FNty>::data> router_entry =
                  router_.match(req->get_method(), req->get_absolute_path());
              if (!router_entry.has_value()) {
                res->not_found_404();
                on_send(res);
                return;
              }
              switch (router_entry->second) {
                case common::execution_policy::kSync:
                  router_entry->first(*req, *res);
                  on_send(res);
                  break;
                case common::execution_policy::kAsync:
                  thread_pool_->enqueue([req, res, on_send, router_entry]() {
                    router_entry->first(*req, *res);
                    on_send(res);
                  });
                  break;
              }
              break;
            }
            case target::kAuthorityForm:
              // CONNECT tunnelling (RFC 9110 §9.3.6) is deliberately deferred
              // to Doba's future client (dial-out) module: it will open the
              // outbound connection to the target authority already owned by
              // the request (req->get_target_authority_host()/_port()) and
              // drive the raw-byte relay, keeping this server-side transport
              // agnostic of CONNECT. Until that module exists, the request
              // must not be left unanswered.
              res->not_implemented_501();
              on_send(res);
              return;
            case target::kAsteriskForm:
              // OPTIONS * (RFC 9110 §9.3.7) addresses the server in general
              // rather than a specific resource; acknowledge it without
              // routing to a handler.
              res->ok_200();
              on_send(res);
              return;
            default:
              res->bad_request_400();
              on_send(res);
              return;
          }
        };
    transport_.on_bad_request = [](std::string_view reason,
                                   std::shared_ptr<RSty> res) {
      res->bad_request_400().set_body(reason);
    };
    transport_.on_connection = [this]() { connections_++; };
    transport_.on_disconnection = [this]() { connections_--; };
  }
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
    common::date_server::get().start();
    thread_pool_ = std::make_shared<common::thread_pool>(
        std::thread::hardware_concurrency());
    transport_.start(port);
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +=========================================================================+
  void stop() {
    if (thread_pool_) thread_pool_->stop();
    transport_.stop();
  }
  // +=========================================================================+
  // | [>] add_route                                                ( public ) |
  // +=========================================================================+
  server& add_route(
      const std::string& method, const std::string& route, FNty fn,
      common::execution_policy policy = common::execution_policy::kSync) {
    router_.add(method, route, fn, policy);
    return *this;
  }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  std::shared_ptr<common::thread_pool> thread_pool_;
  std::atomic<uint32_t> connections_{0};
  TRty<RQty, RSty, DEty> transport_;
  ROty<FNty> router_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
