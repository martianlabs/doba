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

#ifndef martianlabs_doba_protocol_http11_checkers_connection_h
#define martianlabs_doba_protocol_http11_checkers_connection_h

#include <ranges>
#include <string_view>

#include "protocol/http11/constants.h"
#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::checkers {
// =============================================================================
// |                                                            [ connection ] |
// +---------------------------------------------------------------------------+
// | RFC 9110 §7.6.1 - Connection                                              |
// +---------------------------------------------------------------------------+
// | The "Connection" header field provides a means for communicating control  |
// | information for the current connection. It is primarily used to indicate  |
// | options that are desired for that particular connection and that are not  |
// | to be communicated forward by proxies.                                    |
// |                                                                           |
// | ABNF:                                                                     |
// |     Connection = #connection-option                                       |
// |     connection-option = token                                             |
// |                                                                           |
// | A proxy or gateway MUST remove or replace any header fields nominated by  |
// | the Connection header field before forwarding the message.                |
// | In other words, if a header field is listed in the Connection header      |
// | field, then it is to be removed from the message before forwarding it     |
// | to the next recipient.                                                    |
// | Any HTTP/1.1 message containing a Connection header field MUST include at |
// | least one connection option.                                              |
// |                                                                           |
// | The Connection header field's value has no meaning in HTTP/2 or HTTP/3.   |
// | A sender MUST NOT generate a message containing connection options for    |
// | those versions, and a recipient MUST treat messages that contain          |
// | connection options as malformed.                                          |
// |                                                                           |
// | Connection options are case-insensitive. If a proxy receives a            |
// | message with a Connection header field, it MUST remove that header field  |
// | and any header fields nominated by it before forwarding the message.      |
// |                                                                           |
// | For example:                                                              |
// |     Connection: keep-alive                                                |
// | would allow the sender to indicate that it desires a                      |
// | persistent connection.                                                    |
// |                                                                           |
// | When a client sends a Connection header field with the "close" connection |
// | option, it is indicating that it wants to close the connection after this |
// | request is complete.                                                      |
// |                                                                           |
// | When a server sends a Connection header field with the "close" connection |
// | option, it is indicating that it will close the connection                |
// | after delivering this response.                                           |
// |                                                                           |
// | The connection options do not have to correspond to a header              |
// | field present in the message, since a connection option might be used to  |
// | indicate other features or options (such as "close") that are             |
// | not implemented as header fields.                                         |
// |                                                                           |
// | A sender MUST NOT include a connection option corresponding to a header   |
// | field that is defined as being connection-specific, such as               |
// | Transfer-Encoding, since such header fields are always implicitly         |
// | hop-by-hop.                                                               |
// |                                                                           |
// | A sender MUST NOT include a connection option for a header field that is  |
// | not in the message because the option would have no effect.               |
// | However, a proxy that receives a message with such a connection           |
// | option MAY delete it before forwarding the message.                       |
// +---------------------------------------------------------------------------+
// +---------------------+-----------------------------------------------------+
// | Field               | Definition                                          |
// +---------------------+-----------------------------------------------------+
// | Connection          | 1#connection-option                                 |
// | connection-option   | token                                               |
// +---------------------+-----------------------------------------------------+
// =============================================================================
static auto connection_check_fn = [](std::string_view v) -> bool {
  for (auto token : v | std::views::split(constants::character::kComma)) {
    std::string_view value(&*token.begin(), std::ranges::distance(token));
    value = helpers::ows_ltrim(helpers::ows_rtrim(value));
    if (!helpers::is_token(value)) return false;
  }
  return true;
};
}  // namespace martianlabs::doba::protocol::http11::checkers

#endif
