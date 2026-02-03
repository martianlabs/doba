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

#ifndef martianlabs_doba_protocol_http11_checkers_date_h
#define martianlabs_doba_protocol_http11_checkers_date_h

#include <string_view>

#include "common/hash_set.h"
#include "protocol/http11/helpers.h"

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
static auto date_fn = [](std::string_view v) -> bool {
  static constexpr std::string_view kDays[] = {"Mon", "Tue", "Wed", "Thu",
                                               "Fri", "Sat", "Sun"};
  static constexpr std::string_view kMons[] = {"Jan", "Feb", "Mar", "Apr",
                                               "May", "Jun", "Jul", "Aug",
                                               "Sep", "Oct", "Nov", "Dec"};
  // "DDD, DD MMM YYYY HH:MM:SS GMT" --> 29 characters (fixed size)!
  if (v.size() != 29) return false;
  // [day-name]
  bool day_name_ok = false;
  std::string_view day_name = v.substr(0, 3);
  for (auto const& day : kDays) {
    if ((day_name_ok = !day.compare(day_name))) break;
  }
  if (!day_name_ok) return false;
  // [,<sp>]
  if (v[3] != constants::character::kComma) return false;
  if (v[4] != constants::character::kSpace) return false;
  // [day]
  if (!helpers::is_digit(v[5]) || !helpers::is_digit(v[6])) return false;
  // [ ]
  if (v[7] != constants::character::kSpace) return false;
  // [month]
  bool month_name_ok = false;
  std::string_view month_name = v.substr(8, 3);
  for (auto const& month : kMons) {
    if ((month_name_ok = !month.compare(month_name))) break;
  }
  if (!month_name_ok) return false;
  // [<sp>]
  if (v[11] != constants::character::kSpace) return false;
  // [year]
  if (!helpers::is_digit(v[12]) || !helpers::is_digit(v[13]) ||
      !helpers::is_digit(v[14]) || !helpers::is_digit(v[15])) {
    return false;
  }
  // [<sp>]
  if (v[16] != constants::character::kSpace) return false;
  // [hour]
  if (!helpers::is_digit(v[17]) || !helpers::is_digit(v[18])) return false;
  // [:]
  if (v[19] != constants::character::kColon) return false;
  // [minute]
  if (!helpers::is_digit(v[20]) || !helpers::is_digit(v[21])) return false;
  // [:]
  if (v[22] != constants::character::kColon) return false;
  // [second]
  if (!helpers::is_digit(v[23]) || !helpers::is_digit(v[24])) return false;
  // [<sp>]
  if (v[25] != constants::character::kSpace) return false;
  // [GMT]
  if (v[26] != 'G' || v[27] != 'M' || v[28] != 'T') return false;
  return true;
};
}  // namespace martianlabs::doba::protocol::http11::checkers

#endif
