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

#ifndef martianlabs_doba_protocol_http11_checkers_date_h
#define martianlabs_doba_protocol_http11_checkers_date_h

#include <string_view>

namespace martianlabs::doba::protocol::http11::checkers {
// =============================================================================
// |                                                                  [ date ] |
// +---------------------------------------------------------------------------+
// | RFC 9110 §10.1.1 Date                                                     |
// +---------------------------------------------------------------------------+
// | The "Date" header field represents the date and time at which the message |
// | was originated.                                                           |
// |                                                                           |
// | An origin server MUST send a Date header field in all responses,          |
// | except in these cases:                                                    |
// |                                                                           |
// |   * A response to a 1xx (Informational) request,                          |
// |   * A 204 (No Content) response, or                                       |
// |   * A 304 (Not Modified) response.                                        |
// |                                                                           |
// | A recipient with a clock that receives a response message without a Date  |
// | header field MUST record the time it was received and use that value      |
// | as the Date field value.                                                  |
// |                                                                           |
// | The field value is an HTTP-date, as defined in Section 5.6.7              |
// | ("Date/Time Formats").                                                    |
// |                                                                           |
// | Example:                                                                  |
// |  Date: Tue, 15 Nov 1994 08:12:31 GMT                                      |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.7 Date/Time Formats (ABNF summary)                          |
// +---------------------------------------------------------------------------+
// +----------------+----------------------------------------------------------+
// | Field          | Definition                                               |
// +----------------+----------------------------------------------------------+
// | Date           | HTTP-date                                                |
// | HTTP-date      | IMF-fixdate                                              |
// | IMF-fixdate    | day-name "," SP date1 SP time-of-day SP GMT              |
// | day-name       | "Mon" / "Tue" / "Wed" / "Thu" / "Fri" / "Sat" /          |
// |                | "Sun"                                                    |
// | date1          | day SP month SP year                                     |
// | day            | 2DIGIT                                                   |
// | month          | "Jan" / "Feb" / "Mar" / "Apr" / "May" / "Jun" /          |
// |                | "Jul" / "Aug" / "Sep" / "Oct" / "Nov" / "Dec"            |
// | year           | 4DIGIT                                                   |
// | time-of-day    | hour ":" minute ":" second                               |
// | hour           | 2DIGIT                                                   |
// | minute         | 2DIGIT                                                   |
// | second         | 2DIGIT                                                   |
// | GMT            | literal "GMT"                                            |
// +----------------+----------------------------------------------------------+
// =============================================================================
static auto date_check_fn = [](std::string_view v) -> bool { return true; };
}  // namespace martianlabs::doba::protocol::http11::checkers

#endif
