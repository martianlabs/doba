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

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "common/reader.h"
#include "protocol/http/v11/server.h"

namespace {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] usings                                                     ( public ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
namespace common = martianlabs::doba::common;
namespace http = martianlabs::doba::protocol::http::v11;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] custom_transport                                            ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename ENty, typename FNty>
class custom_transport {
 public:
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using policies_type = std::string*;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  custom_transport(policies_type output, FNty create_engine)
      : output_(output), create_engine_(std::move(create_engine)) {}
  // +=========================================================================+
  // | [>] set_on_connection                                        ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_connection(std::function<void()> callback) {
    on_connection_ = std::move(callback);
  }
  // +=========================================================================+
  // | [>] set_on_disconnection                                     ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_disconnection(std::function<void()> callback) {
    on_disconnection_ = std::move(callback);
  }
  // +=========================================================================+
  // | [>] start                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  void start() {
    connection_.reset(new ENty(create_engine_()));
    connection_->set_on_send(
        [this](std::string_view head, std::string_view body,
               std::unique_ptr<common::reader> source) {
          output_->append(head);
          output_->append(body);
          if (source) source->read_all(*output_);
        });
    connection_->set_on_close([this]() { close_requested_ = true; });
    on_connection_();

    constexpr std::string_view request =
        "GET /hello HTTP/1.1\r\nHost: example.com\r\n"
        "Connection: close\r\n\r\n";
    connection_->on_bytes_received(request.data(), request.size(),
                                   request.size());
    if (close_requested_) stop();
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  void stop() {
    if (!connection_) return;
    connection_.reset();
    on_disconnection_();
  }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  policies_type output_;
  FNty create_engine_;
  std::unique_ptr<ENty> connection_;
  std::function<void()> on_connection_;
  std::function<void()> on_disconnection_;
  bool close_requested_{false};
};
}  // namespace

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// | Runs doba's HTTP/1.1 server on an in-memory transport.                    |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main() {
  using routes_type =
      martianlabs::doba::protocol::http::router<http::request, http::response>;
  using engine_type = http::engine<http::request, http::response>;

  std::string output;
  http::server<http::request, http::response, routes_type, engine_type,
               custom_transport>
      server(&output);
  server.add_route("GET", "/hello",
                   [](const http::request&, http::response& response) {
                     response.ok_200();
                     response.set_body("hello from memory");
                   });
  server.start();
  server.stop();
  std::cout << output;
  return 0;
}
