//                              _       _
//                           __| | ___ | |__   __ _
//                          / _` |/ _ \| '_ \ / _` |
//                         | (_| | (_) | |_) | (_| |
//                          \__,_|\___/|_.__/ \__,_|
//
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/LICENSE-2.0
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

#ifndef martianlabs_doba_http_server_h
#define martianlabs_doba_http_server_h

#include <mutex>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "common/date_server.h"
#include "transport/server/tcp.h"
#include "protocol/http/common/router.h"
#include "protocol/http/v11/contracts.h"
#include "protocol/http/v11/engine.h"
#include "protocol/http/v11/request.h"
#include "protocol/http/v11/response.h"
#include "server.h"

namespace martianlabs::doba {
namespace detail {
template <typename ROty>
class http_router_storage {
 protected:
  ROty router_;
};
}  // namespace detail

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] http_server                                                 ( class ) |
// +---------------------------------------------------------------------------+
// | Composes HTTP/1.1 routing with a protocol engine and transport.           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty = protocol::http::v11::request,
          typename RSty = protocol::http::v11::response,
          typename ROty = protocol::http::router<RQty, RSty>,
          protocol::contracts::engine ENty =
              protocol::http::v11::engine<RQty, RSty, ROty>,
          template <typename, typename> class TRty = transport::server::tcp>
  requires protocol::http::v11::contracts::server<ROty, ENty, TRty>
class http_server
    : private detail::http_router_storage<ROty>,
      public server<ENty, TRty<ENty,
                               protocol::http::v11::engine_factory<ENty,
                                                                    ROty>>> {
 private:
  // +=========================================================================+
  // | [>] USINGs                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  using base_type = server<ENty, TRty<ENty,
                                      protocol::http::v11::engine_factory<
                                          ENty, ROty>>>;

 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  explicit http_server(
      typename TRty<ENty, protocol::http::v11::engine_factory<
                              ENty, ROty>>::policies_type
          transport_configuration = {},
      typename ENty::policies_type engine_configuration = {})
      : base_type{std::move(transport_configuration),
                  protocol::http::v11::make_engine_factory<ENty>(
                      std::move(engine_configuration), this->router_)} {}
  ~http_server() { stop(); }
  // +=========================================================================+
  // | [>] start                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  void start() {
    base_type::start([]() { common::date_server::get().start(); },
                     []() { common::date_server::get().stop(); });
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  void stop() {
    base_type::stop(
        [this]() {
          if constexpr (requires(const ROty& routes) { routes.wait_async(); }) {
            this->router_.wait_async();
          }
        },
        []() { common::date_server::get().stop(); });
  }
  // +=========================================================================+
  // | [>] add_route                                                ( public ) |
  // +-------------------------------------------------------------------------+
  template <typename Hty>
    requires protocol::http::router_handler_lambda<Hty>
  http_server& add_route(std::string_view method, std::string_view route,
                         Hty handler) {
    std::lock_guard<std::mutex> lock(this->locked_mutex_);
    if (this->locked_) {
      throw std::runtime_error("Cannot add route when the server is running");
    }
    this->router_.add(method, route, std::move(handler));
    return *this;
  }
  // +=========================================================================+
  // | [>] add_controller                                           ( public ) |
  // +-------------------------------------------------------------------------+
  template <typename CTty, typename... Args>
  http_server& add_controller(Args&&... args) {
    std::lock_guard<std::mutex> lock(this->locked_mutex_);
    if (this->locked_) {
      throw std::runtime_error(
          "Cannot add controller when the server is running");
    }
    this->router_.template add_controller<CTty>(std::forward<Args>(args)...);
    return *this;
  }
};
}  // namespace martianlabs::doba

#endif
