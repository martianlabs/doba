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

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

#include "protocol/processing_result.h"
#include "tcpip_client.h"
#include "test_helper.h"
#include "../../../helpers/concurrency.h"

namespace {
using martianlabs::doba::protocol::processing_result;
// Intercept APIs only in this executable; keep production and the driver
// hook-free. Watch operations while the submitting call is still active.
std::atomic<OVERLAPPED*> receive_operation{nullptr}, send_operation{nullptr};
// Completions seen by another real IOCP worker.
std::atomic<bool> receive_seen{false}, send_seen{false};
// Watch the first operation of each kind, not later idle receives.
std::atomic<bool> receive_watched{false}, send_watched{false};
// Fail within a bound if no packet arrives.
std::atomic<bool> timed_out{false};

// +---------------------------------------------------------------------------+
// | [>] wait_for_packet                                          ( function ) |
// +---------------------------------------------------------------------------+
void wait_for_packet(const std::atomic<bool>& seen) {
  const auto deadline = std::chrono::steady_clock::now() +
                        std::chrono::seconds{5};
  while (!seen.load()) {
    if (std::chrono::steady_clock::now() >= deadline) {
      timed_out = true;
      return;
    }
    std::this_thread::yield();
  }
}

// +---------------------------------------------------------------------------+
// | [>] tracked_receive                                          ( function ) |
// +---------------------------------------------------------------------------+
int WSAAPI tracked_receive(SOCKET socket, LPWSABUF buffers, DWORD count,
                          LPDWORD bytes, LPDWORD flags,
                          LPWSAOVERLAPPED operation,
                          LPWSAOVERLAPPED_COMPLETION_ROUTINE completion) {
  const bool watch = !receive_watched.exchange(true);
  if (watch) receive_operation = operation;
  const int result = ::WSARecv(socket, buffers, count, bytes, flags,
                                operation, completion);
  const int error = WSAGetLastError();
  if (watch && (result == 0 || error == WSA_IO_PENDING))
    wait_for_packet(receive_seen);
  WSASetLastError(error);
  return result;
}

// +---------------------------------------------------------------------------+
// | [>] tracked_send                                             ( function ) |
// +---------------------------------------------------------------------------+
int WSAAPI tracked_send(SOCKET socket, LPWSABUF buffers, DWORD count,
                       LPDWORD bytes, DWORD flags,
                       LPWSAOVERLAPPED operation,
                       LPWSAOVERLAPPED_COMPLETION_ROUTINE completion) {
  const bool watch = !send_watched.exchange(true);
  if (watch) send_operation = operation;
  const int result = ::WSASend(socket, buffers, count, bytes, flags,
                                operation, completion);
  const int error = WSAGetLastError();
  if (watch && (result == 0 || error == WSA_IO_PENDING))
    wait_for_packet(send_seen);
  WSASetLastError(error);
  return result;
}

// +---------------------------------------------------------------------------+
// | [>] tracked_completion                                       ( function ) |
// +---------------------------------------------------------------------------+
BOOL WINAPI tracked_completion(HANDLE port, LPDWORD bytes, PULONG_PTR key,
                               LPOVERLAPPED* operation, DWORD timeout) {
  const BOOL result = ::GetQueuedCompletionStatus(port, bytes, key,
                                                 operation, timeout);
  const DWORD error = GetLastError();
  if (!receive_seen.load() && *operation &&
      *operation == receive_operation.load()) receive_seen = true;
  if (!send_seen.load() && *operation &&
      *operation == send_operation.load()) send_seen = true;
  SetLastError(error);
  return result;
}
}  // namespace

#define WSARecv tracked_receive
#define WSASend tracked_send
#define GetQueuedCompletionStatus tracked_completion
#include "transport/server/tcp.h"
#undef GetQueuedCompletionStatus
#undef WSASend
#undef WSARecv

namespace {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] completion_engine                                          ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct completion_engine {
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using policies_type = int;
  // +=========================================================================+
  // | [>] set_on_send                                              ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_send(martianlabs::doba::protocol::send_delegate value) {
    output = std::move(value);
  }
  // +=========================================================================+
  // | [>] set_on_close                                             ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_close(std::function<void()>) {}
  // +=========================================================================+
  // | [>] on_bytes_received                                        ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_bytes_received(std::string_view input) {
    output(input, {}, nullptr);
    return {};
  }
  // +=========================================================================+
  // | [>] on_processing_turn                                       ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_processing_turn(bool) { return {}; }

  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // Send synchronously; the transport owns its copied response bytes.
  martianlabs::doba::protocol::send_delegate output;
};
}  // namespace

// +===========================================================================+
// | [>] completion precedes socket API return                   ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Windows handles IOCP packets dequeued before socket API returns") {
  namespace tr = martianlabs::doba::transport::server;
  using namespace std::chrono_literals;
  martianlabs::doba::tests::integration::tcpip_client client;
  const auto port = client.find_available_port();
  auto factory = [] { return completion_engine{}; };
  tr::tcp<completion_engine, decltype(factory)> transport(
      {.worker_count = 4, .ip = "127.0.0.1", .port = std::to_string(port)},
      factory);
  transport.set_on_connection([] {});
  transport.set_on_disconnection([] {});
  martianlabs::doba::tests::scope_exit cleanup([&] {
    client.close();
    transport.stop();
  });
  transport.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("first"));
  const auto bytes = client.receive(5, 10s);
  DOBA_EXPECT(bytes.has_value());
  if (bytes) DOBA_EXPECT_EQUAL(*bytes, "first");
  // Check that both watched calls returned and the connection still progresses.
  DOBA_EXPECT(client.send_all("next"));
  const auto next = client.receive(4, 10s);
  DOBA_EXPECT(next.has_value());
  if (next) DOBA_EXPECT_EQUAL(*next, "next");
  client.close();
  transport.stop();
  DOBA_EXPECT(receive_seen.load());
  DOBA_EXPECT(send_seen.load());
  DOBA_EXPECT(!timed_out.load());
}
