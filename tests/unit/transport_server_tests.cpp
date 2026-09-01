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
#include <condition_variable>
#include <coroutine>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <utility>

#include "protocol/deserialization.h"
#include "protocol/serialization.h"
#include "test_helper.h"
#include "transport/server/tcpip.h"

namespace {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] deferred_signal                                             ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class deferred_signal {
 public:
  bool await_ready() const noexcept { return false; }
  void await_suspend(std::coroutine_handle<> continuation) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      continuation_ = continuation;
    }
    condition_.notify_all();
  }
  void await_resume() const noexcept {}
  bool wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    return condition_.wait_for(lock, std::chrono::seconds(3),
                               [this]() { return continuation_ != nullptr; });
  }
  void resume() {
    std::coroutine_handle<> continuation;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      continuation = std::exchange(continuation_, nullptr);
    }
    if (continuation) continuation.resume();
  }

 private:
  std::mutex mutex_;
  std::condition_variable condition_;
  std::coroutine_handle<> continuation_;
};

struct transport_request {
  explicit transport_request(char in_value) : value(in_value) {}
  char value;
};

struct transport_response {
  std::unique_ptr<martianlabs::doba::protocol::serialization_result>
  serialize() {
    auto result = std::make_unique<
        martianlabs::doba::protocol::serialization_result>();
    result->prefix = std::move(value);
    return result;
  }
  std::string value;
};

template <typename RQty, typename RSty>
class transport_decoder {
 public:
  std::size_t accumulate(char* buffer, std::size_t size) {
    if (!size || ready_) return 0;
    value_ = *buffer;
    ready_ = true;
    return 1;
  }
  martianlabs::doba::protocol::deserialization_result<RQty> deserialize() {
    if (!ready_) {
      return {martianlabs::doba::protocol::deserialization_status::
                  kMoreBytesNeeded};
    }
    ready_ = false;
    return {std::make_shared<RQty>(value_)};
  }

 private:
  char value_{0};
  bool ready_{false};
};

class client_socket {
 public:
  client_socket() = default;
  client_socket(const client_socket&) = delete;
  client_socket(client_socket&&) noexcept = delete;
  ~client_socket() { close(); }
  client_socket& operator=(const client_socket&) = delete;
  client_socket& operator=(client_socket&&) noexcept = delete;
  bool connect(uint16_t port) {
    close();
    socket_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_ == invalid_socket()) return false;
#ifdef _WIN32
    DWORD timeout = 3000;
    ::setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO,
                 reinterpret_cast<const char*>(&timeout), sizeof(timeout));
#else
    timeval timeout{3, 0};
    ::setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
#endif
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (::connect(socket_, reinterpret_cast<sockaddr*>(&address),
                  sizeof(address)) == 0) {
      return true;
    }
    close();
    return false;
  }
  bool send(char value) {
    return ::send(socket_, &value, 1, 0) == 1;
  }
  std::string receive(std::size_t size) {
    std::string result(size, '\0');
    std::size_t received = 0;
    while (received < size) {
      int count = ::recv(socket_, result.data() + received,
                         static_cast<int>(size - received), 0);
      if (count <= 0) return {};
      received += static_cast<std::size_t>(count);
    }
    return result;
  }
  void close() {
    if (socket_ == invalid_socket()) return;
#ifdef _WIN32
    ::shutdown(socket_, SD_BOTH);
    ::closesocket(socket_);
#else
    ::shutdown(socket_, SHUT_RDWR);
    ::close(socket_);
#endif
    socket_ = invalid_socket();
  }

 private:
#ifdef _WIN32
  static SOCKET invalid_socket() { return INVALID_SOCKET; }
  SOCKET socket_{INVALID_SOCKET};
#else
  static int invalid_socket() { return -1; }
  int socket_{-1};
#endif
};

uint16_t find_available_port() {
#ifdef _WIN32
  SOCKET socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (socket == INVALID_SOCKET) return 0;
#else
  int socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (socket == -1) return 0;
#endif
  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_port = 0;
  address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  if (::bind(socket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) !=
      0) {
#ifdef _WIN32
    ::closesocket(socket);
#else
    ::close(socket);
#endif
    return 0;
  }
  socklen_t size = sizeof(address);
  if (::getsockname(socket, reinterpret_cast<sockaddr*>(&address), &size) !=
      0) {
#ifdef _WIN32
    ::closesocket(socket);
#else
    ::close(socket);
#endif
    return 0;
  }
#ifdef _WIN32
  ::closesocket(socket);
#else
  ::close(socket);
#endif
  return ntohs(address.sin_port);
}

bool wait_for_count(const std::atomic<std::size_t>& value,
                    std::size_t expected) {
  auto timeout = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (std::chrono::steady_clock::now() < timeout) {
    if (value.load() >= expected) return true;
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
  return value.load() >= expected;
}

martianlabs::doba::common::task<transport_response> make_response(
    std::shared_ptr<const transport_request> request,
    std::shared_ptr<deferred_signal> signal) {
  co_await *signal;
  transport_response response;
  response.value = request->value == 'A' ? "async" : "discarded";
  co_return response;
}
}  // namespace

// +===========================================================================+
// | [>] transport response delivery                             ( test-case ) |
// +===========================================================================+
DOBA_TEST("transport delivers immediate and deferred responses") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  uint16_t port = find_available_port();
  DOBA_EXPECT(port != 0);
  std::shared_ptr<deferred_signal> signals[] = {
      std::make_shared<deferred_signal>(),
      std::make_shared<deferred_signal>()};
  std::atomic<std::size_t> deferred = 0;
  std::atomic<std::size_t> disconnected = 0;
  server.set_on_request(
      [&signals, &deferred](const std::shared_ptr<transport_request>& request,
                            transport_response& response)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        if (request->value == 'S') {
          response.value = "sync";
          return std::nullopt;
        }
        std::size_t index = deferred.fetch_add(1);
        return make_response(
            std::shared_ptr<const transport_request>(request), signals[index]);
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection([&disconnected]() { disconnected.fetch_add(1); });
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  {
    client_socket client;
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send('S'));
    DOBA_EXPECT_EQUAL(client.receive(4), "sync");
  }
  DOBA_EXPECT(wait_for_count(disconnected, 1));

  {
    client_socket client;
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send('A'));
    DOBA_EXPECT(signals[0]->wait());
    signals[0]->resume();
    DOBA_EXPECT_EQUAL(client.receive(5), "async");
  }
  DOBA_EXPECT(wait_for_count(disconnected, 2));

  {
    client_socket client;
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send('D'));
    DOBA_EXPECT(signals[1]->wait());
  }
  DOBA_EXPECT(wait_for_count(disconnected, 3));
  signals[1]->resume();

  {
    client_socket client;
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send('S'));
    DOBA_EXPECT_EQUAL(client.receive(4), "sync");
  }
  DOBA_EXPECT(wait_for_count(disconnected, 4));
  server.stop();
}
