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

#include <array>
#include <atomic>
#include <barrier>
#include <chrono>
#include <condition_variable>
#include <coroutine>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <stop_token>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "common/byte_storage.h"
#include "common/task.h"
#include "protocol/deserialization.h"
#include "protocol/serialization.h"
#include "tcpip_client.h"
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
  transport_request(char in_value, std::string in_payload)
      : value(in_value), payload(std::move(in_payload)) {}
  char value;
  std::string payload;
};

enum class serialization_behavior {
  kNormal,
  kSource,
  kNull,
  kThrowStandard,
  kThrowUnknown
};

struct transport_response {
  std::unique_ptr<martianlabs::doba::protocol::serialization_result>
  serialize() {
    if (serialized) serialized->fetch_add(1);
    if (behavior == serialization_behavior::kNull) return nullptr;
    if (behavior == serialization_behavior::kThrowStandard) {
      throw std::runtime_error("Serialization error!");
    }
    if (behavior == serialization_behavior::kThrowUnknown) throw 1;
    auto result = std::make_unique<
        martianlabs::doba::protocol::serialization_result>();
    result->prefix = std::move(value);
    if (behavior == serialization_behavior::kSource) {
      martianlabs::doba::common::byte_storage storage;
      storage.write(source.data(), source.size());
      storage.finish(source.size());
      result->source.emplace(std::move(storage));
    }
    return result;
  }
  std::string value;
  std::string source;
  serialization_behavior behavior{serialization_behavior::kNormal};
  std::shared_ptr<std::atomic<std::size_t>> serialized;
};

template <typename RQty, typename RSty>
class transport_decoder {
 public:
  std::size_t accumulate(char* buffer, std::size_t size) {
    if (!size || ready_) return 0;
    if (*buffer == 'Z') return 0;
    if (*buffer == 'O') return size + 1;
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
    if (value_ == 'E') {
      return {martianlabs::doba::protocol::deserialization_status::
                  kInvalidSource,
              42};
    }
    if (value_ == 'N') {
      return martianlabs::doba::protocol::deserialization_result<RQty>(
          std::shared_ptr<RQty>());
    }
    return {std::make_shared<RQty>(value_),
            value_ == 'C'
                ? martianlabs::doba::protocol::channel_intent::kClose
                : martianlabs::doba::protocol::channel_intent::kKeep};
  }

 private:
  char value_{0};
  bool ready_{false};
};

template <typename RQty, typename RSty>
class framed_decoder {
 public:
  std::size_t accumulate(char* buffer, std::size_t size) {
    data_.append(buffer, size);
    return size;
  }
  martianlabs::doba::protocol::deserialization_result<RQty> deserialize() {
    using martianlabs::doba::protocol::channel_intent;
    using martianlabs::doba::protocol::deserialization_result;
    using martianlabs::doba::protocol::deserialization_status;
    if (data_.size() < 2) {
      deserialization_result<RQty> result(
          deserialization_status::kMoreBytesNeeded);
      if (data_ == "I" && !interim_sent_) {
        result.interim = "interim";
        interim_sent_ = true;
      }
      return result;
    }
    std::size_t payload_size =
        static_cast<unsigned char>(data_[1]);
    if (data_.size() < payload_size + 2) {
      return {deserialization_status::kMoreBytesNeeded};
    }
    char type = data_[0];
    std::string payload = data_.substr(2, payload_size);
    data_.erase(0, payload_size + 2);
    interim_sent_ = false;
    return {std::make_shared<RQty>(type, std::move(payload)),
            type == 'C' ? channel_intent::kClose
                        : channel_intent::kKeep};
  }

 private:
  std::string data_;
  bool interim_sent_{false};
};

template <typename RQty, typename RSty>
class sized_decoder {
 public:
  std::size_t accumulate(char* buffer, std::size_t size) {
    data_.append(buffer, size);
    return size;
  }
  martianlabs::doba::protocol::deserialization_result<RQty> deserialize() {
    using martianlabs::doba::protocol::deserialization_status;
    if (data_.size() < 4) {
      return {deserialization_status::kMoreBytesNeeded};
    }
    std::size_t payload_size =
        (static_cast<std::size_t>(
             static_cast<unsigned char>(data_[0]))
         << 24) |
        (static_cast<std::size_t>(
             static_cast<unsigned char>(data_[1]))
         << 16) |
        (static_cast<std::size_t>(
             static_cast<unsigned char>(data_[2]))
         << 8) |
        static_cast<std::size_t>(
            static_cast<unsigned char>(data_[3]));
    if (data_.size() < payload_size + 4) {
      return {deserialization_status::kMoreBytesNeeded};
    }
    std::string payload = data_.substr(4, payload_size);
    data_.erase(0, payload_size + 4);
    return {std::make_shared<RQty>('P', std::move(payload))};
  }

 private:
  std::string data_;
};

std::string frame(char type, std::string_view payload) {
  std::string result;
  result.push_back(type);
  result.push_back(static_cast<char>(payload.size()));
  result.append(payload);
  return result;
}

std::string sized_frame(std::string_view payload) {
  std::size_t size = payload.size();
  std::string result;
  result.push_back(static_cast<char>((size >> 24) & 0xff));
  result.push_back(static_cast<char>((size >> 16) & 0xff));
  result.push_back(static_cast<char>((size >> 8) & 0xff));
  result.push_back(static_cast<char>(size & 0xff));
  result.append(payload);
  return result;
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

bool remains_equal(const std::atomic<std::size_t>& value,
                   std::size_t expected) {
  auto timeout = std::chrono::steady_clock::now() +
                 std::chrono::milliseconds(100);
  while (std::chrono::steady_clock::now() < timeout) {
    if (value.load() != expected) return false;
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
  return value.load() == expected;
}

martianlabs::doba::common::task<transport_response> make_response(
    std::shared_ptr<const transport_request> request,
    std::shared_ptr<deferred_signal> signal,
    std::shared_ptr<std::atomic<std::size_t>> serialized) {
  co_await *signal;
  transport_response response;
  switch (request->value) {
    case 'A':
      response.value = "async";
      break;
    case 'F':
      response.value = "first";
      break;
    case 'L':
      response.value = "second";
      break;
    default:
      response.value = "discarded";
      break;
  }
  response.serialized = std::move(serialized);
  co_return response;
}

martianlabs::doba::common::task<transport_response> make_deferred_response(
    std::shared_ptr<deferred_signal> signal, transport_response response) {
  co_await *signal;
  co_return response;
}

martianlabs::doba::common::task<transport_response> make_cancellable_response(
    std::shared_ptr<deferred_signal> signal,
    std::shared_ptr<std::atomic<std::size_t>> serialized,
    std::shared_ptr<std::atomic<std::size_t>> completed) {
  co_await *signal;
  completed->fetch_add(1);
  transport_response response;
  response.value = "cancelled";
  response.serialized = std::move(serialized);
  co_return response;
}

martianlabs::doba::common::task<transport_response> make_failed_response(
    std::shared_ptr<deferred_signal> signal, bool standard) {
  co_await *signal;
  if (standard) throw std::runtime_error("Deferred handler error!");
  throw 1;
  co_return transport_response();
}
}  // namespace

// +===========================================================================+
// | [>] immediate response lifecycle                             ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip serves independent loopback connections") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> connected = 0;
  std::atomic<std::size_t> disconnected = 0;
  std::atomic<std::size_t> requests = 0;
  server.set_on_request(
      [&requests](const std::shared_ptr<transport_request>& request,
                  transport_response& response,
                  const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        requests.fetch_add(1);
        response.value = request->value == 'S' ? "sync" : "unexpected";
        return std::nullopt;
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([&connected]() { connected.fetch_add(1); });
  server.set_on_disconnection(
      [&disconnected]() { disconnected.fetch_add(1); });
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("S"));
  auto first_response = client.receive(4);
  DOBA_EXPECT(first_response.has_value());
  DOBA_EXPECT_EQUAL(*first_response, "sync");
  client.close();
  DOBA_EXPECT(wait_for_count(disconnected, 1));

  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("S"));
  auto second_response = client.receive(4);
  DOBA_EXPECT(second_response.has_value());
  DOBA_EXPECT_EQUAL(*second_response, "sync");
  client.close();
  DOBA_EXPECT(wait_for_count(connected, 2));
  DOBA_EXPECT(wait_for_count(disconnected, 2));
  DOBA_EXPECT_EQUAL(requests.load(), 2);
  server.stop();
}

// +===========================================================================+
// | [>] persistent connection reuse                              ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip reuses a connection after each completed response") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> requests = 0;
  server.set_on_request(
      [&requests](const std::shared_ptr<transport_request>& request,
                  transport_response& response,
                  const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        requests.fetch_add(1);
        response.value.assign(1, request->value);
        return std::nullopt;
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  DOBA_EXPECT(client.connect(port));
  for (char request : std::string_view("ABDS")) {
    DOBA_EXPECT(client.send_all(std::string_view(&request, 1)));
    auto response = client.receive(1);
    DOBA_EXPECT(response.has_value());
    DOBA_EXPECT_EQUAL((*response)[0], request);
  }
  DOBA_EXPECT_EQUAL(requests.load(), 4);
  client.close();
  server.stop();
}

// +===========================================================================+
// | [>] synchronous pipeline delivery                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip delivers batched synchronous responses in request order") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server.set_on_request(
      [](const std::shared_ptr<transport_request>& request,
         transport_response& response,
         const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        response.value.assign(1, request->value);
        return std::nullopt;
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("ABDS"));
  auto response = client.receive(4);
  DOBA_EXPECT(response.has_value());
  DOBA_EXPECT_EQUAL(*response, "ABDS");
  client.close();
  server.stop();
}

// +===========================================================================+
// | [>] fragmented request delivery                             ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip waits for every fragment before dispatching a request") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, framed_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> requests = 0;
  server.set_on_request(
      [&requests](const std::shared_ptr<transport_request>& request,
                  transport_response& response,
                  const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        requests.fetch_add(1);
        response.value = request->payload;
        return std::nullopt;
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  std::string request = frame('P', "fragmented");
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(std::string_view(request).substr(0, 1)));
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(50)));
  DOBA_EXPECT(client.send_all(std::string_view(request).substr(1, 3)));
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(50)));
  DOBA_EXPECT(client.send_all(std::string_view(request).substr(4)));
  auto response = client.receive(10);
  DOBA_EXPECT(response.has_value());
  DOBA_EXPECT_EQUAL(*response, "fragmented");
  DOBA_EXPECT_EQUAL(requests.load(), 1);
  client.close();
  server.stop();
}

// +===========================================================================+
// | [>] interim response delivery                               ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip sends one interim response before final dispatch") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, framed_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> requests = 0;
  server.set_on_request(
      [&requests](const std::shared_ptr<transport_request>& request,
                  transport_response& response,
                  const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        requests.fetch_add(1);
        response.value = request->payload;
        return std::nullopt;
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  std::string request = frame('I', "final");
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(std::string_view(request).substr(0, 1)));
  auto interim = client.receive(7);
  DOBA_EXPECT(interim.has_value());
  DOBA_EXPECT_EQUAL(*interim, "interim");
  DOBA_EXPECT_EQUAL(requests.load(), 0);
  DOBA_EXPECT(client.send_all(std::string_view(request).substr(1)));
  auto response = client.receive(5);
  DOBA_EXPECT(response.has_value());
  DOBA_EXPECT_EQUAL(*response, "final");
  DOBA_EXPECT_EQUAL(requests.load(), 1);
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(50)));
  client.close();
  server.stop();
}

// +===========================================================================+
// | [>] buffered request delivery                               ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip dispatches every complete buffered request") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, framed_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server.set_on_request(
      [](const std::shared_ptr<transport_request>& request,
         transport_response& response,
         const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        response.value = request->payload;
        return std::nullopt;
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  std::string requests = frame('P', "one") + frame('P', "two") +
                         frame('P', "three");
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(requests));
  auto response = client.receive(11);
  DOBA_EXPECT(response.has_value());
  DOBA_EXPECT_EQUAL(*response, "onetwothree");
  client.close();
  server.stop();
}

// +===========================================================================+
// | [>] receive buffer boundaries                               ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip preserves requests across receive buffer boundaries") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, sized_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<bool> valid = true;
  std::atomic<std::size_t> expected_size = 0;
  server.set_on_request(
      [&valid, &expected_size](
          const std::shared_ptr<transport_request>& request,
          transport_response& response,
          const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        std::size_t expected = expected_size.load();
        if (request->payload.size() != expected ||
            request->payload != std::string(expected, 'x')) {
          valid.store(false);
        }
        response.value = "ok";
        return std::nullopt;
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  DOBA_EXPECT(client.connect(port));
  constexpr std::size_t sizes[] = {8191, 8192, 8193, 16385};
  for (std::size_t size : sizes) {
    expected_size.store(size);
    DOBA_EXPECT(client.send_all(sized_frame(std::string(size, 'x'))));
    auto response = client.receive(2);
    DOBA_EXPECT(response.has_value());
    DOBA_EXPECT_EQUAL(*response, "ok");
  }
  DOBA_EXPECT(valid.load());
  client.close();
  server.stop();
}

// +===========================================================================+
// | [>] binary request delivery                                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip preserves binary request payloads") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, sized_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server.set_on_request(
      [](const std::shared_ptr<transport_request>& request,
         transport_response& response,
         const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        response.value = request->payload;
        return std::nullopt;
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  const char bytes[] = {'\0', '\x01', '\x7f',
                        static_cast<char>(0x80),
                        static_cast<char>(0xff)};
  std::string payload(bytes, sizeof(bytes));
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(sized_frame(payload)));
  auto response = client.receive(payload.size());
  DOBA_EXPECT(response.has_value());
  DOBA_EXPECT_EQUAL(*response, payload);
  client.close();
  server.stop();
}

// +===========================================================================+
// | [>] null decoded request rejection                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip rejects a successful decode without a request") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> requests = 0;
  std::atomic<int> rejection_code = -1;
  server.set_on_request(
      [&requests](const std::shared_ptr<transport_request>&,
                  transport_response&,
                  const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        requests.fetch_add(1);
        return std::nullopt;
      });
  server.set_on_bad_request(
      [&rejection_code](int code, std::string_view,
                        transport_response& response) {
        rejection_code.store(code);
        response.value = "invalid";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("N"));
  auto response = client.receive(7);
  DOBA_EXPECT(response.has_value());
  DOBA_EXPECT_EQUAL(*response, "invalid");
  DOBA_EXPECT(client.wait_for_close(std::chrono::seconds(3)));
  DOBA_EXPECT_EQUAL(requests.load(), 0);
  DOBA_EXPECT_EQUAL(rejection_code.load(), 0);
  server.stop();
}

// +===========================================================================+
// | [>] decoder accumulation rejection                         ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip rejects invalid decoder accumulation counts") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> requests = 0;
  std::atomic<std::size_t> rejections = 0;
  std::atomic<int> rejection_code = -1;
  server.set_on_request(
      [&requests](const std::shared_ptr<transport_request>&,
                  transport_response&,
                  const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        requests.fetch_add(1);
        return std::nullopt;
      });
  server.set_on_bad_request(
      [&rejections, &rejection_code](int code, std::string_view,
                                     transport_response& response) {
        rejections.fetch_add(1);
        rejection_code.store(code);
        response.value = "invalid";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  constexpr std::string_view invalid_requests[] = {"Z", "O"};
  for (std::string_view request : invalid_requests) {
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all(request));
    auto response = client.receive(7);
    DOBA_EXPECT(response.has_value());
    DOBA_EXPECT_EQUAL(*response, "invalid");
    DOBA_EXPECT(client.wait_for_close(std::chrono::seconds(3)));
    client.close();
  }
  DOBA_EXPECT_EQUAL(requests.load(), 0);
  DOBA_EXPECT_EQUAL(rejections.load(), 2);
  DOBA_EXPECT_EQUAL(rejection_code.load(), 0);
  server.stop();
}

// +===========================================================================+
// | [>] decoder rejection delivery                              ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip sends a rejection response then closes the client channel") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> requests = 0;
  std::atomic<int> rejection_code = 0;
  server.set_on_request(
      [&requests](const std::shared_ptr<transport_request>&,
                  transport_response&,
                  const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        requests.fetch_add(1);
        return std::nullopt;
      });
  server.set_on_bad_request(
      [&rejection_code](int code, std::string_view,
                        transport_response& response) {
        rejection_code.store(code);
        response.value = "invalid";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("E"));
  auto response = client.receive(7);
  DOBA_EXPECT(response.has_value());
  DOBA_EXPECT_EQUAL(*response, "invalid");
  DOBA_EXPECT(client.wait_for_close(std::chrono::seconds(3)));
  DOBA_EXPECT_EQUAL(requests.load(), 0);
  DOBA_EXPECT_EQUAL(rejection_code.load(), 42);
  server.stop();
}

// +===========================================================================+
// | [>] pipelined async response ordering                        ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip preserves response order for pipelined deferred requests") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto first_signal = std::make_shared<deferred_signal>();
  auto second_signal = std::make_shared<deferred_signal>();
  auto serialized = std::make_shared<std::atomic<std::size_t>>(0);
  std::atomic<std::size_t> deferred = 0;
  server.set_on_request(
      [&deferred, &first_signal, &second_signal, &serialized](
          const std::shared_ptr<transport_request>& request,
          transport_response&,
          const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        std::size_t index = deferred.fetch_add(1);
        return make_response(std::shared_ptr<const transport_request>(request),
                             index == 0 ? first_signal : second_signal,
                             serialized);
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("FL"));
  DOBA_EXPECT(first_signal->wait());
  DOBA_EXPECT(second_signal->wait());
  second_signal->resume();
  DOBA_EXPECT(wait_for_count(*serialized, 1));
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
  first_signal->resume();
  auto response = client.receive(11);
  DOBA_EXPECT(response.has_value());
  DOBA_EXPECT_EQUAL(*response, "firstsecond");
  client.close();
  server.stop();
}

// +===========================================================================+
// | [>] deferred response cancellation                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip cooperatively cancels deferred responses") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto client_signal = std::make_shared<deferred_signal>();
  auto stop_signal = std::make_shared<deferred_signal>();
  auto serialized = std::make_shared<std::atomic<std::size_t>>(0);
  auto completed = std::make_shared<std::atomic<std::size_t>>(0);
  std::optional<std::stop_callback<std::function<void()>>>
      client_cancellation;
  std::optional<std::stop_callback<std::function<void()>>>
      stop_cancellation;
  std::atomic<std::size_t> deferred = 0;
  std::atomic<std::size_t> cancelled = 0;
  std::atomic<std::size_t> disconnected = 0;
  server.set_on_request(
      [&client_signal, &stop_signal, &serialized, &completed,
       &client_cancellation, &stop_cancellation, &deferred, &cancelled](
          const std::shared_ptr<transport_request>&,
          transport_response&,
          const std::stop_token& stop_token)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        std::size_t index = deferred.fetch_add(1);
        std::shared_ptr<deferred_signal> signal =
            index == 0 ? client_signal : stop_signal;
        std::function<void()> callback = [signal, &cancelled]() {
          cancelled.fetch_add(1);
          signal->resume();
        };
        if (index == 0) {
          client_cancellation.emplace(stop_token, std::move(callback));
        } else {
          stop_cancellation.emplace(stop_token, std::move(callback));
        }
        return make_cancellable_response(signal, serialized, completed);
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection(
      [&disconnected]() { disconnected.fetch_add(1); });
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("D"));
  DOBA_EXPECT(client_signal->wait());
  client.close();
  DOBA_EXPECT(wait_for_count(disconnected, 1));
  DOBA_EXPECT(wait_for_count(cancelled, 1));
  DOBA_EXPECT(wait_for_count(*completed, 1));
  DOBA_EXPECT(remains_equal(*serialized, 0));

  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("D"));
  DOBA_EXPECT(stop_signal->wait());
  server.stop();
  DOBA_EXPECT(wait_for_count(disconnected, 2));
  DOBA_EXPECT(wait_for_count(cancelled, 2));
  DOBA_EXPECT(wait_for_count(*completed, 2));
  DOBA_EXPECT(remains_equal(*serialized, 0));
}

// +===========================================================================+
// | [>] mixed response ordering                                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip orders mixed synchronous and deferred responses") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto signals = std::make_shared<
      std::array<std::shared_ptr<deferred_signal>, 2>>();
  (*signals)[0] = std::make_shared<deferred_signal>();
  (*signals)[1] = std::make_shared<deferred_signal>();
  std::atomic<std::size_t> deferred = 0;
  server.set_on_request(
      [&signals, &deferred](
          const std::shared_ptr<transport_request>& request,
          transport_response& response,
          const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        if (request->value == 'S') {
          response.value = "sync";
          return std::nullopt;
        }
        transport_response result;
        result.value = "async";
        std::size_t index = deferred.fetch_add(1);
        return make_deferred_response((*signals)[index], std::move(result));
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("AS"));
  DOBA_EXPECT((*signals)[0]->wait());
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
  (*signals)[0]->resume();
  auto deferred_first = client.receive(9);
  DOBA_EXPECT(deferred_first.has_value());
  DOBA_EXPECT_EQUAL(*deferred_first, "asyncsync");
  client.close();

  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("SA"));
  auto synchronous_first = client.receive(4);
  DOBA_EXPECT(synchronous_first.has_value());
  DOBA_EXPECT_EQUAL(*synchronous_first, "sync");
  DOBA_EXPECT((*signals)[1]->wait());
  (*signals)[1]->resume();
  auto deferred_second = client.receive(5);
  DOBA_EXPECT(deferred_second.has_value());
  DOBA_EXPECT_EQUAL(*deferred_second, "async");
  client.close();
  server.stop();
}

// +===========================================================================+
// | [>] ordered interim response                                ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip keeps an interim behind an earlier deferred response") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, framed_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto signal = std::make_shared<deferred_signal>();
  server.set_on_request(
      [&signal](const std::shared_ptr<transport_request>& request,
                transport_response& response,
                const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        if (request->value == 'D') {
          transport_response result;
          result.value = request->payload;
          return make_deferred_response(signal, std::move(result));
        }
        response.value = request->payload;
        return std::nullopt;
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  std::string first = frame('D', "first");
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(first + "I"));
  DOBA_EXPECT(signal->wait());
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
  signal->resume();
  auto ordered = client.receive(12);
  DOBA_EXPECT(ordered.has_value());
  DOBA_EXPECT_EQUAL(*ordered, "firstinterim");
  std::string second = frame('I', "final");
  DOBA_EXPECT(client.send_all(std::string_view(second).substr(1)));
  auto final = client.receive(5);
  DOBA_EXPECT(final.has_value());
  DOBA_EXPECT_EQUAL(*final, "final");
  client.close();
  server.stop();
}

// +===========================================================================+
// | [>] synchronous channel close                              ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip drains a synchronous close response before eof") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> requests = 0;
  server.set_on_request(
      [&requests](const std::shared_ptr<transport_request>& request,
                  transport_response& response,
                  const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        requests.fetch_add(1);
        response.value = request->value == 'C' ? "close" : "unexpected";
        return std::nullopt;
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("CS"));
  auto response = client.receive(5);
  DOBA_EXPECT(response.has_value());
  DOBA_EXPECT_EQUAL(*response, "close");
  DOBA_EXPECT(client.wait_for_close(std::chrono::seconds(3)));
  DOBA_EXPECT_EQUAL(requests.load(), 1);
  server.stop();
}

// +===========================================================================+
// | [>] deferred channel close                                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip drains a deferred close response before eof") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto signal = std::make_shared<deferred_signal>();
  std::atomic<std::size_t> requests = 0;
  server.set_on_request(
      [&signal, &requests](
          const std::shared_ptr<transport_request>& request,
          transport_response&,
          const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        requests.fetch_add(1);
        transport_response response;
        response.value = request->value == 'C' ? "close" : "unexpected";
        return make_deferred_response(signal, std::move(response));
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("CS"));
  DOBA_EXPECT(signal->wait());
  signal->resume();
  auto response = client.receive(5);
  DOBA_EXPECT(response.has_value());
  DOBA_EXPECT_EQUAL(*response, "close");
  DOBA_EXPECT(client.wait_for_close(std::chrono::seconds(3)));
  DOBA_EXPECT_EQUAL(requests.load(), 1);
  server.stop();
}

// +===========================================================================+
// | [>] empty response delivery                                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip removes an empty response without blocking its queue") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server.set_on_request(
      [](const std::shared_ptr<transport_request>& request,
         transport_response& response,
         const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        if (request->value == 'S') response.value = "sync";
        return std::nullopt;
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("QS"));
  auto response = client.receive(4);
  DOBA_EXPECT(response.has_value());
  DOBA_EXPECT_EQUAL(*response, "sync");
  client.close();
  server.stop();
}

// +===========================================================================+
// | [>] streamed response boundaries                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip streams response sources across send boundaries") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  constexpr std::array<std::size_t, 8> sizes = {
      0, 8191, 8192, 8193, 65535, 65536, 65537, 131089};
  std::atomic<std::size_t> index = 0;
  server.set_on_request(
      [&index, &sizes](const std::shared_ptr<transport_request>&,
                       transport_response& response,
                       const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        std::size_t current = index.fetch_add(1);
        response.value = "p";
        response.source.assign(sizes[current],
                               static_cast<char>('a' + current));
        response.behavior = serialization_behavior::kSource;
        return std::nullopt;
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  DOBA_EXPECT(client.connect(port));
  for (std::size_t current = 0; current < sizes.size(); current++) {
    DOBA_EXPECT(client.send_all("B"));
    auto response = client.receive(sizes[current] + 1);
    DOBA_EXPECT(response.has_value());
    DOBA_EXPECT_EQUAL((*response)[0], 'p');
    DOBA_EXPECT_EQUAL(response->substr(1),
                      std::string(sizes[current],
                                  static_cast<char>('a' + current)));
  }
  client.close();
  server.stop();
}

// +===========================================================================+
// | [>] large prefix delivery                                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip sends prefixes larger than its bounded send buffer") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  constexpr std::size_t response_size = 131089;
  server.set_on_request(
      [](const std::shared_ptr<transport_request>&,
         transport_response& response,
         const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        response.value.assign(response_size, 'p');
        return std::nullopt;
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("B"));
  auto response = client.receive(response_size);
  DOBA_EXPECT(response.has_value());
  DOBA_EXPECT_EQUAL(*response, std::string(response_size, 'p'));
  client.close();
  server.stop();
}

// +===========================================================================+
// | [>] streamed pipeline delivery                              ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip completes a streamed response before its successor") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  constexpr std::size_t source_size = 70001;
  server.set_on_request(
      [](const std::shared_ptr<transport_request>& request,
         transport_response& response,
         const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        if (request->value == 'B') {
          response.value = "prefix";
          response.source.assign(source_size, 'b');
          response.behavior = serialization_behavior::kSource;
        } else {
          response.value = "tail";
        }
        return std::nullopt;
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("BS"));
  auto response = client.receive(source_size + 10);
  DOBA_EXPECT(response.has_value());
  DOBA_EXPECT_EQUAL(*response,
                    "prefix" + std::string(source_size, 'b') + "tail");
  client.close();
  server.stop();
}

// +===========================================================================+
// | [>] slow client isolation                                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip serves another client while a large response is blocked") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client slow_client;
  martianlabs::doba::tests::integration::tcpip_client fast_client;
  uint16_t port = slow_client.find_available_port();
  DOBA_EXPECT(port != 0);
  constexpr std::size_t source_size = 128 * 1024;
  auto serialized = std::make_shared<std::atomic<std::size_t>>(0);
  server.set_on_request(
      [&serialized](const std::shared_ptr<transport_request>& request,
                    transport_response& response,
                    const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        if (request->value == 'L') {
          response.source.assign(source_size, 'l');
          response.behavior = serialization_behavior::kSource;
          response.serialized = serialized;
        } else {
          response.value = "fast";
        }
        return std::nullopt;
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  DOBA_EXPECT(slow_client.connect(port));
  DOBA_EXPECT(slow_client.set_receive_buffer_size(4096));
  DOBA_EXPECT(slow_client.send_all("L"));
  DOBA_EXPECT(wait_for_count(*serialized, 1));
  DOBA_EXPECT(fast_client.connect(port));
  DOBA_EXPECT(fast_client.send_all("S"));
  auto fast_response = fast_client.receive(4);
  DOBA_EXPECT(fast_response.has_value());
  DOBA_EXPECT_EQUAL(*fast_response, "fast");
  auto slow_response = slow_client.receive(source_size);
  DOBA_EXPECT(slow_response.has_value());
  DOBA_EXPECT_EQUAL(*slow_response, std::string(source_size, 'l'));
  fast_client.close();
  slow_client.close();
  server.stop();
}

// +===========================================================================+
// | [>] reset during send recovery                             ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip recovers after a client resets a large response") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client reset_client;
  martianlabs::doba::tests::integration::tcpip_client next_client;
  uint16_t port = reset_client.find_available_port();
  DOBA_EXPECT(port != 0);
  constexpr std::size_t source_size = 4 * 1024 * 1024;
  auto serialized = std::make_shared<std::atomic<std::size_t>>(0);
  std::atomic<std::size_t> disconnected = 0;
  server.set_on_request(
      [&serialized](const std::shared_ptr<transport_request>& request,
                    transport_response& response,
                    const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        if (request->value == 'L') {
          response.source.assign(source_size, 'r');
          response.behavior = serialization_behavior::kSource;
          response.serialized = serialized;
        } else {
          response.value = "next";
        }
        return std::nullopt;
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection(
      [&disconnected]() { disconnected.fetch_add(1); });
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  DOBA_EXPECT(reset_client.connect(port));
  DOBA_EXPECT(reset_client.set_receive_buffer_size(4096));
  DOBA_EXPECT(reset_client.send_all("L"));
  DOBA_EXPECT(wait_for_count(*serialized, 1));
  reset_client.abort();
  DOBA_EXPECT(wait_for_count(disconnected, 1));
  DOBA_EXPECT(next_client.connect(port));
  DOBA_EXPECT(next_client.send_all("S"));
  auto response = next_client.receive(4);
  DOBA_EXPECT(response.has_value());
  DOBA_EXPECT_EQUAL(*response, "next");
  next_client.close();
  server.stop();
}

// +===========================================================================+
// | [>] synchronous handler failure                           ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip converts synchronous handler exceptions to errors") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> errors = 0;
  std::atomic<int> rejection_code = 0;
  server.set_on_request(
      [](const std::shared_ptr<transport_request>& request,
         transport_response& response,
         const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        if (request->value == 'A') {
          throw std::runtime_error("Handler error!");
        }
        if (request->value == 'B') throw 1;
        response.value = "next";
        return std::nullopt;
      });
  server.set_on_bad_request(
      [&errors, &rejection_code](int code, std::string_view,
                                 transport_response& response) {
        errors.fetch_add(1);
        rejection_code.store(code);
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  for (std::string_view request : {std::string_view("A"),
                                   std::string_view("B")}) {
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all(request));
    auto response = client.receive(5);
    DOBA_EXPECT(response.has_value());
    DOBA_EXPECT_EQUAL(*response, "error");
    DOBA_EXPECT(client.wait_for_close(std::chrono::seconds(3)));
    client.close();
  }
  DOBA_EXPECT_EQUAL(errors.load(), 2);
  DOBA_EXPECT_EQUAL(rejection_code.load(), 7);
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("S"));
  auto recovery = client.receive(4);
  DOBA_EXPECT(recovery.has_value());
  DOBA_EXPECT_EQUAL(*recovery, "next");
  client.close();
  server.stop();
}

// +===========================================================================+
// | [>] deferred handler failure                              ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip converts deferred handler exceptions to errors") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::array<std::shared_ptr<deferred_signal>, 2> signals = {
      std::make_shared<deferred_signal>(),
      std::make_shared<deferred_signal>()};
  std::atomic<std::size_t> deferred = 0;
  std::atomic<std::size_t> errors = 0;
  std::atomic<std::size_t> disconnected = 0;
  server.set_on_request(
      [&signals, &deferred](
          const std::shared_ptr<transport_request>&,
          transport_response&,
          const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        std::size_t index = deferred.fetch_add(1);
        return make_failed_response(signals[index], index == 0);
      });
  server.set_on_bad_request(
      [&errors](int code, std::string_view,
                transport_response& response) {
        if (code == 7) errors.fetch_add(1);
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection(
      [&disconnected]() { disconnected.fetch_add(1); });
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  for (std::size_t index = 0; index < signals.size(); index++) {
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all("A"));
    DOBA_EXPECT(signals[index]->wait());
    signals[index]->resume();
    auto response = client.receive(5);
    DOBA_EXPECT(response.has_value());
    DOBA_EXPECT_EQUAL(*response, "error");
    DOBA_EXPECT(client.wait_for_close(std::chrono::seconds(3)));
    DOBA_EXPECT(wait_for_count(disconnected, index + 1));
    client.close();
  }
  DOBA_EXPECT_EQUAL(errors.load(), 2);
  server.stop();
}

// +===========================================================================+
// | [>] ordered deferred failure                              ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip orders a deferred error before accepted successors") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto signal = std::make_shared<deferred_signal>();
  server.set_on_request(
      [&signal](const std::shared_ptr<transport_request>& request,
                transport_response& response,
                const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        if (request->value == 'A') {
          return make_failed_response(signal, true);
        }
        response.value = "sync";
        return std::nullopt;
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("AS"));
  DOBA_EXPECT(signal->wait());
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
  signal->resume();
  auto response = client.receive(9);
  DOBA_EXPECT(response.has_value());
  DOBA_EXPECT_EQUAL(*response, "errorsync");
  DOBA_EXPECT(client.wait_for_close(std::chrono::seconds(3)));
  server.stop();
}

// +===========================================================================+
// | [>] synchronous serialization failure                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip handles synchronous serialization failures") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> errors = 0;
  server.set_on_request(
      [](const std::shared_ptr<transport_request>& request,
         transport_response& response,
         const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        if (request->value == 'T') {
          response.behavior = serialization_behavior::kThrowStandard;
        } else if (request->value == 'U') {
          response.behavior = serialization_behavior::kThrowUnknown;
        } else if (request->value == 'R') {
          response.behavior = serialization_behavior::kNull;
        } else {
          response.value = "next";
        }
        return std::nullopt;
      });
  server.set_on_bad_request(
      [&errors](int code, std::string_view,
                transport_response& response) {
        if (code == 7) errors.fetch_add(1);
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  for (std::string_view request : {std::string_view("T"),
                                   std::string_view("U")}) {
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all(request));
    auto response = client.receive(5);
    DOBA_EXPECT(response.has_value());
    DOBA_EXPECT_EQUAL(*response, "error");
    DOBA_EXPECT(client.wait_for_close(std::chrono::seconds(3)));
    client.close();
  }
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("R"));
  auto empty = client.receive_until_close(1);
  DOBA_EXPECT(empty.has_value());
  DOBA_EXPECT(empty->empty());
  DOBA_EXPECT_EQUAL(errors.load(), 2);
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("S"));
  auto recovery = client.receive(4);
  DOBA_EXPECT(recovery.has_value());
  DOBA_EXPECT_EQUAL(*recovery, "next");
  client.close();
  server.stop();
}

// +===========================================================================+
// | [>] deferred serialization failure                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip handles deferred serialization failures") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::array<std::shared_ptr<deferred_signal>, 3> signals = {
      std::make_shared<deferred_signal>(),
      std::make_shared<deferred_signal>(),
      std::make_shared<deferred_signal>()};
  std::atomic<std::size_t> deferred = 0;
  std::atomic<std::size_t> errors = 0;
  server.set_on_request(
      [&signals, &deferred](
          const std::shared_ptr<transport_request>&,
          transport_response&,
          const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        std::size_t index = deferred.fetch_add(1);
        transport_response response;
        if (index == 0) {
          response.behavior = serialization_behavior::kThrowStandard;
        } else if (index == 1) {
          response.behavior = serialization_behavior::kThrowUnknown;
        } else {
          response.behavior = serialization_behavior::kNull;
        }
        return make_deferred_response(signals[index], std::move(response));
      });
  server.set_on_bad_request(
      [&errors](int code, std::string_view,
                transport_response& response) {
        if (code == 7) errors.fetch_add(1);
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  for (std::size_t index = 0; index < signals.size(); index++) {
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all("A"));
    DOBA_EXPECT(signals[index]->wait());
    signals[index]->resume();
    if (index < 2) {
      auto response = client.receive(5);
      DOBA_EXPECT(response.has_value());
      DOBA_EXPECT_EQUAL(*response, "error");
      DOBA_EXPECT(client.wait_for_close(std::chrono::seconds(3)));
    } else {
      auto response = client.receive_until_close(1);
      DOBA_EXPECT(response.has_value());
      DOBA_EXPECT(response->empty());
    }
    client.close();
  }
  DOBA_EXPECT_EQUAL(errors.load(), 2);
  server.stop();
}

// +===========================================================================+
// | [>] error generation failure                                ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip recovers when error response generation fails") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> errors = 0;
  server.set_on_request(
      [](const std::shared_ptr<transport_request>&,
         transport_response& response,
         const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        response.value = "next";
        return std::nullopt;
      });
  server.set_on_bad_request(
      [&errors](int, std::string_view, transport_response& response) {
        std::size_t error = errors.fetch_add(1);
        if (error == 0) throw std::runtime_error("Error callback failed!");
        if (error == 1) {
          response.behavior = serialization_behavior::kThrowStandard;
          return;
        }
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  for (std::size_t index = 0; index < 2; index++) {
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all("E"));
    auto response = client.receive_until_close(1);
    DOBA_EXPECT(response.has_value());
    DOBA_EXPECT(response->empty());
    client.close();
  }
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("E"));
  auto error = client.receive(5);
  DOBA_EXPECT(error.has_value());
  DOBA_EXPECT_EQUAL(*error, "error");
  DOBA_EXPECT(client.wait_for_close(std::chrono::seconds(3)));
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("S"));
  auto recovery = client.receive(4);
  DOBA_EXPECT(recovery.has_value());
  DOBA_EXPECT_EQUAL(*recovery, "next");
  client.close();
  server.stop();
}

// +===========================================================================+
// | [>] completed request half close                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip drains a response after the client half closes") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> disconnected = 0;
  server.set_on_request(
      [](const std::shared_ptr<transport_request>&,
         transport_response& response,
         const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        response.value = "sync";
        return std::nullopt;
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection(
      [&disconnected]() { disconnected.fetch_add(1); });
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("S"));
  DOBA_EXPECT(client.shutdown_write());
  auto response = client.receive(4);
  DOBA_EXPECT(response.has_value());
  DOBA_EXPECT_EQUAL(*response, "sync");
  DOBA_EXPECT(client.wait_for_close(std::chrono::seconds(3)));
  DOBA_EXPECT(wait_for_count(disconnected, 1));
  server.stop();
}

// +===========================================================================+
// | [>] incomplete request half close                           ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip closes silently after a partial request eof") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, framed_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> requests = 0;
  std::atomic<std::size_t> errors = 0;
  server.set_on_request(
      [&requests](const std::shared_ptr<transport_request>&,
                  transport_response&,
                  const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        requests.fetch_add(1);
        return std::nullopt;
      });
  server.set_on_bad_request(
      [&errors](int, std::string_view, transport_response& response) {
        errors.fetch_add(1);
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("P"));
  DOBA_EXPECT(client.shutdown_write());
  auto response = client.receive_until_close(1);
  DOBA_EXPECT(response.has_value());
  DOBA_EXPECT(response->empty());
  DOBA_EXPECT_EQUAL(requests.load(), 0);
  DOBA_EXPECT_EQUAL(errors.load(), 0);
  server.stop();
}

// +===========================================================================+
// | [>] deferred request half close                             ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip cancels a deferred response after input eof") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto signal = std::make_shared<deferred_signal>();
  auto serialized = std::make_shared<std::atomic<std::size_t>>(0);
  std::atomic<std::size_t> disconnected = 0;
  server.set_on_request(
      [&signal, &serialized](
          const std::shared_ptr<transport_request>&,
          transport_response&,
          const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        transport_response response;
        response.value = "deferred";
        response.serialized = serialized;
        return make_deferred_response(signal, std::move(response));
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection(
      [&disconnected]() { disconnected.fetch_add(1); });
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("D"));
  DOBA_EXPECT(signal->wait());
  DOBA_EXPECT(client.shutdown_write());
  DOBA_EXPECT(client.wait_for_close(std::chrono::seconds(3)));
  DOBA_EXPECT(wait_for_count(disconnected, 1));
  signal->resume();
  DOBA_EXPECT(remains_equal(*serialized, 0));
  server.stop();
}

// +===========================================================================+
// | [>] reset during fragmented receive                         ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip recovers after a reset during request reception") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, framed_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client reset_client;
  martianlabs::doba::tests::integration::tcpip_client next_client;
  uint16_t port = reset_client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> disconnected = 0;
  server.set_on_request(
      [](const std::shared_ptr<transport_request>& request,
         transport_response& response,
         const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        response.value = request->payload;
        return std::nullopt;
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection(
      [&disconnected]() { disconnected.fetch_add(1); });
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  DOBA_EXPECT(reset_client.connect(port));
  DOBA_EXPECT(reset_client.send_all("P"));
  reset_client.abort();
  DOBA_EXPECT(wait_for_count(disconnected, 1));
  DOBA_EXPECT(next_client.connect(port));
  DOBA_EXPECT(next_client.send_all(frame('P', "next")));
  auto response = next_client.receive(4);
  DOBA_EXPECT(response.has_value());
  DOBA_EXPECT_EQUAL(*response, "next");
  next_client.close();
  server.stop();
}

// +===========================================================================+
// | [>] concurrent fragmented clients                           ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip isolates interleaved requests from concurrent clients") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, framed_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client port_client;
  uint16_t port = port_client.find_available_port();
  DOBA_EXPECT(port != 0);
  constexpr std::size_t client_count = 8;
  std::atomic<std::size_t> connected = 0;
  std::atomic<std::size_t> disconnected = 0;
  server.set_on_request(
      [](const std::shared_ptr<transport_request>& request,
         transport_response& response,
         const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        response.value = request->payload;
        return std::nullopt;
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([&connected]() { connected.fetch_add(1); });
  server.set_on_disconnection(
      [&disconnected]() { disconnected.fetch_add(1); });
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  std::barrier synchronization(static_cast<std::ptrdiff_t>(client_count));
  std::array<bool, client_count> results{};
  std::array<std::thread, client_count> clients;
  for (std::size_t index = 0; index < client_count; index++) {
    clients[index] = std::thread([&, index]() {
      martianlabs::doba::tests::integration::tcpip_client client;
      std::string payload(16, static_cast<char>('a' + index));
      std::string request = frame('P', payload);
      bool connected_client = client.connect(port);
      bool first_sent =
          connected_client &&
          client.send_all(std::string_view(request).substr(0, 1));
      synchronization.arrive_and_wait();
      bool rest_sent =
          first_sent &&
          client.send_all(std::string_view(request).substr(1));
      auto response = rest_sent ? client.receive(payload.size())
                                : std::optional<std::string>();
      results[index] = response.has_value() && *response == payload;
      client.close();
    });
  }
  for (auto& client : clients) client.join();
  for (bool result : results) DOBA_EXPECT(result);
  DOBA_EXPECT(wait_for_count(connected, client_count));
  DOBA_EXPECT(wait_for_count(disconnected, client_count));
  server.stop();
}

// +===========================================================================+
// | [>] mixed state server stop                                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip stops idle blocked and deferred clients exactly once") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client idle_client;
  martianlabs::doba::tests::integration::tcpip_client slow_client;
  martianlabs::doba::tests::integration::tcpip_client deferred_client;
  uint16_t port = idle_client.find_available_port();
  DOBA_EXPECT(port != 0);
  constexpr std::size_t source_size = 4 * 1024 * 1024;
  auto signal = std::make_shared<deferred_signal>();
  auto serialized = std::make_shared<std::atomic<std::size_t>>(0);
  std::atomic<std::size_t> connected = 0;
  std::atomic<std::size_t> disconnected = 0;
  server.set_on_request(
      [&signal, &serialized](
          const std::shared_ptr<transport_request>& request,
          transport_response& response,
          const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        if (request->value == 'D') {
          transport_response deferred;
          deferred.value = "deferred";
          deferred.serialized = serialized;
          return make_deferred_response(signal, std::move(deferred));
        }
        response.source.assign(source_size, 's');
        response.behavior = serialization_behavior::kSource;
        return std::nullopt;
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([&connected]() { connected.fetch_add(1); });
  server.set_on_disconnection(
      [&disconnected]() { disconnected.fetch_add(1); });
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  DOBA_EXPECT(idle_client.connect(port));
  DOBA_EXPECT(slow_client.connect(port));
  DOBA_EXPECT(slow_client.set_receive_buffer_size(4096));
  DOBA_EXPECT(slow_client.send_all("L"));
  DOBA_EXPECT(deferred_client.connect(port));
  DOBA_EXPECT(deferred_client.send_all("D"));
  DOBA_EXPECT(signal->wait());
  DOBA_EXPECT(wait_for_count(connected, 3));
  server.stop();
  DOBA_EXPECT(wait_for_count(disconnected, 3));
  signal->resume();
  DOBA_EXPECT(remains_equal(*serialized, 0));
}

// +===========================================================================+
// | [>] server restart                                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip restarts the same server on the same port") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> requests = 0;
  server.set_on_request(
      [&requests](const std::shared_ptr<transport_request>&,
                  transport_response& response,
                  const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        requests.fetch_add(1);
        response.value = "sync";
        return std::nullopt;
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  std::string port_text = std::to_string(port);
  constexpr std::size_t iteration_count = 16;

  for (std::size_t iteration = 0; iteration < iteration_count; iteration++) {
    server.start(port_text.c_str());
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all("S"));
    auto response = client.receive(4);
    DOBA_EXPECT(response.has_value());
    DOBA_EXPECT_EQUAL(*response, "sync");
    client.close();
    server.stop();
  }
  DOBA_EXPECT_EQUAL(requests.load(), iteration_count);
}

// +===========================================================================+
// | [>] connection callback failure                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip survives a failing connection callback") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> connections = 0;
  server.set_on_request(
      [](const std::shared_ptr<transport_request>&,
         transport_response& response,
         const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        response.value = "sync";
        return std::nullopt;
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([&connections]() {
    if (connections.fetch_add(1) == 0) {
      throw std::runtime_error("Connection callback error!");
    }
  });
  server.set_on_disconnection([]() {});
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  DOBA_EXPECT(client.connect(port));
  auto rejected = client.receive_until_close(1);
  DOBA_EXPECT(rejected.has_value());
  DOBA_EXPECT(rejected->empty());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("S"));
  auto response = client.receive(4);
  DOBA_EXPECT(response.has_value());
  DOBA_EXPECT_EQUAL(*response, "sync");
  client.close();
  server.stop();
}

// +===========================================================================+
// | [>] disconnection callback failure                         ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip survives failing disconnection callbacks") {
  martianlabs::doba::transport::server::tcpip<
      transport_request, transport_response, transport_decoder>
      server;
  martianlabs::doba::tests::integration::tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> disconnected = 0;
  server.set_on_request(
      [](const std::shared_ptr<transport_request>&,
         transport_response& response,
         const std::stop_token&)
          -> std::optional<
              martianlabs::doba::common::task<transport_response>> {
        response.value = "sync";
        return std::nullopt;
      });
  server.set_on_bad_request(
      [](int, std::string_view, transport_response& response) {
        response.value = "error";
      });
  server.set_on_connection([]() {});
  server.set_on_disconnection([&disconnected]() {
    disconnected.fetch_add(1);
    throw std::runtime_error("Disconnection callback error!");
  });
  std::string port_text = std::to_string(port);
  server.start(port_text.c_str());

  for (std::size_t iteration = 0; iteration < 2; iteration++) {
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all("S"));
    auto response = client.receive(4);
    DOBA_EXPECT(response.has_value());
    DOBA_EXPECT_EQUAL(*response, "sync");
    client.close();
    DOBA_EXPECT(wait_for_count(disconnected, iteration + 1));
  }
  server.stop();
}
