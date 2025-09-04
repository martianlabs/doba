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

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

#include "protocol/http11/server.h"

using namespace martianlabs::doba::common;
using namespace martianlabs::doba::protocol::http11;

// Tablas preformateadas para días y meses (3 chars cada uno).
static const char* kWeekDays[] = {"Sun", "Mon", "Tue", "Wed",
                                  "Thu", "Fri", "Sat"};
static const char* kMonths[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

// Cache thread-local: cada worker mantiene su Date string.
inline const std::string& make_date_header() {
  thread_local std::string cached;
  thread_local std::time_t last_sec = 0;

  std::time_t now = std::time(nullptr);
  if (now != last_sec) {
    last_sec = now;
    std::tm gmt{};
#if defined(_WIN32)
    gmtime_s(&gmt, &now);
#else
    gmtime_r(&now, &gmt);
#endif
    char buf[40];
    // Formato: "Sun, 06 Nov 1994 08:49:37 GMT"
    std::snprintf(buf, sizeof(buf), "%s, %02d %s %04d %02d:%02d:%02d GMT",
                  kWeekDays[gmt.tm_wday], gmt.tm_mday, kMonths[gmt.tm_mon],
                  1900 + gmt.tm_year, gmt.tm_hour, gmt.tm_min, gmt.tm_sec);
    cached.assign(buf);
  }
  return cached;
}

int main(int argc, char* argv[]) {
  server my_server("10001");
  my_server
      .add_route(
          method::kGet, "/plaintext",
          [](std::shared_ptr<const request> req,
             std::shared_ptr<response> res) {
            res->ok_200()
                .add_header("Date", make_date_header())
                .add_header("Content-Type", "text/plain")
                .add_header("Connection", "keep-alive")
                .add_header("Content-Length", 13)
                .add_body("Hello, World!");
          },
          execution_policy::kAsync)
      .start();
  return getchar();
}
