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

#ifndef martianlabs_doba_protocol_http11_headers_retry_after_h
#define martianlabs_doba_protocol_http11_headers_retry_after_h

#include "protocol/http11/headers/date.h"
#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                               retry-after |
// +===========================================================================+
// | RFC 9110 §10.2.3 Retry-After                                              |
// +---------------------------------------------------------------------------+
// | The "Retry-After" response header field indicates how long the user agent |
// | ought to wait before making a follow-up request.                          |
// |                                                                           |
// | The field value can be either an HTTP-date or a number of                 |
// | seconds to delay after receiving the response.                            |
// |                                                                           |
// | When sent with a 503 (Service Unavailable) response,                      |
// | Retry-After indicates how long the service is expected to be unavailable. |
// |                                                                           |
// | When sent with any 3xx (Redirection) response, Retry-After indicates the  |
// | minimum time that the user agent is asked to wait before issuing the      |
// | redirected request.                                                       |
// |                                                                           |
// | Retry-After is also commonly used with 429 (Too Many Requests) responses  |
// | to indicate how long the client ought to wait before sending another      |
// | request.                                                                  |
// |                                                                           |
// | Examples:                                                                 |
// |   Retry-After: Fri, 31 Dec 1999 23:59:59 GMT                              |
// |   Retry-After: 120                                                        |
// +---------------------------------------------------------------------------+
// | RFC 9110 §10.2.3 Retry-After (ABNF summary)                               |
// +---------------------------------------------------------------------------+
// +---------------+-----------------------------------------------------------+
// | Field         | Definition                                                |
// +---------------+-----------------------------------------------------------+
// | Retry-After   | HTTP-date / delay-seconds                                 |
// | delay-seconds | 1*DIGIT                                                   |
// +---------------+-----------------------------------------------------------+
// | RFC 9110 §5.6.7 Date/Time Formats                                         |
// +---------------------------------------------------------------------------+
// +---------------+-----------------------------------------------------------+
// | Field         | Definition                                                |
// +---------------+-----------------------------------------------------------+
// | HTTP-date     | IMF-fixdate / obs-date                                    |
// | IMF-fixdate   | day-name "," SP date1 SP time-of-day SP GMT               |
// | obs-date      | rfc850-date / asctime-date                                |
// | rfc850-date   | day-name-l "," SP date2 SP time-of-day SP GMT             |
// | asctime-date  | day-name SP date3 SP time-of-day SP year                  |
// | date1         | day SP month SP year                                      |
// | date2         | day "-" month "-" 2DIGIT                                  |
// | date3         | month SP ( 2DIGIT / ( SP 1DIGIT ) )                       |
// | day           | 2DIGIT                                                    |
// | year          | 4DIGIT                                                    |
// | time-of-day   | hour ":" minute ":" second                                |
// | hour          | 2DIGIT                                                    |
// | minute        | 2DIGIT                                                    |
// | second        | 2DIGIT                                                    |
// | GMT           | %s"GMT"                                                   |
// | day-name      | %s"Mon" / %s"Tue" / %s"Wed" / %s"Thu" / %s"Fri" /         |
// |               | %s"Sat" / %s"Sun"                                         |
// | day-name-l    | %s"Monday" / %s"Tuesday" / %s"Wednesday" /                |
// |               | %s"Thursday" / %s"Friday" / %s"Saturday" / %s"Sunday"     |
// | month         | %s"Jan" / %s"Feb" / %s"Mar" / %s"Apr" / %s"May" /         |
// |               | %s"Jun" / %s"Jul" / %s"Aug" / %s"Sep" / %s"Oct" /         |
// |               | %s"Nov" / %s"Dec"                                         |
// | DIGIT         | %x30-39                                                   |
// | SP            | %x20                                                      |
// +---------------------------------------------------------------------------+
// | Notes                                                                     |
// +---------------------------------------------------------------------------+
// | Senders SHOULD generate dates in IMF-fixdate format. Recipients MUST      |
// | accept all three HTTP-date formats: IMF-fixdate, rfc850-date, and         |
// | asctime-date.                                                             |
// |                                                                           |
// | delay-seconds is syntactically 1*DIGIT. The ABNF does not impose an upper |
// | bound; overflow handling is an implementation concern.                    |
// |                                                                           |
// | Retry-After is not a list field. Commas are only valid as part of the     |
// | HTTP-date alternatives where the grammar requires them.                   |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class retry_after {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    return consume_delay_seconds(sv) || date::check(sv);
  }

 private:
  // +=========================================================================+
  // | [>] consume_delay_seconds                                   ( private ) |
  // +=========================================================================+
  static constexpr bool consume_delay_seconds(std::string_view sv) {
    return helpers::is_digits(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
