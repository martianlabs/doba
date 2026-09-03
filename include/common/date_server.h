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

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <thread>

#include "date_server_helpers.h"

namespace martianlabs::doba::protocol::http::v11 {
class response;
}

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
  date_server() { update(); }

 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  date_server(const date_server&) = delete;
  date_server(date_server&&) noexcept = delete;
  ~date_server() { stop(); }
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
    bool expected = false;
    if (!running_.compare_exchange_strong(expected, true,
                                          std::memory_order_acq_rel,
                                          std::memory_order_acquire)) {
      // Already running, do nothing.
      return;
    }
    jthread_ = std::jthread([this] {
      while (running_.load(std::memory_order_acquire)) {
        update();
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    });
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +=========================================================================+
  void stop() {
    if (!running_.exchange(false, std::memory_order_acq_rel)) {
      // Not running, do nothing.
      return;
    }
    if (jthread_.joinable()) jthread_.join();
  }
  // +=========================================================================+
  // | [>] current                                                  ( public ) |
  // +=========================================================================+
  std::string_view current() const noexcept {
    thread_local char buffer[kDateLen];
    copy_current(buffer);
    return {buffer, kDateLen};
  }

 private:
  friend class martianlabs::doba::protocol::http::v11::response;
  // +=========================================================================+
  // | [>] copy_current                                            ( private ) |
  // +=========================================================================+
  void copy_current(char* out) const noexcept {
    std::uint64_t current[kWordCount];
    for (;;) {
      std::uint64_t sequence = sequence_.load(std::memory_order_seq_cst);
      // Odd sequence values mark an update in progress.
      if (sequence & 1) continue;
      for (std::size_t index = 0; index < kWordCount; ++index) {
        current[index] = words_[index].load(std::memory_order_seq_cst);
      }
      if (sequence_.load(std::memory_order_seq_cst) == sequence) break;
    }
    std::memcpy(out, current, kDateLen);
  }
  // +=========================================================================+
  // | [>] CONSTANTs                                               ( private ) |
  // +=========================================================================+
  static constexpr std::size_t kDateLen = 29;
  static constexpr std::size_t kBufSize = kDateLen + 1;
  static constexpr std::size_t kWordCount = 4;
  static constexpr std::size_t kStorageSize =
      kWordCount * sizeof(std::uint64_t);
  static constexpr const char* kWeekDays[] = {"Sun", "Mon", "Tue", "Wed",
                                              "Thu", "Fri", "Sat"};
  static constexpr const char* kMonths[] = {"Jan", "Feb", "Mar", "Apr",
                                            "May", "Jun", "Jul", "Aug",
                                            "Sep", "Oct", "Nov", "Dec"};
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
  static void write_date(char* out) noexcept {
    std::time_t now = std::time(nullptr);
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
    alignas(std::uint64_t) char text[kStorageSize]{};
    std::uint64_t next[kWordCount];
    write_date(text);
    std::memcpy(next, text, kStorageSize);
    sequence_.fetch_add(1, std::memory_order_seq_cst);
    for (std::size_t index = 0; index < kWordCount; ++index) {
      words_[index].store(next[index], std::memory_order_seq_cst);
    }
    sequence_.fetch_add(1, std::memory_order_seq_cst);
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  std::array<std::atomic<std::uint64_t>, kWordCount> words_{};
  std::atomic<std::uint64_t> sequence_{0};
  std::atomic<bool> running_{false};
  std::jthread jthread_;
};
}  // namespace martianlabs::doba::common

#endif
