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
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "common/date_server.h"
#include "protocol/http/v11/response.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::date_server;
using martianlabs::doba::protocol::http::v11::response;

class date_owner {
 public:
  date_owner() { date_server::get().start(); }
  date_owner(const date_owner&) = delete;
  date_owner& operator=(const date_owner&) = delete;
  ~date_owner() { stop(); }
  void stop() {
    if (!active_) return;
    date_server::get().stop();
    active_ = false;
  }

 private:
  bool active_ = true;
};

bool valid_http_date(std::string_view value) {
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
}  // namespace

// +===========================================================================+
// | [>] concurrent serialization preserves HTTP dates           ( test-case ) |
// +===========================================================================+
DOBA_TEST("concurrent serialization preserves HTTP dates") {
  date_owner owner;
  std::atomic<bool> valid{true};
  std::array<std::size_t, 4> operations{};
  std::barrier start(5);
  std::chrono::steady_clock::time_point end;
  std::vector<std::jthread> threads;
  for (std::size_t thread = 0; thread < operations.size(); ++thread) {
    threads.emplace_back([&, thread] {
      start.arrive_and_wait();
      while (std::chrono::steady_clock::now() < end) {
        response value;
        auto serialized = value.serialize();
        const std::size_t begin = serialized->prefix.find("Date: ");
        operations[thread]++;
        if (begin == std::string::npos ||
            !valid_http_date(serialized->prefix.substr(begin + 6, 29))) {
          valid.store(false);
          return;
        }
      }
    });
  }
  end = std::chrono::steady_clock::now() + std::chrono::milliseconds(1100);
  start.arrive_and_wait();
  for (auto& thread : threads) thread.join();
  owner.stop();
  DOBA_EXPECT(valid.load());
  for (std::size_t count : operations) DOBA_EXPECT(count > 0);
  DOBA_EXPECT(valid_http_date(date_server::get().current()));
}
// +===========================================================================+
// | [>] date server waits for last owner                        ( test-case ) |
// +===========================================================================+
DOBA_TEST("date server remains active until its last owner stops") {
  auto& value = date_server::get();
  date_owner first;
  date_owner last;
  first.stop();
  const std::string initial(value.current());
  DOBA_EXPECT(valid_http_date(initial));
  const auto deadline = std::chrono::steady_clock::now() +
                        std::chrono::milliseconds(2200);
  bool updated = false;
  while (std::chrono::steady_clock::now() < deadline) {
    if (value.current() != initial) {
      updated = true;
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  const std::string current(value.current());
  last.stop();
  DOBA_EXPECT(valid_http_date(current));
  DOBA_EXPECT(updated);
}
// +===========================================================================+
// | [>] concurrent owners preserve lifecycle                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("concurrent date server owners preserve lifecycle and dates") {
  auto& value = date_server::get();
  date_owner owner;
  std::atomic<bool> valid{true};
  std::array<std::size_t, 4> operations{};
  std::vector<std::jthread> threads;
  for (std::size_t thread = 0; thread < 4; ++thread) {
    threads.emplace_back([&value, &valid, &operations, thread] {
      for (std::size_t iteration = 0; iteration < 100; ++iteration) {
        date_owner worker;
        if (!valid_http_date(value.current())) valid.store(false);
        operations[thread]++;
      }
    });
  }
  for (auto& thread : threads) thread.join();
  owner.stop();
  for (std::size_t count : operations) DOBA_EXPECT_EQUAL(count, 100);
  DOBA_EXPECT(valid.load());
  DOBA_EXPECT(valid_http_date(value.current()));

}

// +===========================================================================+
// | [>] date server restarts from zero owners                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("date server restarts after the last owner stops") {
  auto& value = date_server::get();
  for (std::size_t cycle = 0; cycle < 3; cycle++) {
    date_owner owner;
    const std::string initial(value.current());
    DOBA_EXPECT(valid_http_date(initial));
    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::milliseconds(2200);
    std::string current = initial;
    while (current == initial && std::chrono::steady_clock::now() < deadline) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      current = value.current();
    }
    const auto stopping = std::chrono::steady_clock::now();
    owner.stop();
    DOBA_EXPECT(std::chrono::steady_clock::now() - stopping <
                std::chrono::seconds(2));
    DOBA_EXPECT(valid_http_date(current));
    DOBA_EXPECT(current != initial);
  }
}

// +===========================================================================+
// | [>] concurrent last stop and first start                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("date server handles concurrent last stop and first start") {
  auto& value = date_server::get();
  for (std::size_t cycle = 0; cycle < 3; cycle++) {
    date_owner previous;
    std::barrier start(3);
    std::atomic<bool> stopped = false;
    bool valid = true;
    bool updated = false;
    std::size_t operations = 0;
    std::jthread stopping([&]() {
      start.arrive_and_wait();
      previous.stop();
      stopped = true;
    });
    std::jthread starting([&]() {
      start.arrive_and_wait();
      date_owner next;
      const auto deadline = std::chrono::steady_clock::now() +
                            std::chrono::milliseconds(2200);
      while (!stopped.load() && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::yield();
      }
      valid = stopped.load();
      const std::string initial(value.current());
      valid = valid && valid_http_date(initial);
      while (std::chrono::steady_clock::now() < deadline) {
        const std::string current(value.current());
        operations++;
        if (!valid_http_date(current)) valid = false;
        if (current != initial) {
          updated = true;
          break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    });
    start.arrive_and_wait();
    stopping.join();
    starting.join();
    DOBA_EXPECT(stopped.load());
    DOBA_EXPECT(valid);
    DOBA_EXPECT(updated);
    DOBA_EXPECT(operations > 0);
  }
}
