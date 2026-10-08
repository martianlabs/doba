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
#include <functional>
#include <string_view>
#include <utility>

#include "common/signaler.h"
#include "protocol/send_delegate.h"
#include "transport/server/tcp.h"

namespace {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] usings                                                     ( public ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
namespace protocol = martianlabs::doba::protocol;
namespace transport = martianlabs::doba::transport::server;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] custom_protocol                                            ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct custom_protocol {
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  struct policies_type {};
  // +=========================================================================+
  // | [>] set_on_send                                              ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_send(protocol::send_delegate output) {
    on_send_ = std::move(output);
  }
  // +=========================================================================+
  // | [>] set_on_close                                             ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_close(std::function<void()>) {}
  // +=========================================================================+
  // | [>] on_bytes_received                                        ( public ) |
  // +-------------------------------------------------------------------------+
  std::size_t on_bytes_received(const char* buffer, std::size_t size,
                                std::size_t) {
    std::size_t consumed = 0;
    while (consumed < size) {
      const std::string_view remaining(buffer + consumed, size - consumed);
      const std::size_t end = remaining.find('\n');
      if (end == std::string_view::npos) break;
      const std::string_view line = remaining.substr(0, end + 1);
      on_send_("echo: ", line, nullptr);
      consumed += line.size();
    }
    return consumed;
  }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  protocol::send_delegate on_send_;
};
}  // namespace

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// | Runs a line-based protocol on doba's TCP transport.                       |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main() {
  auto create_engine = []() { return custom_protocol{}; };
  transport::tcp<custom_protocol, decltype(create_engine)> server(
      {.ip = "0.0.0.0", .port = "8080"}, create_engine);
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  server.start();
  martianlabs::doba::common::signaler::wait();
  server.stop();
  return 0;
}
