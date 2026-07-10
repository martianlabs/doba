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

#ifndef martianlabs_doba_protocol_http11_headers_date_h
#define martianlabs_doba_protocol_http11_headers_date_h

#include <string_view>

#include "common/hash_set.h"
#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] date                                                        ( class ) |
// +---------------------------------------------------------------------------+
// | RFC 9110 §6.6.1 Date                                                      |
// +---------------------------------------------------------------------------+
// | The "Date" header field represents the date and time at which the message |
// | was originated. Its field value is an HTTP-date, as defined in RFC 9110   |
// | Section 5.6.7 ("Date/Time Formats").                                      |
// |                                                                           |
// | A sender SHOULD generate the field value as the best available            |
// | approximation of the date and time at which the message was generated.    |
// |                                                                           |
// | An origin server with a clock MUST generate a Date header field in all    |
// | 2xx, 3xx, and 4xx responses. It MAY generate one in 1xx and 5xx           |
// | responses. An origin server without a clock MUST NOT generate this field. |
// |                                                                           |
// | A recipient parsing an HTTP-date MUST accept all three date formats:      |
// |                                                                           |
// |   * IMF-fixdate                                                           |
// |   * rfc850-date                                                           |
// |   * asctime-date                                                          |
// |                                                                           |
// | A sender generating an HTTP-date MUST use IMF-fixdate.                    |
// |                                                                           |
// | Example:                                                                  |
// |   Date: Tue, 15 Nov 1994 08:12:31 GMT                                     |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.7 Date/Time Formats — ABNF                                  |
// +---------------------------------------------------------------------------+
// +----------------+----------------------------------------------------------+
// | Field          | Definition                                               |
// +----------------+----------------------------------------------------------+
// | Date           | HTTP-date                                                |
// | HTTP-date      | IMF-fixdate / obs-date                                   |
// |                |                                                          |
// | IMF-fixdate    | day-name "," SP date1 SP time-of-day SP GMT              |
// | day-name       | %s"Mon" / %s"Tue" / %s"Wed" / %s"Thu" / %s"Fri" /        |
// |                | %s"Sat" / %s"Sun"                                        |
// | date1          | day SP month SP year                                     |
// | day            | 2DIGIT                                                   |
// | month          | %s"Jan" / %s"Feb" / %s"Mar" / %s"Apr" / %s"May" /        |
// |                | %s"Jun" / %s"Jul" / %s"Aug" / %s"Sep" / %s"Oct" /        |
// |                | %s"Nov" / %s"Dec"                                        |
// | year           | 4DIGIT                                                   |
// | GMT            | %s"GMT"                                                  |
// | time-of-day    | hour ":" minute ":" second                               |
// | hour           | 2DIGIT                                                   |
// | minute         | 2DIGIT                                                   |
// | second         | 2DIGIT                                                   |
// |                |                                                          |
// | obs-date       | rfc850-date / asctime-date                               |
// |                |                                                          |
// | rfc850-date    | day-name-l "," SP date2 SP time-of-day SP GMT            |
// | date2          | day "-" month "-" 2DIGIT                                 |
// | day-name-l     | %s"Monday" / %s"Tuesday" / %s"Wednesday" /               |
// |                | %s"Thursday" / %s"Friday" / %s"Saturday" / %s"Sunday"    |
// |                |                                                          |
// | asctime-date   | day-name SP date3 SP time-of-day SP year                 |
// | date3          | month SP ( 2DIGIT / ( SP 1DIGIT ) )                      |
// +---------------------------------------------------------------------------+
// | RFC 5234 core rules used above:                                           |
// |                                                                           |
// |   DIGIT = %x30-39                                                         |
// |   SP    = %x20                                                            |
// |                                                                           |
// | The %s prefix indicates a case-sensitive string. Therefore, day names,    |
// | month names, and "GMT" have exactly the capitalization shown above.       |
// |                                                                           |
// | No whitespace other than the SP characters explicitly present in the      |
// | grammar is permitted inside an HTTP-date.                                 |
// |                                                                           |
// | IMPORTANT: field-value is expected to be normalized, with no OWS around   |
// | the value, before this syntax validator is called.                        |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class date {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view v) {
    return is_imf_fixdate(v) || is_rfc850_date(v) || is_asctime_date(v);
  }

 private:
  // +=========================================================================+
  // | [>] CONSTANTs                                               ( private ) |
  // +=========================================================================+
  static constexpr inline std::size_t kDaysInWeek = 7;
  static constexpr inline std::size_t kMonthsInYear = 12;
  static constexpr inline std::string_view kShortDayNames[kDaysInWeek] = {
      "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
  static constexpr inline std::string_view kLongDayNames[kDaysInWeek] = {
      "Monday", "Tuesday",  "Wednesday", "Thursday",
      "Friday", "Saturday", "Sunday"};
  static constexpr inline std::string_view kShortMonthNames[kMonthsInYear] = {
      "Jan", "Feb", "Mar", "Apr", "May", "Jun",
      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  // +=========================================================================+
  // | [>] parse_short_day_name                                    ( private ) |
  // +=========================================================================+
  static constexpr bool parse_short_day_name(std::string_view sv,
                                             std::string_view delimiter,
                                             std::size_t& used_characters) {
    std::size_t delimiter_pos = sv.find(delimiter);
    if (delimiter_pos == std::string_view::npos) return false;
    for (std::size_t i = 0; i < kDaysInWeek; i++) {
      if (!kShortDayNames[i].compare(sv.substr(0, delimiter_pos))) {
        used_characters = delimiter_pos + delimiter.size();
        return true;
      }
    }
    return false;
  }
  // +=========================================================================+
  // | [>] parse_long_day_name                                     ( private ) |
  // +=========================================================================+
  static constexpr bool parse_long_day_name(std::string_view sv,
                                            std::string_view delimiter,
                                            std::size_t& used_characters) {
    std::size_t delimiter_pos = sv.find(delimiter);
    if (delimiter_pos == std::string_view::npos) return false;
    for (std::size_t i = 0; i < kDaysInWeek; i++) {
      if (!kLongDayNames[i].compare(sv.substr(0, delimiter_pos))) {
        used_characters = delimiter_pos + delimiter.size();
        return true;
      }
    }
    return false;
  }
  // +=========================================================================+
  // | [>] parse_short_month_name                                  ( private ) |
  // +=========================================================================+
  static constexpr bool parse_short_month_name(std::string_view sv,
                                               std::string_view delimiter,
                                               std::size_t& used_characters) {
    std::size_t delimiter_pos = sv.find(delimiter);
    if (delimiter_pos == std::string_view::npos) return false;
    for (std::size_t i = 0; i < kMonthsInYear; i++) {
      if (!kShortMonthNames[i].compare(sv.substr(0, delimiter_pos))) {
        used_characters = delimiter_pos + delimiter.size();
        return true;
      }
    }
    return false;
  }
  // +=========================================================================+
  // | [>] parse_n_digit_number                                    ( private ) |
  // +=========================================================================+
  static constexpr bool parse_n_digit_number(std::string_view sv,
                                             std::size_t n) {
    if (sv.size() < n) return false;
    for (std::size_t i = 0; i < n; i++) {
      if (!helpers::is_digit(sv[i])) return false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] parse_asctime_day                                       ( private ) |
  // +=========================================================================+
  static constexpr bool parse_asctime_day(std::string_view sv) {
    if (sv.size() < 2) return false;
    return (helpers::is_digit(sv[0]) && helpers::is_digit(sv[1])) ||
           (sv[0] == ' ' && helpers::is_digit(sv[1]));
  }
  // +=========================================================================+
  // | [>] IMF-fixdate                                             ( private ) |
  // +=========================================================================+
  // | "DDD, DD MMM YYYY HH:MM:SS GMT" --> 29 characters (fixed size)          |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_imf_fixdate(std::string_view sv) {
    constexpr std::size_t kImfFixdateLength = 29;
    const std::size_t len = sv.size();
    if (len != kImfFixdateLength) return false;
    std::size_t i = 0, bytes_used = 0;
    if (!parse_short_day_name(sv, ", ", bytes_used)) return false;
    i += bytes_used;
    if (!parse_n_digit_number(sv.substr(i), 2)) return false;
    i += 2;
    if (len <= i || sv[i] != ' ') return false;
    i++;
    if (!parse_short_month_name(sv.substr(i), " ", bytes_used)) return false;
    i += bytes_used;
    if (!parse_n_digit_number(sv.substr(i), 4)) return false;
    i += 4;
    if (len <= i || sv[i] != ' ') return false;
    i++;
    if (!parse_n_digit_number(sv.substr(i), 2)) return false;
    i += 2;
    if (len <= i || sv[i] != ':') return false;
    i++;
    if (!parse_n_digit_number(sv.substr(i), 2)) return false;
    i += 2;
    if (len <= i || sv[i] != ':') return false;
    i++;
    if (!parse_n_digit_number(sv.substr(i), 2)) return false;
    i += 2;
    if (len <= i || sv[i] != ' ') return false;
    i++;
    if (len < i + 3 || sv[i] != 'G' || sv[i + 1] != 'M' || sv[i + 2] != 'T') {
      return false;
    }
    i += 3;
    return len == i;
  }
  // +=========================================================================+
  // | [>] is_rfc850_date                                          ( private ) |
  // +=========================================================================+
  // | "DDD, DD MMM YYYY HH:MM:SS GMT" --> 29 characters (fixed size)          |
  // | "DDDDDDDDD, DD-MMM-YY HH:MM:SS GMT" --> 30-33 characters                |
  // +-------------------------------------------------------------------------+
  // | Variable size because the full day name contains 6-9 characters.        |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_rfc850_date(std::string_view sv) {
    constexpr std::size_t kMinRfc850DateLength = 30;
    constexpr std::size_t kMaxRfc850DateLength = 33;
    const std::size_t len = sv.size();
    if (len < kMinRfc850DateLength || len > kMaxRfc850DateLength) return false;
    std::size_t i = 0, bytes = 0;
    if (!parse_long_day_name(sv, ", ", bytes)) return false;
    i += bytes;
    if (!parse_n_digit_number(sv.substr(i), 2)) return false;
    i += 2;
    if (len <= i || sv[i] != '-') return false;
    i++;
    if (!parse_short_month_name(sv.substr(i), "-", bytes)) return false;
    i += bytes;
    if (!parse_n_digit_number(sv.substr(i), 2)) return false;
    i += 2;
    if (len <= i || sv[i] != ' ') return false;
    i++;
    if (!parse_n_digit_number(sv.substr(i), 2)) return false;
    i += 2;
    if (len <= i || sv[i] != ':') return false;
    i++;
    if (!parse_n_digit_number(sv.substr(i), 2)) return false;
    i += 2;
    if (len <= i || sv[i] != ':') return false;
    i++;
    if (!parse_n_digit_number(sv.substr(i), 2)) return false;
    i += 2;
    if (len <= i || sv[i] != ' ') return false;
    i++;
    if (len < i + 3 || sv[i] != 'G' || sv[i + 1] != 'M' || sv[i + 2] != 'T') {
      return false;
    }
    i += 3;
    return len == i;
  }
  // +=========================================================================+
  // | [>] is_asctime_date                                         ( private ) |
  // +=========================================================================+
  // | "DDD MMM DD HH:MM:SS YYYY" --> 24 characters (fixed size)               |
  // +-------------------------------------------------------------------------+
  // | A single-digit day is represented as SP DIGIT, e.g. "Nov  6".           |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_asctime_date(std::string_view sv) {
    constexpr std::size_t kAsctimeDateLength = 24;
    const std::size_t len = sv.size();
    if (len != kAsctimeDateLength) return false;
    std::size_t off = 0, bytes_used = 0;
    if (!parse_short_day_name(sv, " ", bytes_used)) return false;
    off += bytes_used;
    if (!parse_short_month_name(sv.substr(off), " ", bytes_used)) return false;
    off += bytes_used;
    if (!parse_asctime_day(sv.substr(off))) return false;
    off += 2;
    if (len <= off || sv[off] != ' ') return false;
    off++;
    if (!parse_n_digit_number(sv.substr(off), 2)) return false;
    off += 2;
    if (len <= off || sv[off] != ':') return false;
    off++;
    if (!parse_n_digit_number(sv.substr(off), 2)) return false;
    off += 2;
    if (len <= off || sv[off] != ':') return false;
    off++;
    if (!parse_n_digit_number(sv.substr(off), 2)) return false;
    off += 2;
    if (len <= off || sv[off] != ' ') return false;
    off++;
    if (!parse_n_digit_number(sv.substr(off), 4)) return false;
    off += 4;
    return off == len;
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
