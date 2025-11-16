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
static auto date_check_fn = [](std::string_view v) -> bool {
  static const common::hash_set<std::string> day_names = {
      "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
  static const common::hash_set<std::string> month_names = {
      "Jan", "Feb", "Mar", "Apr", "May", "Jun",
      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  std::size_t i = 0;
  // [day-name]
  if (v.size() < 3) return false;
  std::string_view day(v.begin(), v.begin() + 3);
  if (day_names.find(day) == day_names.end()) return false;
  i += 3;
  if (i >= v.size()) return false;
  if (v[i++] != constants::character::kComma) return false;
  if (i >= v.size()) return false;
  if (v[i++] != constants::character::kSpace) return false;
  // [day]
  if (i + 1 >= v.size()) return false;
  if (!helpers::is_digit(v[i]) || !helpers::is_digit(v[i + 1])) return false;
  i += 2;
  if (i >= v.size()) return false;
  if (v[i++] != constants::character::kSpace) return false;
  // [month]
  if (i + 2 >= v.size()) return false;
  std::string_view mon(v.begin() + i, v.begin() + i + 3);
  if (month_names.find(mon) == month_names.end()) return false;
  i += 3;
  if (i >= v.size()) return false;
  if (v[i++] != constants::character::kSpace) return false;
  // [year]
  if (i + 3 >= v.size()) return false;
  if (!helpers::is_digit(v[i]) || !helpers::is_digit(v[i + 1]) ||
      !helpers::is_digit(v[i + 2]) || !helpers::is_digit(v[i + 3])) {
    return false;
  }
  i += 4;
  if (i >= v.size()) return false;
  if (v[i++] != constants::character::kSpace) return false;
  // [hour]
  if (i + 1 >= v.size()) return false;
  if (!helpers::is_digit(v[i]) || !helpers::is_digit(v[i + 1])) return false;
  i += 2;
  if (i >= v.size()) return false;
  if (v[i++] != constants::character::kColon) return false;
  // [minute]
  if (i + 1 >= v.size()) return false;
  if (!helpers::is_digit(v[i]) || !helpers::is_digit(v[i + 1])) return false;
  i += 2;
  if (i >= v.size()) return false;
  if (v[i++] != constants::character::kColon) return false;
  // [second]
  if (i + 1 >= v.size()) return false;
  if (!helpers::is_digit(v[i]) || !helpers::is_digit(v[i + 1])) return false;
  i += 2;
  // [gmt]
  if (i + 3 >= v.size()) return false;
  if (v[i] != constants::character::kSpace || v[i + 1] != 'G' ||
      v[i + 2] != 'M' || v[i + 3] != 'T') {
    return false;
  }
  i += 4;
  return i == v.size();
};
}  // namespace martianlabs::doba::protocol::http11::checkers

#endif
