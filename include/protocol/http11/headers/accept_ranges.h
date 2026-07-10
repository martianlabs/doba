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

#ifndef martianlabs_doba_protocol_http11_headers_accept_ranges_h
#define martianlabs_doba_protocol_http11_headers_accept_ranges_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                             accept-ranges |
// +===========================================================================+
// | RFC 9110 §14.3 Accept-Ranges                                              |
// +---------------------------------------------------------------------------+
// | The "Accept-Ranges" response header field indicates whether an upstream   |
// | server supports range requests for the target resource.                   |
// |                                                                           |
// | A server that supports byte-range requests can advertise that support by  |
// | sending:                                                                  |
// |                                                                           |
// |   Accept-Ranges: bytes                                                    |
// |                                                                           |
// | This information can encourage clients to use range requests for future   |
// | transfers of the same resource, improving performance and avoiding the    |
// | unnecessary retransmission of representation data.                        |
// |                                                                           |
// | A client MAY generate a range request regardless of whether it previously |
// | received an Accept-Ranges field. The field is advisory and does not grant |
// | or restrict the client's ability to send a Range request.                 |
// |                                                                           |
// | A client MUST NOT assume that receiving Accept-Ranges guarantees that a   |
// | future range request will produce a partial response. The representation  |
// | might change, server support might depend on current conditions, or a     |
// | different intermediary might process the subsequent request.              |
// |                                                                           |
// | A server that does not support any range requests for the target resource |
// | MAY advertise that fact using the reserved range unit "none":             |
// |                                                                           |
// |   Accept-Ranges: none                                                     |
// |                                                                           |
// | The "none" value is syntactically a range-unit token, but its reserved    |
// | meaning is defined by the semantics of Accept-Ranges.                     |
// |                                                                           |
// | Range-unit names are case-insensitive and ought to be registered in the   |
// | HTTP Range Unit Registry. The "bytes" range unit is defined by RFC 9110;  |
// | extension range units can also be advertised.                             |
// |                                                                           |
// | Accept-Ranges MAY be sent in a trailer section. Sending it as a header    |
// | field is preferred because the information is especially useful when a    |
// | large transfer fails before its trailer section is received.              |
// |                                                                           |
// | Examples:                                                                 |
// |   Accept-Ranges: bytes                                                    |
// |   Accept-Ranges: none                                                     |
// |   Accept-Ranges: bytes, example-unit                                      |
// +---------------------------------------------------------------------------+
// | RFC 9110 §14.3 Accept-Ranges (ABNF summary)                               |
// +---------------------------------------------------------------------------+
// +-------------------+-------------------------------------------------------+
// | Field             | Definition                                            |
// +-------------------+-------------------------------------------------------+
// | Accept-Ranges     | acceptable-ranges                                     |
// | acceptable-ranges | 1#range-unit                                          |
// | range-unit        | token                                                 |
// | token             | 1*tchar                                               |
// | tchar             | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" / |
// |                   | "." / "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA     |
// | OWS               | *( SP / HTAB )                                        |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.1 list expansion                                            |
// +---------------------------------------------------------------------------+
// | Sender syntax:                                                            |
// |                                                                           |
// |   1#range-unit = range-unit *( OWS "," OWS range-unit )                   |
// |                                                                           |
// | A sender MUST generate at least one range-unit and MUST NOT               |
// | generate empty list elements.                                             |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   1#range-unit = [ range-unit ]                                           |
// |                  *( OWS "," OWS [ range-unit ] )                          |
// |                                                                           |
// | A recipient MUST parse and ignore a reasonable number of empty list       |
// | elements. Empty elements do not contribute to the element count.          |
// |                                                                           |
// | Since Accept-Ranges uses "1#range-unit", at least one                     |
// | non-empty range-unit is still required after empty elements have          |
// | been ignored. Consequently, the following normalized values are invalid:  |
// |                                                                           |
// |   ""                                                                      |
// |   ","                                                                     |
// |   ",   ,"                                                                 |
// |                                                                           |
// | Values such as these are valid for a recipient:                           |
// |                                                                           |
// |   "bytes,"                                                                |
// |   ",bytes"                                                                |
// |   "bytes, ,example-unit"                                                  |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class accept_ranges {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    bool at_least_one_range_unit = false;
    const bool result = helpers::for_each_list_element(
        sv, [&at_least_one_range_unit](std::string_view element) {
          if (!consume_range_unit(element)) return false;
          at_least_one_range_unit = true;
          return true;
        });
    // 1#range-unit requires at least one valid range-unit.
    return result && at_least_one_range_unit;
  }

 private:
  // +=========================================================================+
  // | [>] consume_range_unit                                      ( private ) |
  // +=========================================================================+
  // | range-unit = token                                                      |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_range_unit(std::string_view sv) {
    return helpers::is_token(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
