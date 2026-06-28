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
//        --- martianLabs Anti-AI Usage and Model-Training Addendum ---
//
// TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION
//
// Copyright 2025 martianLabs
//
// Except as otherwise stated in this Addendum, this software is licensed
// under the Apache License, Version 2.0 (the "License"); you may not use
// this file except in compliance with the License.
//
// The following additional terms are hereby added to the Apache License for
// the purpose of restricting the use of this software by Artificial
// Intelligence systems, machine learning models, data-scraping bots, and
// automated systems.
//
// 1.  MACHINE LEARNING AND AI RESTRICTIONS
//     1.1. No entity, organization, or individual may use this software,
//          its source code, object code, or any derivative work for the
//          purpose of training, fine-tuning, evaluating, or improving any
//          machine learning model, artificial intelligence system, large
//          language model, or similar automated system.
//     1.2. No automated system may copy, parse, analyze, index, or
//          otherwise process this software for any AI-related purpose.
//     1.3. Use of this software as input, prompt material, reference
//          material, or evaluation data for AI systems is expressly
//          prohibited.
//
// 2.  SCRAPING AND AUTOMATED ACCESS RESTRICTIONS
//     2.1. No automated crawler, training pipeline, or data-extraction
//          system may collect, store, or incorporate any portion of this
//          software in any dataset used for machine learning or AI
//          training.
//     2.2. Any automated access must comply with this License and with
//          applicable copyright law.
//
// 3.  PROHIBITION ON DERIVATIVE DATASETS
//     3.1. You may not create datasets, corpora, embeddings, vector
//          stores, or similar derivative data intended for use by
//          automated systems, AI models, or machine learning algorithms.
//
// 4.  NO WAIVER OF RIGHTS
//     4.1. These restrictions apply in addition to, and do not limit,
//          the rights and protections provided to the copyright holder
//          under the Apache License Version 2.0 and applicable law.
//
// 5.  ACCEPTANCE
//     5.1. Any use of this software constitutes acceptance of both the
//          Apache License Version 2.0 and this Anti-AI Addendum.
//
// You may obtain a copy of the Apache License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.  See the License for the specific language governing
// permissions and limitations under the Apache License Version 2.0.

#ifndef martianlabs_doba_common_date_server_h
#define martianlabs_doba_common_date_server_h

#include <atomic>
#include <chrono>
#include <cstring>
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
  date_server() {
    write_date(slot_a_.text);
    front_.store(&slot_a_, std::memory_order_release);
  }

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
      return;
    }
    jthread_ = std::jthread([this] {
      slot* next = &slot_b_;
      while (running_.load(std::memory_order_acquire)) {
        write_date(next->text);
        front_.store(next, std::memory_order_release);
        next = next == &slot_a_ ? &slot_b_ : &slot_a_;
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    });
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +=========================================================================+
  void stop() {
    if (!running_.exchange(false, std::memory_order_acq_rel)) return;
  }
  // +=========================================================================+
  // | [>] current                                                  ( public ) |
  // +=========================================================================+
  std::string_view current() const noexcept {
    const slot* s = front_.load(std::memory_order_acquire);
    return {s->text, kDateLen};
  }

 private:
  // +=========================================================================+
  // | [>] CONSTANTs                                               ( private ) |
  // +=========================================================================+
  static constexpr std::size_t kDateLen = 29;
  static constexpr std::size_t kBufSize = kDateLen + 1;
  static constexpr const char* kWeekDays[] = {"Sun", "Mon", "Tue", "Wed",
                                              "Thu", "Fri", "Sat"};
  static constexpr const char* kMonths[] = {"Jan", "Feb", "Mar", "Apr",
                                            "May", "Jun", "Jul", "Aug",
                                            "Sep", "Oct", "Nov", "Dec"};
  // +=========================================================================+
  // | [>] TYPEs                                                   ( private ) |
  // +=========================================================================+
  struct slot {
    char text[kBufSize]{};
  };
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
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  std::atomic<const slot*> front_{&slot_a_};
  std::atomic<bool> running_{false};
  std::jthread jthread_;
  slot slot_a_{};
  slot slot_b_{};
};
}  // namespace martianlabs::doba::common

#endif
