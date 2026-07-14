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

#ifndef martianlabs_doba_protocol_http11_headers_expires_h
#define martianlabs_doba_protocol_http11_headers_expires_h

#include "protocol/http11/headers/date.h"
#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                                   expires |
// +===========================================================================+
// | RFC 9111 �5.3 Expires                                                     |
// +---------------------------------------------------------------------------+
// | The "Expires" response header field gives the date/time after which the   |
// | response is considered stale. It is part of the HTTP caching model and is |
// | used to compute an explicit freshness lifetime when more specific cache   |
// | directives are not present.                                               |
// |                                                                           |
// | The presence of an Expires header field does not imply that the original  |
// | resource will change or cease to exist at, before, or after that time.    |
// | It only describes cache freshness.                                        |
// |                                                                           |
// | The field value is a single HTTP-date timestamp. A sender that generates  |
// | an HTTP-date timestamp MUST use the IMF-fixdate format. A recipient that  |
// | parses an HTTP-date value MUST accept all three HTTP-date formats:        |
// | IMF-fixdate, rfc850-date, and asctime-date.                               |
// |                                                                           |
// | If Cache-Control contains max-age, a recipient MUST ignore Expires. If    |
// | Cache-Control contains s-maxage, a shared cache recipient MUST ignore     |
// | Expires. In these cases, Expires is only for recipients that do not yet   |
// | implement Cache-Control.                                                  |
// |                                                                           |
// | A cache recipient MUST interpret invalid date formats, especially "0", as |
// | representing a time in the past, meaning the response is already expired. |
// | This is cache semantics, not ABNF: "0" is not a syntactically valid       |
// | Expires field value for a strict ABNF checker.                            |
// |                                                                           |
// | Examples:                                                                 |
// |   Expires: Thu, 01 Dec 1994 16:00:00 GMT                                  |
// |   Expires: Sun, 06 Nov 1994 08:49:37 GMT                                  |
// +---------------------------------------------------------------------------+
// | RFC 9111 �5.3 Expires (ABNF summary)                                      |
// +---------------------------------------------------------------------------+
// +--------------+------------------------------------------------------------+
// | Field        | Definition                                                 |
// +--------------+------------------------------------------------------------+
// | Expires      | HTTP-date                                                  |
// | HTTP-date    | IMF-fixdate / obs-date                                     |
// | obs-date     | rfc850-date / asctime-date                                 |
// +---------------------------------------------------------------------------+
// | RFC 9110 �5.6.7 Date/Time Formats                                         |
// +---------------------------------------------------------------------------+
// +--------------+------------------------------------------------------------+
// | Field        | Definition                                                 |
// +--------------+------------------------------------------------------------+
// | IMF-fixdate  | day-name "," SP date1 SP time-of-day SP GMT                |
// | day-name     | %s"Mon" / %s"Tue" / %s"Wed" / %s"Thu" / %s"Fri" /          |
// |              | %s"Sat" / %s"Sun"                                          |
// | date1        | day SP month SP year                                       |
// | day          | 2DIGIT                                                     |
// | month        | %s"Jan" / %s"Feb" / %s"Mar" / %s"Apr" / %s"May" /          |
// |              | %s"Jun" / %s"Jul" / %s"Aug" / %s"Sep" / %s"Oct" /          |
// |              | %s"Nov" / %s"Dec"                                          |
// | year         | 4DIGIT                                                     |
// | GMT          | %s"GMT"                                                    |
// | time-of-day  | hour ":" minute ":" second                                 |
// | hour         | 2DIGIT                                                     |
// | minute       | 2DIGIT                                                     |
// | second       | 2DIGIT                                                     |
// +--------------+------------------------------------------------------------+
// | rfc850-date  | day-name-l "," SP date2 SP time-of-day SP GMT              |
// | date2        | day "-" month "-" 2DIGIT                                   |
// | day-name-l   | %s"Monday" / %s"Tuesday" / %s"Wednesday" /                 |
// |              | %s"Thursday" / %s"Friday" / %s"Saturday" / %s"Sunday"      |
// +--------------+------------------------------------------------------------+
// | asctime-date | day-name SP date3 SP time-of-day SP year                   |
// | date3        | month SP ( 2DIGIT / ( SP 1DIGIT ) )                        |
// +---------------------------------------------------------------------------+
// | Notes                                                                     |
// +---------------------------------------------------------------------------+
// | Expires is not a list field. It does not use the RFC 9110 �5.6.1 "#rule"  |
// | list extension, and comma-separated multiple values are not               |
// | valid ABNF for| this field.                                               |
// |                                                                           |
// | HTTP-date is case-sensitive. Cache recipients are allowed to match date   |
// | values case-insensitively as a cache-specific robustness rule.            |
// |                                                                           |
// | Senders MUST NOT generate additional whitespace in an HTTP-date beyond    |
// | the SP characters explicitly present in the grammar.                      |
// |                                                                           |
// | For pure syntactic validation, the normalized field-value must be exactly |
// | one HTTP-date.                                                            |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class expires {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) { return date::check(sv); }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
