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

#ifndef martianlabs_doba_tests_unit_http_date_h
#define martianlabs_doba_tests_unit_http_date_h

#include <chrono>
#include <cstddef>
#include <string_view>

namespace martianlabs::doba::tests {
inline bool valid_http_date(std::string_view value) {
  if (value.size() != 29 || value[3] != ',' || value[4] != ' ' ||
      value[7] != ' ' || value[11] != ' ' || value[16] != ' ' ||
      value[19] != ':' || value[22] != ':' || value[25] != ' ' ||
      value.substr(26) != "GMT")
    return false;
  constexpr std::string_view days[] = {"Sun", "Mon", "Tue", "Wed",
                                       "Thu", "Fri", "Sat"};
  constexpr std::string_view months[] = {"Jan", "Feb", "Mar", "Apr",
                                         "May", "Jun", "Jul", "Aug",
                                         "Sep", "Oct", "Nov", "Dec"};
  constexpr std::size_t digits[] = {5,  6,  12, 13, 14, 15,
                                    17, 18, 20, 21, 23, 24};
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
  if (!date.ok() || number(17) > 23 || number(20) > 59 || number(23) > 60)
    return false;
  const std::chrono::weekday weekday{std::chrono::sys_days(date)};
  return value.substr(0, 3) == days[weekday.c_encoding()];
}

}  // namespace martianlabs::doba::tests

#endif
