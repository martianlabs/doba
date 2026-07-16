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

#ifndef martianlabs_doba_protocol_http11_headers_last_modified_h
#define martianlabs_doba_protocol_http11_headers_last_modified_h

#include <ranges>
#include <string_view>

#include "protocol/http11/headers/date.h"
#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                             last-modified |
// +===========================================================================+
// | RFC 9110 �8.8.2 Last-Modified                                             |
// +---------------------------------------------------------------------------+
// | The "Last-Modified" header field in a response provides the date and time |
// | at which the origin server believes the selected representation was last  |
// | modified, as determined when request handling concludes.                  |
// |                                                                           |
// | It is a representation validator. Its use in conditional requests and     |
// | cache freshness evaluation can avoid unnecessary representation transfers |
// | and improve availability and scalability.                                 |
// |                                                                           |
// | An origin server SHOULD send Last-Modified whenever the selected          |
// | representation has a modification date that can be determined reasonably  |
// | and consistently.                                                         |
// |                                                                           |
// | The value SHOULD be obtained as close as possible to the time at which    |
// | the response's Date field is generated.                                   |
// |                                                                           |
// | An origin server with a clock MUST NOT generate a Last-Modified value     |
// | later than the response's message origination time. A future value        |
// | obtained from representation metadata MUST be replaced with the message   |
// | origination date.                                                         |
// |                                                                           |
// | An origin server without a clock MUST NOT generate Last-Modified unless   |
// | the value was assigned to the resource by another system, presumably one  |
// | with a clock.                                                             |
// |                                                                           |
// | When used as a validator, Last-Modified is implicitly weak unless the     |
// | conditions in RFC 9110 �8.8.2.2 allow it to be treated as strong.         |
// |                                                                           |
// | Example:                                                                  |
// |   Last-Modified: Tue, 15 Nov 1994 12:45:26 GMT                            |
// +---------------------------------------------------------------------------+
// | RFC 9110 �8.8.2 Last-Modified (ABNF summary)                              |
// +---------------------------------------------------------------------------+
// +------------------+--------------------------------------------------------+
// | Field            | Definition                                             |
// +------------------+--------------------------------------------------------+
// | Last-Modified    | HTTP-date                                              |
// | HTTP-date        | IMF-fixdate / obs-date                                 |
// | IMF-fixdate      | day-name "," SP date1 SP time-of-day SP GMT            |
// | day-name         | %s"Mon" / %s"Tue" / %s"Wed" / %s"Thu"                  |
// |                  |  / %s"Fri" / %s"Sat" / %s"Sun"                         |
// | date1            | day SP month SP year                                   |
// | day              | 2DIGIT                                                 |
// | month            | %s"Jan" / %s"Feb" / %s"Mar" / %s"Apr"                  |
// |                  |  / %s"May" / %s"Jun" / %s"Jul" / %s"Aug"               |
// |                  |  / %s"Sep" / %s"Oct" / %s"Nov" / %s"Dec"               |
// | year             | 4DIGIT                                                 |
// | GMT              | %s"GMT"                                                |
// | time-of-day      | hour ":" minute ":" second                             |
// | hour             | 2DIGIT                                                 |
// | minute           | 2DIGIT                                                 |
// | second           | 2DIGIT                                                 |
// | obs-date         | rfc850-date / asctime-date                             |
// | rfc850-date      | day-name-l "," SP date2 SP time-of-day SP GMT          |
// | date2            | day "-" month "-" 2DIGIT                               |
// | day-name-l       | %s"Monday" / %s"Tuesday" / %s"Wednesday"               |
// |                  |  / %s"Thursday" / %s"Friday" / %s"Saturday"            |
// |                  |  / %s"Sunday"                                          |
// | asctime-date     | day-name SP date3 SP time-of-day SP year               |
// | date3            | month SP ( 2DIGIT / ( SP 1DIGIT ) )                    |
// | SP               | %x20                                                   |
// | DIGIT            | %x30-39                                                |
// +---------------------------------------------------------------------------+
// | RFC 9110 �5.6.7 HTTP-date requirements                                    |
// +---------------------------------------------------------------------------+
// | A recipient that parses an HTTP timestamp MUST accept all three formats:  |
// | IMF-fixdate, rfc850-date, and asctime-date.                               |
// |                                                                           |
// | A sender MUST generate HTTP-date values only in IMF-fixdate format. The   |
// | obsolete formats exist solely for recipient compatibility.                |
// |                                                                           |
// | HTTP-date is case-sensitive. A sender MUST NOT generate whitespace beyond |
// | the exact SP characters required by the grammar.                          |
// |                                                                           |
// | Accepted recipient forms:                                                 |
// |   Sun, 06 Nov 1994 08:49:37 GMT    ; IMF-fixdate                          |
// |   Sunday, 06-Nov-94 08:49:37 GMT   ; obsolete RFC 850 format              |
// |   Sun Nov  6 08:49:37 1994         ; ANSI C asctime() format              |
// |                                                                           |
// | The time-of-day semantics are 00:00:00 through 23:59:60, where second 60  |
// | represents a leap second.                                                 |
// |                                                                           |
// | For rfc850-date, a two-digit year appearing more than 50 years in the     |
// | future MUST be interpreted as the most recent past year with the same     |
// | final two digits.                                                         |
// |                                                                           |
// | Last-Modified is not a list-based field. Its normalized field value       |
// | contains exactly one HTTP-date; commas only occur where required by the   |
// | date syntax.                                                              |
// |                                                                           |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is normalized (no OWS around the value).           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class last_modified {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) { return date::check(sv); }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
