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

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <fcntl.h>
#include <functional>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/resource.h>
#include <unistd.h>
#include <utility>
#include <vector>

#include "protocol/processing_result.h"
#include "tcpip_client.h"
#include "transport/server/tcp.h"

namespace {
using martianlabs::doba::protocol::processing_result;
using martianlabs::doba::tests::integration::tcpip_client;
namespace tr = martianlabs::doba::transport::server;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] echo_engine                                                ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct echo_engine {
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using policies_type = int;
  // +=========================================================================+
  // | [>] set_on_send                                              ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_send(martianlabs::doba::protocol::send_delegate callback) {
    send = std::move(callback);
  }
  // +=========================================================================+
  // | [>] set_on_close                                             ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_close(std::function<void()>) {}
  // +=========================================================================+
  // | [>] on_bytes_received                                        ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_bytes_received(std::string_view bytes) {
    send(bytes, {}, nullptr);
    return {};
  }
  // +=========================================================================+
  // | [>] on_processing_turn                                       ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_processing_turn(bool) { return {}; }

  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // Use this transport callback only during input.
  martianlabs::doba::protocol::send_delegate send;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] descriptor_budget                                           ( class ) |
// +---------------------------------------------------------------------------+
// | Exhaust descriptors in this isolated process, not other tests.            |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class descriptor_budget {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  descriptor_budget() {
    descriptors_.reserve(128);
    if (::getrlimit(RLIMIT_NOFILE, &original_) != 0) {
      throw std::runtime_error("getrlimit failed");
    }
    auto limited = original_;
    limited.rlim_cur = std::min<rlim_t>(128, limited.rlim_cur);
    if (::setrlimit(RLIMIT_NOFILE, &limited) != 0) {
      throw std::runtime_error("setrlimit failed");
    }
  }
  descriptor_budget(const descriptor_budget&) = delete;
  ~descriptor_budget() {
    for (const int descriptor : descriptors_) ::close(descriptor);
    ::setrlimit(RLIMIT_NOFILE, &original_);
  }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  descriptor_budget& operator=(const descriptor_budget&) = delete;
  // +=========================================================================+
  // | [>] leave_slots                                              ( public ) |
  // +-------------------------------------------------------------------------+
  void leave_slots(std::size_t slots) {
    for (;;) {
      const int descriptor = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
      if (descriptor < 0) {
        if (errno != EMFILE || descriptors_.empty()) {
          throw std::runtime_error("could not exhaust descriptors");
        }
        break;
      }
      descriptors_.push_back(descriptor);
    }
    while (slots--) {
      ::close(descriptors_.back());
      descriptors_.pop_back();
    }
  }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  // Restore before destroying the transport, even after failed checks.
  rlimit original_{};
  // Use dummy files to fill the descriptor table.
  std::vector<int> descriptors_;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] make_engine                                              ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
// Use one factory type in both cases. A healthy exchange warms UBSan's thread
// type metadata before partial_start uses every descriptor; LLVM may open a
// file on first inspection.
echo_engine make_engine() { return {}; }

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] exercise                                                 ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
bool exercise() {
  using namespace std::chrono_literals;
  auto factory = make_engine;
  tcpip_client existing;
  const auto port = existing.find_available_port();
  if (!port) return false;
  tr::tcp<echo_engine, decltype(factory)> transport(
      {.worker_count = 1, .ip = "127.0.0.1", .port = std::to_string(port)},
      factory);
  transport.start();
  if (!existing.connect(port) || !existing.send_all("ready") ||
      existing.receive(5) != std::optional<std::string>("ready")) return false;
  {
    descriptor_budget budget;
    budget.leave_slots(1);
    tcpip_client rejected;
    // The client takes the last slot; accept4 must fail with EMFILE.
    if (rejected.connect(port)) {
      if (!rejected.wait_for_close(3s) && !rejected.connection_reset()) {
        return false;
      }
    } else if (rejected.native_error() != ECONNREFUSED &&
               !rejected.connection_reset()) {
      return false;
    }
    if (!existing.send_all("alive") ||
        existing.receive(5) != std::optional<std::string>("alive")) {
      return false;
    }
  }
  transport.stop();
  transport.start();
  tcpip_client restarted;
  if (!restarted.connect(port) || !restarted.send_all("again")) return false;
  return restarted.receive(5) == std::optional<std::string>("again");
}
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] partial_start                                            ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
bool partial_start() {
  auto factory = make_engine;
  tcpip_client probe;
  const auto port = probe.find_available_port();
  if (!port) return false;
  tr::tcp<echo_engine, decltype(factory)> transport(
      {.worker_count = 4, .ip = "127.0.0.1", .port = std::to_string(port)},
      factory);
  {
    descriptor_budget budget;
    // Each worker needs three descriptors. Force the second to fail and unwind
    // both.
    budget.leave_slots(4);
    bool rejected = false;
    try { transport.start(); }
    catch (const std::runtime_error&) { rejected = true; }
    if (!rejected) return false;
    std::vector<int> recovered;
    for (int index = 0; index < 4; ++index) {
      const int descriptor = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
      if (descriptor < 0) {
        for (const int open : recovered) ::close(open);
        return false;
      }
      recovered.push_back(descriptor);
    }
    for (const int open : recovered) ::close(open);
  }
  transport.start();
  if (!probe.connect(port) || !probe.send_all("ready")) return false;
  return probe.receive(5) == std::optional<std::string>("ready");
}
}  // namespace

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main() {
  try {
    if (exercise() && partial_start()) return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
  }
  std::cerr << "Linux descriptor exhaustion test failed\n";
  return 1;
}
