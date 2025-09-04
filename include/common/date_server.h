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
    thread_ = std::make_shared<std::thread>([this] {
      char buffer[64];
      bool toggle = false;
      while (running_) {
        std::time_t now = std::time(nullptr);
        std::tm gmt{};
        gm_time(&gmt, &now);
        std::snprintf(buffer, sizeof(buffer),
                      "%s, %02d %s %04d %02d:%02d:%02d GMT",
                      kWeekDays[gmt.tm_wday], gmt.tm_mday, kMonths[gmt.tm_mon],
                      1900 + gmt.tm_year, gmt.tm_hour, gmt.tm_min, gmt.tm_sec);
        std::string& target = toggle ? date_buf_a_ : date_buf_b_;
        target.assign(buffer);
        date_ptr_.store(&target, std::memory_order_release);
        toggle = !toggle;
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    });
    // let's do this wait to ensure that the date server put a valid value!
    while (get().empty()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(0));
    }
  }
  void stop() {
    if (!running_) return;
    running_ = false;
    if (thread_ && thread_->joinable()) {
      thread_->join();
    }
    thread_.reset();
    date_buf_a_.clear();
    date_buf_b_.clear();
    date_ptr_ = &date_buf_a_;
  }
  const std::string& get() {
    return *date_ptr_.load(std::memory_order_acquire);
  }

 private:
  // ___________________________________________________________________________
  // CONSTANTs                                                       ( private )
  //
  static constexpr const char* kWeekDays[] = {"Sun", "Mon", "Tue", "Wed",
                                              "Thu", "Fri", "Sat"};
  static constexpr const char* kMonths[] = {"Jan", "Feb", "Mar", "Apr",
                                            "May", "Jun", "Jul", "Aug",
                                            "Sep", "Oct", "Nov", "Dec"};
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  std::atomic<bool> running_ = false;
  std::string date_buf_a_;
  std::string date_buf_b_;
  std::shared_ptr<std::thread> thread_;
  std::atomic<const std::string*> date_ptr_{&date_buf_a_};
};
}  // namespace martianlabs::doba::common

#endif
