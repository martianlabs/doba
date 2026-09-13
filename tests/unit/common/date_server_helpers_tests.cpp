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

#include "common/date_server_helpers.h"

#include "test_helper.h"

namespace {
using martianlabs::doba::common::gm_time;
}  // namespace

// +===========================================================================+
// | [>] utc conversion preserves calendar boundaries            ( test-case ) |
// +===========================================================================+
DOBA_TEST("utc conversion preserves calendar boundaries") {
  struct test_case {
    time_t timestamp;
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int weekday;
    int yearday;
  };
  const test_case cases[] = {
      {0, 1970, 1, 1, 0, 0, 0, 4, 0},
      {86399, 1970, 1, 1, 23, 59, 59, 4, 0},
      {86400, 1970, 1, 2, 0, 0, 0, 5, 1},
      {951782400, 2000, 2, 29, 0, 0, 0, 2, 59},
      {951868800, 2000, 3, 1, 0, 0, 0, 3, 60},
      {978307199, 2000, 12, 31, 23, 59, 59, 0, 365},
      {978307200, 2001, 1, 1, 0, 0, 0, 1, 0},
  };
  for (const auto& value : cases) {
    struct tm output {};
    gm_time(&output, &value.timestamp);
    DOBA_EXPECT_EQUAL(output.tm_year + 1900, value.year);
    DOBA_EXPECT_EQUAL(output.tm_mon + 1, value.month);
    DOBA_EXPECT_EQUAL(output.tm_mday, value.day);
    DOBA_EXPECT_EQUAL(output.tm_hour, value.hour);
    DOBA_EXPECT_EQUAL(output.tm_min, value.minute);
    DOBA_EXPECT_EQUAL(output.tm_sec, value.second);
    DOBA_EXPECT_EQUAL(output.tm_wday, value.weekday);
    DOBA_EXPECT_EQUAL(output.tm_yday, value.yearday);
    DOBA_EXPECT_EQUAL(output.tm_isdst, 0);
  }
}
