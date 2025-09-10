//      _       _           
//   __| | ___ | |__   __ _ 
//  / _` |/ _ \| '_ \ / _` |
// | (_| | (_) | |_) | (_| |
//  \__,_|\___/|_.__/ \__,_|
// 
// Copyright 2025 martianLabs
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
