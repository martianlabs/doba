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

#ifndef martianlabs_doba_tests_integration_http_test_helper_h
#define martianlabs_doba_tests_integration_http_test_helper_h

#include <atomic>
#include <charconv>
#include <chrono>
#include <condition_variable>
#include <coroutine>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include "tcpip_client.h"

namespace martianlabs::doba::tests::integration {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] http_test_response                                         ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct http_test_response {
  std::string status;
  std::vector<std::pair<std::string, std::string>> headers;
  std::string body;

  std::optional<std::string_view> header(std::string_view name) const {
    for (const auto& field : headers) {
      if (field.first == name) return field.second;
    }
    return std::nullopt;
  }
};

// +===========================================================================+
// | [>] valid_http_date                                            ( method ) |
// +===========================================================================+
inline bool valid_http_date(std::string_view value) {
  if (value.size() != 29 || value[3] != ',' || value[4] != ' ' ||
      value[7] != ' ' || value[11] != ' ' || value[16] != ' ' ||
      value[19] != ':' || value[22] != ':' || value[25] != ' ' ||
      value.substr(26) != "GMT") return false;
  constexpr std::string_view days[] = {
      "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  constexpr std::string_view months[] = {
      "Jan", "Feb", "Mar", "Apr", "May", "Jun",
      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  constexpr std::size_t digits[] = {
      5, 6, 12, 13, 14, 15, 17, 18, 20, 21, 23, 24};
  for (std::size_t position : digits) {
    if (value[position] < '0' || value[position] > '9') return false;
  }
  const auto number = [&](std::size_t position) {
    return (value[position] - '0') * 10 + value[position + 1] - '0';
  };
  const int year = number(12) * 100 + number(14);
  unsigned int month = 0;
  for (unsigned int i = 0; i < 12; i++) {
    if (value.substr(8, 3) == months[i]) month = i + 1;
  }
  const std::chrono::year_month_day date{
      std::chrono::year(year), std::chrono::month(month),
      std::chrono::day(static_cast<unsigned int>(number(5)))};
  if (!date.ok() || number(17) > 23 || number(20) > 59 ||
      number(23) > 60) return false;
  const std::chrono::weekday weekday{std::chrono::sys_days(date)};
  return value.substr(0, 3) == days[weekday.c_encoding()];
}

// +===========================================================================+
// | [>] receive_http_response                                      ( method ) |
// +===========================================================================+
inline std::optional<http_test_response> receive_http_response(
    tcpip_client& client, bool head = false) {
  const auto deadline = std::chrono::steady_clock::now() +
                        std::chrono::seconds(3);
  std::string wire;
  while (!wire.ends_with("\r\n\r\n")) {
    if (wire.size() >= 16384) return std::nullopt;
    const auto remaining =
        std::chrono::duration_cast<std::chrono::milliseconds>(
        deadline - std::chrono::steady_clock::now());
    if (remaining.count() <= 0) return std::nullopt;
    const auto byte = client.receive(1, remaining);
    if (!byte.has_value()) return std::nullopt;
    wire += *byte;
  }
  http_test_response result;
  std::size_t position = wire.find("\r\n");
  result.status = wire.substr(0, position);
  if (!result.status.starts_with("HTTP/1.1 ") || result.status.size() < 13 ||
      result.status[12] != ' ') return std::nullopt;
  int status = 0;
  const auto parsed_status = std::from_chars(
      result.status.data() + 9, result.status.data() + 12, status);
  if (parsed_status.ec != std::errc{} ||
      parsed_status.ptr != result.status.data() + 12) return std::nullopt;
  position += 2;
  while (position + 2 < wire.size()) {
    const auto end = wire.find("\r\n", position);
    const auto colon = wire.find(':', position);
    if (end == std::string::npos || colon == std::string::npos ||
        colon == position || colon >= end) return std::nullopt;
    std::size_t begin = colon + 1;
    while (begin < end && (wire[begin] == ' ' || wire[begin] == '\t')) begin++;
    const std::string name = wire.substr(position, colon - position);
    if ((name == "Content-Length" || name == "Transfer-Encoding" ||
         name == "Date") && result.header(name).has_value()) {
      return std::nullopt;
    }
    result.headers.emplace_back(name, wire.substr(begin, end - begin));
    position = end + 2;
  }
  if (status >= 200) {
    const auto date = result.header("Date");
    if (!date.has_value() || !valid_http_date(*date)) return std::nullopt;
  }
  const auto length = result.header("Content-Length");
  if (result.header("Transfer-Encoding").has_value()) return std::nullopt;
  std::size_t size = 0;
  if (length.has_value()) {
    const auto parsed = std::from_chars(
        length->data(), length->data() + length->size(), size);
    if (parsed.ec != std::errc{} ||
        parsed.ptr != length->data() + length->size()) {
      return std::nullopt;
    }
  }
  if ((status >= 100 && status < 200) || status == 204) {
    if (length.has_value()) return std::nullopt;
    size = 0;
  } else if (head || status == 304) {
    size = 0;
  } else if (!length.has_value()) {
    return std::nullopt;
  }
  if (size > 1024 * 1024) return std::nullopt;
  const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
      deadline - std::chrono::steady_clock::now());
  if (remaining.count() <= 0) return std::nullopt;
  const auto body = client.receive(size, remaining);
  if (!body.has_value()) return std::nullopt;
  result.body = *body;
  return result;
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] http_test_signal                                            ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class http_test_signal {
 public:
  bool await_ready() const noexcept { return false; }
  bool await_suspend(std::coroutine_handle<> continuation) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (released_) return false;
    continuation_ = continuation;
    condition_.notify_all();
    return true;
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
      released_ = true;
      continuation = std::exchange(continuation_, nullptr);
    }
    if (continuation) continuation.resume();
  }

 private:
  std::mutex mutex_;
  std::condition_variable condition_;
  std::coroutine_handle<> continuation_;
  bool released_ = false;
};

// +===========================================================================+
// | [>] wait_for_http_count                                        ( method ) |
// +===========================================================================+
inline bool wait_for_http_count(const std::atomic<std::size_t>& value,
                                std::size_t expected) {
  const auto deadline = std::chrono::steady_clock::now() +
                        std::chrono::seconds(3);
  while (value.load() < expected &&
         std::chrono::steady_clock::now() < deadline) {
    std::this_thread::yield();
  }
  return value.load() == expected;
}
}  // namespace martianlabs::doba::tests::integration

#endif
