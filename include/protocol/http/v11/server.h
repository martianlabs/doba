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

#include <mutex>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "common/date_server.h"
#include "transport/server/tcpip.h"
#include "protocol/http/common/router.h"
#include "protocol/http/v11/contracts.h"
#include "protocol/http/v11/engine.h"
#include "protocol/http/v11/request.h"
#include "protocol/http/v11/response.h"

namespace martianlabs::doba::protocol::http::v11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] server                                                      ( class ) |
// +---------------------------------------------------------------------------+
// | Composes the protocol engine and server transport.                        |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   RQty - request being used (HTTP/1.1 by default).                        |
// |   RSty - response being used (HTTP/1.1 by default).                       |
// |   ROty - router being used.                                               |
// |   ENty - engine being used (HTTP/1.1 by default).                         |
// |   TRty - transport being used (tcp/ip by default).                        |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty = request, typename RSty = response,
          typename ROty = router<RQty, RSty>,
          protocol::contracts::engine ENty = engine<RQty, RSty, ROty>,
          template <typename, typename> class TRty = transport::server::tcpip>
  requires contracts::server<ROty, ENty, TRty>
class server {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  explicit server(
      typename TRty<ENty, engine_factory<ENty, ROty>>::policies_type
          transport_configuration = {},
      typename ENty::policies_type engine_configuration = {})
      : transport_{std::move(transport_configuration),
                   make_engine_factory<ENty>(std::move(engine_configuration),
                                             router_)} {
    transport_.set_on_connection([]() {});
    transport_.set_on_disconnection([]() {});
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
  void start() {
    std::lock_guard<std::mutex> lock(locked_mutex_);
    if (locked_) return;
    common::date_server::get().start();
    try {
      transport_.start();
    } catch (...) {
      common::date_server::get().stop();
      throw;
    }
    locked_ = true;
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +=========================================================================+
  void stop() {
    std::lock_guard<std::mutex> lock(locked_mutex_);
    if (!locked_) return;
    transport_.stop();
    common::date_server::get().stop();
    locked_ = false;
  }
  // +=========================================================================+
  // | [>] add_route                                                ( public ) |
  // +=========================================================================+
  template <typename Hty>
    requires(router_handler_lambda<Hty> || router_async_handler_lambda<Hty>)
  server& add_route(std::string_view method, std::string_view route,
                    Hty handler) {
    std::lock_guard<std::mutex> lock(locked_mutex_);
    if (locked_) {
      throw std::runtime_error("Cannot add route when the server is running");
    }
    router_.add(method, route, std::move(handler));
    return *this;
  }
  // +=========================================================================+
  // | [>] add_controller                                           ( public ) |
  // +=========================================================================+
  template <typename Cty, typename... Args>
  server& add_controller(Args&&... args) {
    std::lock_guard<std::mutex> lock(locked_mutex_);
    if (locked_) {
      throw std::runtime_error(
          "Cannot add controller when the server is running");
    }
    router_.template add_controller<Cty>(std::forward<Args>(args)...);
    return *this;
  }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  TRty<ENty, engine_factory<ENty, ROty>> transport_;
  ROty router_;
  std::mutex locked_mutex_;
  bool locked_{false};
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
