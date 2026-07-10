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

#ifndef martianlabs_doba_protocol_http11_headers_if_unmodified_since_h
#define martianlabs_doba_protocol_http11_headers_if_unmodified_since_h

#include <ranges>
#include <string_view>

#include "protocol/http11/headers/date.h"
#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                       if-unmodified-since |
// +===========================================================================+
// | RFC 9110 §13.1.4 If-Unmodified-Since                                      |
// +---------------------------------------------------------------------------+
// | The "If-Unmodified-Since" header field makes a request conditional on the |
// | selected representation's last modification date being earlier than or    |
// | equal to the date provided in the field value.                            |
// |                                                                           |
// | It serves the same purpose as If-Match when the user agent does not have  |
// | an entity tag for the representation.                                     |
// |                                                                           |
// | A recipient MUST ignore If-Unmodified-Since when the request contains     |
// | If-Match, because If-Match is considered the more accurate precondition.  |
// |                                                                           |
// | A recipient MUST also ignore the field when its value is not a valid      |
// | HTTP-date, including when it appears to contain a list of dates, or when  |
// | no modification date is available for the resource.                       |
// |                                                                           |
// | The timestamp MUST be interpreted according to the origin server's clock. |
// |                                                                           |
// | If-Unmodified-Since is commonly used with state-changing methods such as  |
// | POST, PUT, and DELETE to prevent accidental overwrites and the "lost      |
// | update" problem.                                                          |
// |                                                                           |
// | When If-Match is absent, an origin server receiving this field MUST       |
// | evaluate the precondition before performing the method:                   |
// |                                                                           |
// | 1. If the selected representation's last modification date is earlier     |
// |    than or equal to the supplied date, the condition is true.             |
// |                                                                           |
// | 2. Otherwise, the condition is false.                                     |
// |                                                                           |
// | If the condition is false, the origin server MUST NOT perform the         |
// | requested method. It MAY respond with 412 (Precondition Failed), or with  |
// | a 2xx response when a state-changing request appears to have already been |
// | applied.                                                                  |
// |                                                                           |
// | A client MAY use If-Unmodified-Since with GET to prefer a 412 response    |
// | when the representation has changed. This is mainly useful for completing |
// | a partial representation; If-Range is better when the client prefers the  |
// | current complete representation.                                          |
// |                                                                           |
// | A cache or intermediary MAY ignore If-Unmodified-Since because its        |
// | interoperability behavior is only necessary for an origin server.         |
// |                                                                           |
// | Example:                                                                  |
// |   If-Unmodified-Since: Sat, 29 Oct 1994 19:43:31 GMT                      |
// +---------------------------------------------------------------------------+
// | RFC 9110 §13.1.4 If-Unmodified-Since (ABNF summary)                       |
// +---------------------------------------------------------------------------+
// +---------------------+-----------------------------------------------------+
// |Field                |Definition                                           |
// +---------------------+-----------------------------------------------------+
// |If-Unmodified-Since  |HTTP-date                                            |
// |HTTP-date            |IMF-fixdate / obs-date                               |
// |IMF-fixdate          |day-name "," SP date1 SP time-of-day SP GMT          |
// |day-name             |%s"Mon" / %s"Tue" / %s"Wed" / %s"Thu" /              |
// |                     |%s"Fri" / %s"Sat" / %s"Sun"                          |
// |date1                |day SP month SP year                                 |
// |day                  |2DIGIT                                               |
// |month                |%s"Jan" / %s"Feb" / %s"Mar" / %s"Apr" /              |
// |                     |%s"May" / %s"Jun" / %s"Jul" / %s"Aug" /              |
// |                     |%s"Sep" / %s"Oct" / %s"Nov" / %s"Dec"                |
// |year                 |4DIGIT                                               |
// |time-of-day          |hour ":" minute ":" second                           |
// |hour                 |2DIGIT                                               |
// |minute               |2DIGIT                                               |
// |second               |2DIGIT                                               |
// |GMT                  |%s"GMT"                                              |
// |obs-date             |rfc850-date / asctime-date                           |
// |rfc850-date          |day-name-l "," SP date2 SP time-of-day SP GMT        |
// |day-name-l           |%s"Monday" / %s"Tuesday" / %s"Wednesday" /           |
// |                     |%s"Thursday" / %s"Friday" / %s"Saturday" /           |
// |                     |%s"Sunday"                                           |
// |date2                |day "-" month "-" 2DIGIT                             |
// |asctime-date         |day-name SP date3 SP time-of-day SP year             |
// |date3                |month SP ( 2DIGIT / ( SP 1DIGIT ) )                  |
// |SP                   |%x20                                                 |
// |DIGIT                |%x30-39                                              |
// +---------------------+-----------------------------------------------------+
// | RFC 9110 §5.6.7 Date/Time requirements                                    |
// +---------------------------------------------------------------------------+
// | A sender MUST generate HTTP-date values using IMF-fixdate.                |
// |                                                                           |
// | A recipient parsing an HTTP-date MUST accept IMF-fixdate, rfc850-date,    |
// | and asctime-date.                                                         |
// |                                                                           |
// | HTTP-date is case-sensitive. A sender MUST NOT generate whitespace beyond |
// | the exact SP characters required by the grammar.                          |
// |                                                                           |
// | If-Unmodified-Since contains exactly one HTTP-date; it is not a           |
// | list-valued field.                                                        |
// |                                                                           |
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class if_unmodified_since {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) { return date::check(sv); }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
