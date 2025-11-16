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

#include "date_server_helpers.h"

namespace martianlabs::doba::common {
// =============================================================================
// date_server                                                         ( class )
// -----------------------------------------------------------------------------
// This class holds for a highly optimized dates server.
// -----------------------------------------------------------------------------
// =============================================================================
class date_server {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  date_server() = default;
  date_server(const date_server&) = delete;
  date_server(date_server&&) noexcept = delete;
  ~date_server() { stop(); }
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  date_server& operator=(const date_server&) = delete;
  date_server& operator=(date_server&&) noexcept = delete;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  void start() {
    if (running_) return;
    running_ = true;
    thread_ = std::thread([this] {
      bool toggle = false;
      while (running_) {
        std::time_t now = std::time(nullptr);
        std::tm gmt{};
        gm_time(&gmt, &now);
        char* target = toggle ? buf_a_ : buf_b_;
        std::snprintf(target, kBufSize, "%s, %02d %s %04d %02d:%02d:%02d GMT",
                      kWeekDays[gmt.tm_wday], gmt.tm_mday, kMonths[gmt.tm_mon],
                      1900 + gmt.tm_year, gmt.tm_hour, gmt.tm_min, gmt.tm_sec);

        front_.store(target, std::memory_order_release);
        toggle = !toggle;
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    });
  }
  void stop() {
    if (!running_) return;
    running_ = false;
    if (thread_.joinable()) thread_.join();
    front_.store(buf_a_, std::memory_order_release);
  }
  std::string_view get() { return front_.load(std::memory_order_acquire); }

 private:
  // ___________________________________________________________________________
  // CONSTANTs                                                       ( private )
  //
  static constexpr std::size_t kBufSize = 64;
  static constexpr const char* kWeekDays[] = {"Sun", "Mon", "Tue", "Wed",
                                              "Thu", "Fri", "Sat"};
  static constexpr const char* kMonths[] = {"Jan", "Feb", "Mar", "Apr",
                                            "May", "Jun", "Jul", "Aug",
                                            "Sep", "Oct", "Nov", "Dec"};
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  std::atomic<bool> running_{false};
  std::thread thread_;
  char buf_a_[kBufSize]{};
  char buf_b_[kBufSize]{};
  std::atomic<const char*> front_{buf_a_};
};
}  // namespace martianlabs::doba::common

#endif
