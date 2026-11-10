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
#include <utility>

#include "protocol/processing_result.h"
#include "tcpip_client.h"
#include "test_helper.h"
#include "../../helpers/concurrency.h"

namespace {
using martianlabs::doba::protocol::processing_result;
// Isolated native API faults: failed submissions never post a completion.
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] fault                                                        ( enum ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
enum class fault { kNone, kPort, kAccept, kAssociate, kReceive, kSend };
// One armed failure is consumed atomically by the matching API.
std::atomic<fault> armed{fault::kNone};
// The second AcceptEx fails after the first real operation was submitted.
std::atomic<unsigned> accepts{0};
// Accepted socket whose ownership transfer is being checked.
std::atomic<SOCKET> watched{INVALID_SOCKET};
std::atomic<unsigned> closes{0};
// Real entry point loaded before substituting the test-only accept function.
LPFN_ACCEPTEX native_accept = nullptr;

// +---------------------------------------------------------------------------+
// | [>] consume_fault                                            ( function ) |
// +---------------------------------------------------------------------------+
bool consume_fault(fault expected) {
  return armed.compare_exchange_strong(expected, fault::kNone);
}

// +---------------------------------------------------------------------------+
// | [>] tracked_port                                             ( function ) |
// +---------------------------------------------------------------------------+
HANDLE WINAPI tracked_port(HANDLE file, HANDLE port, ULONG_PTR key,
                           DWORD workers) {
  if ((file == INVALID_HANDLE_VALUE && consume_fault(fault::kPort)) ||
      (port && accepts.load() && consume_fault(fault::kAssociate))) {
    if (port) watched = reinterpret_cast<SOCKET>(file);
    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    return nullptr;
  }
  return ::CreateIoCompletionPort(file, port, key, workers);
}

// +---------------------------------------------------------------------------+
// | [>] tracked_close                                            ( function ) |
// +---------------------------------------------------------------------------+
int WSAAPI tracked_close(SOCKET socket) {
  if (socket == watched.load()) ++closes;
  return ::closesocket(socket);
}

// +---------------------------------------------------------------------------+
// | [>] tracked_accept                                           ( function ) |
// +---------------------------------------------------------------------------+
BOOL PASCAL tracked_accept(SOCKET listener, SOCKET socket, PVOID buffer,
                          DWORD bytes, DWORD local, DWORD remote,
                          LPDWORD received, LPOVERLAPPED operation) {
  if (++accepts == 2 && consume_fault(fault::kAccept)) {
    WSASetLastError(WSAENOBUFS);
    return FALSE;
  }
  return native_accept(listener, socket, buffer, bytes, local, remote,
                       received, operation);
}

// +---------------------------------------------------------------------------+
// | [>] tracked_ioctl                                            ( function ) |
// +---------------------------------------------------------------------------+
int WSAAPI tracked_ioctl(SOCKET socket, DWORD code, LPVOID input,
                         DWORD input_size, LPVOID output, DWORD output_size,
                         LPDWORD bytes, LPWSAOVERLAPPED operation,
                         LPWSAOVERLAPPED_COMPLETION_ROUTINE completion) {
  const int result = ::WSAIoctl(socket, code, input, input_size, output,
                                output_size, bytes, operation, completion);
  if (result == 0 && code == SIO_GET_EXTENSION_FUNCTION_POINTER) {
    native_accept = *static_cast<LPFN_ACCEPTEX*>(output);
    *static_cast<LPFN_ACCEPTEX*>(output) = tracked_accept;
  }
  return result;
}

// +---------------------------------------------------------------------------+
// | [>] tracked_receive                                          ( function ) |
// +---------------------------------------------------------------------------+
int WSAAPI tracked_receive(SOCKET socket, LPWSABUF buffers, DWORD count,
                          LPDWORD bytes, LPDWORD flags,
                          LPWSAOVERLAPPED operation,
                          LPWSAOVERLAPPED_COMPLETION_ROUTINE completion) {
  if (consume_fault(fault::kReceive)) {
    WSASetLastError(WSAENOBUFS);
    return SOCKET_ERROR;
  }
  return ::WSARecv(socket, buffers, count, bytes, flags, operation, completion);
}

// +---------------------------------------------------------------------------+
// | [>] tracked_send                                             ( function ) |
// +---------------------------------------------------------------------------+
int WSAAPI tracked_send(SOCKET socket, LPWSABUF buffers, DWORD count,
                       LPDWORD bytes, DWORD flags,
                       LPWSAOVERLAPPED operation,
                       LPWSAOVERLAPPED_COMPLETION_ROUTINE completion) {
  if (consume_fault(fault::kSend)) {
    WSASetLastError(WSAENOBUFS);
    return SOCKET_ERROR;
  }
  return ::WSASend(socket, buffers, count, bytes, flags, operation, completion);
}
}  // namespace

#define CreateIoCompletionPort tracked_port
#define closesocket tracked_close
#define WSAIoctl tracked_ioctl
#define WSARecv tracked_receive
#define WSASend tracked_send
#include "transport/tcp.h"
#undef WSASend
#undef WSARecv
#undef WSAIoctl
#undef closesocket
#undef CreateIoCompletionPort

namespace {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] lifecycle_engine                                           ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct lifecycle_engine {
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
  // Synchronous engine output; the transport owns the copied response bytes.
  martianlabs::doba::protocol::send_delegate output;
};
}  // namespace

namespace {
// +---------------------------------------------------------------------------+
// | [>] check_fault                                              ( function ) |
// +---------------------------------------------------------------------------+
void check_fault(fault injected) {
  namespace tr = martianlabs::doba::transport;
  using namespace std::chrono_literals;
  using client_type = martianlabs::doba::tests::integration::tcpip_client;
  client_type client;
  const auto port = client.find_available_port();
  auto factory = [] { return lifecycle_engine{}; };
  tr::tcp<lifecycle_engine, decltype(factory)> transport(
      {.worker_count = 4, .max_connections = 1, .ip = "127.0.0.1",
       .port = std::to_string(port)}, factory);
  std::atomic<unsigned> connected{0}, disconnected{0};
  transport.set_on_connection([&] { ++connected; });
  transport.set_on_disconnection([&] { ++disconnected; });
  martianlabs::doba::tests::scope_exit cleanup([&] {
    watched = INVALID_SOCKET;
    client.close();
    transport.stop();
  });
  accepts = closes = 0;
  watched = INVALID_SOCKET;
  armed = injected;
  if (injected == fault::kPort || injected == fault::kAccept) {
    bool failed = false;
    try { transport.start(); } catch (...) { failed = true; }
    DOBA_EXPECT(failed);
    DOBA_EXPECT(armed.load() == fault::kNone);
    if (injected == fault::kAccept) DOBA_EXPECT_EQUAL(accepts.load(), 2u);
    // start() must already have joined workers and drained the real accept.
    transport.stop();
    transport.start();
  } else {
    transport.start();
    DOBA_EXPECT(client.connect(port));
    if (injected == fault::kSend) DOBA_EXPECT(client.send_all("fail"));
    DOBA_EXPECT(client.wait_for_close(5s));
    DOBA_EXPECT(armed.load() == fault::kNone);
    client.close();
  }
  // A one-connection budget proves the failed context released its slot.
  // Socket EOF can precede destruction of the reservation by a few cycles.
  bool healthy = false;
  const auto deadline = std::chrono::steady_clock::now() + 5s;
  do {
    client.close();
    if (client.connect(port, 500ms) && client.send_all("ok", 500ms)) {
      const auto response = client.receive(2, 500ms);
      healthy = response && *response == "ok";
    }
  } while (!healthy && std::chrono::steady_clock::now() < deadline);
  DOBA_EXPECT(healthy);
  if (injected == fault::kAssociate) DOBA_EXPECT_EQUAL(closes.load(), 1u);
  // Stop with the healthy connection's real receive still pending.
  watched = INVALID_SOCKET;
  transport.stop();
  client.close();
  DOBA_EXPECT_EQUAL(connected.load(), disconnected.load());
  transport.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("restart"));
  const auto response = client.receive(7, 5s);
  DOBA_EXPECT(response && *response == "restart");
  transport.stop();
  client.close();
  DOBA_EXPECT_EQUAL(connected.load(), disconnected.load());
}
}  // namespace

// +===========================================================================+
// | [>] failed startup and restart                              ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Windows restarts after IOCP creation failure") {
  check_fault(fault::kPort);
}
DOBA_TEST("Windows drains a pending accept after partial startup failure") {
  check_fault(fault::kAccept);
}
// +===========================================================================+
// | [>] failed socket ownership and submissions                 ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Windows closes an unassociated context socket exactly once") {
  check_fault(fault::kAssociate);
}
DOBA_TEST("Windows closes after immediate receive submission failure") {
  check_fault(fault::kReceive);
}
DOBA_TEST("Windows closes after immediate send submission failure") {
  check_fault(fault::kSend);
}
