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

#ifndef martianlabs_doba_common_date_server_h
#define martianlabs_doba_common_date_server_h

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <mutex>
#include <string_view>
#include <thread>

#include "date_server_helpers.h"

namespace martianlabs::doba::common {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] date_server                                                 ( class ) |
// +---------------------------------------------------------------------------+
// | This specification holds for a [cross-platform] date server.              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class date_server {
  // +=========================================================================+
  // | [>] CONSTRUCTORs                                            ( private ) |
  // +=========================================================================+
#ifdef _MSC_VER
  __declspec(noinline)
#else
  __attribute__((noinline))
#endif
  date_server() { std::jthread([this] { update(); }).join(); }

 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  date_server(const date_server&) = delete;
  date_server(date_server&&) noexcept = delete;
  ~date_server() {
    std::lock_guard<std::mutex> lock(lifecycle_mutex_);
    owners_ = 0;
    jthread_.request_stop();
    wake_.notify_all();
    if (jthread_.joinable()) jthread_.join();
  }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  date_server& operator=(const date_server&) = delete;
  date_server& operator=(date_server&&) noexcept = delete;
  // +=========================================================================+
  // | [>] get                                                      ( public ) |
  // +=========================================================================+
  static date_server& get() {
    static date_server instance;
    return instance;
  }
  // +=========================================================================+
  // | [>] start                                                    ( public ) |
  // +=========================================================================+
  void start() {
    std::lock_guard<std::mutex> lock(lifecycle_mutex_);
    if (owners_++ > 0) return;
    std::unique_lock<std::mutex> wait(wait_mutex_);
    ready_ = false;
    try {
      jthread_ = std::jthread([this](std::stop_token stop) {
        std::unique_lock<std::mutex> lock(wait_mutex_);
        update();
        ready_ = true;
        wake_.notify_all();
        while (!stop.stop_requested()) {
          wake_.wait_for(lock, stop, std::chrono::seconds(1), [] {
            return false;
          });
          if (!stop.stop_requested()) update();
        }
      });
    } catch (...) {
      owners_ = 0;
      throw;
    }
    wake_.wait(wait, [this] { return ready_; });
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +=========================================================================+
  void stop() {
    std::lock_guard<std::mutex> lock(lifecycle_mutex_);
    if (owners_ == 0 || --owners_ > 0) return;
    jthread_.request_stop();
    wake_.notify_all();
    if (jthread_.joinable()) jthread_.join();
  }
  // +=========================================================================+
  // | [>] current                                                  ( public ) |
  // +=========================================================================+
  std::string_view current() const noexcept {
    const std::uint64_t current = generation_.load(std::memory_order_seq_cst);
    thread_local std::uint64_t previous = 0;
    thread_local char buffer[kBufSize]{};
    if (previous != current) {
      previous = refresh(buffer, current);
    }
    return {buffer, kDateLen};
  }

 private:
  // +=========================================================================+
  // | [>] CONSTANTs                                               ( private ) |
  // +=========================================================================+
  static_assert(std::atomic<std::uint64_t>::is_always_lock_free);
  static constexpr std::size_t kDateLen = 29;
  static constexpr std::size_t kBufSize = kDateLen + 1;
  static constexpr const char* kWeekDays[] = {"Sun", "Mon", "Tue", "Wed",
                                              "Thu", "Fri", "Sat"};
  static constexpr const char* kMonths[] = {"Jan", "Feb", "Mar", "Apr",
                                            "May", "Jun", "Jul", "Aug",
                                            "Sep", "Oct", "Nov", "Dec"};
  // +=========================================================================+
  // | [>] refresh                                                 ( private ) |
  // +=========================================================================+
#ifdef _MSC_VER
  __declspec(noinline)
#else
  __attribute__((noinline))
#endif
  std::uint64_t refresh(char* out, std::uint64_t current) const noexcept {
    std::uint64_t words[4];
    // Atomic words allow a retry even if the buffer is reused.
    for (;;) {
      for (std::size_t i = 0; i < 4; ++i) {
        words[i] = buffers_[current & 1][i].load(std::memory_order_seq_cst);
      }
      const std::uint64_t next = generation_.load(std::memory_order_seq_cst);
      if (next == current) break;
      current = next;
    }
    std::memcpy(out, words, kBufSize);
    return current;
  }
  // +=========================================================================+
  // | [>] two_digits                                              ( private ) |
  // +=========================================================================+
  static void two_digits(char* out, int value) noexcept {
    out[0] = static_cast<char>('0' + value / 10);
    out[1] = static_cast<char>('0' + value % 10);
  }
  // +=========================================================================+
  // | [>] four_digits                                             ( private ) |
  // +=========================================================================+
  static void four_digits(char* out, int value) noexcept {
    out[0] = static_cast<char>('0' + value / 1000 % 10);
    out[1] = static_cast<char>('0' + value / 100 % 10);
    out[2] = static_cast<char>('0' + value / 10 % 10);
    out[3] = static_cast<char>('0' + value % 10);
  }
  // +=========================================================================+
  // | [>] write_date                                              ( private ) |
  // +=========================================================================+
  static void write_date(char* out, std::time_t now) noexcept {
    std::tm gmt{};
    gm_time(&gmt, &now);
    std::memcpy(out + 0, kWeekDays[gmt.tm_wday], 3);
    out[3] = ',';
    out[4] = ' ';
    two_digits(out + 5, gmt.tm_mday);
    out[7] = ' ';
    std::memcpy(out + 8, kMonths[gmt.tm_mon], 3);
    out[11] = ' ';
    four_digits(out + 12, 1900 + gmt.tm_year);
    out[16] = ' ';
    two_digits(out + 17, gmt.tm_hour);
    out[19] = ':';
    two_digits(out + 20, gmt.tm_min);
    out[22] = ':';
    two_digits(out + 23, gmt.tm_sec);
    out[25] = ' ';
    std::memcpy(out + 26, "GMT", 3);
    out[29] = '\0';
  }
  // +=========================================================================+
  // | [>] update                                                  ( private ) |
  // +=========================================================================+
  void update() noexcept {
    char buffer[sizeof(std::uint64_t) * 4]{};
    write_date(buffer, std::time(nullptr));
    std::uint64_t words[4];
    std::memcpy(words, buffer, sizeof(words));
    const std::uint64_t next = generation_.load(std::memory_order_seq_cst) + 1;
    for (std::size_t i = 0; i < 4; ++i) {
      buffers_[next & 1][i].store(words[i], std::memory_order_seq_cst);
    }
    generation_.store(next, std::memory_order_seq_cst);
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  std::atomic<std::uint64_t> generation_{0};
  std::atomic<std::uint64_t> buffers_[2][4]{};
  std::condition_variable_any wake_;
  std::mutex lifecycle_mutex_;
  std::mutex wait_mutex_;
  std::size_t owners_{0};
  bool ready_{false};
  std::jthread jthread_;
};
}  // namespace martianlabs::doba::common

#endif
