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

#include <cstddef>
#include <functional>
#include <string_view>
#include <utility>

#include "protocol/processing_result.h"
#include "common/signaler.h"
#include "protocol/contracts.h"
#include "server.h"
#include "transport/tcp.h"

namespace {
using martianlabs::doba::protocol::processing_result;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] usings                                                     ( public ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
namespace protocol = martianlabs::doba::protocol;
namespace transport = martianlabs::doba::transport;

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
  void set_on_close(std::function<void()> close) {
    on_close_ = std::move(close);
  }
  // +=========================================================================+
  // | [>] on_bytes_received                                        ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_bytes_received(std::string_view input) {
    while (!input.empty()) {
      const auto end = input.find('\n');
      const auto size = end == std::string_view::npos ? input.size() : end + 1;
      if (size > kMaximumLineSize - input_.size()) {
        on_close_();
        return {};
      }
      input_.append(input.substr(0, size));
      input.remove_prefix(size);
      if (end != std::string_view::npos) {
        on_send_("echo: ", input_, nullptr);
        input_.clear();
      }
    }
    return {};
  }
  // +=========================================================================+
  // | [>] on_processing_turn                                       ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_processing_turn(bool) { return {}; }


 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  // Limit the whole line, including its newline, regardless of receive size.
  static constexpr std::size_t kMaximumLineSize = 4096;
  // Keep an unfinished line until more input arrives.
  std::string input_;
  // Call these only during the current engine call.
  protocol::send_delegate on_send_;
  std::function<void()> on_close_;
};
}  // namespace

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// | Run a line-based protocol over TCP.                                       |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main() {
  auto create_engine = []() { return custom_protocol{}; };
  martianlabs::doba::server<
      custom_protocol, transport::tcp<custom_protocol, decltype(create_engine)>>
      server({.ip = "0.0.0.0", .port = "8080"}, create_engine);
  server.start();
  martianlabs::doba::common::signaler::wait();
  server.stop();
  return 0;
}
