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

#ifndef martianlabs_doba_protocol_http11_headers_sec_websocket_version_h
#define martianlabs_doba_protocol_http11_headers_sec_websocket_version_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                     sec-websocket-version |
// +===========================================================================+
// | RFC 6455 §11.3.5 Sec-WebSocket-Version                                    |
// +---------------------------------------------------------------------------+
// | The "Sec-WebSocket-Version" header field is used in the WebSocket opening |
// | handshake to indicate the protocol version used by the client.            |
// |                                                                           |
// | A client MUST include this header field in its opening handshake. For RFC |
// | 6455, the value sent by the client MUST be "13".                          |
// |                                                                           |
// | If the server does not understand or does not support the version sent by |
// | the client, it MUST abort the WebSocket opening handshake and send an HTTP|
// | response with status code 426 (Upgrade Required). That response MUST      |
// | include a Sec-WebSocket-Version header field indicating the version, or   |
// | versions, that the server is capable of understanding.                    |
// |                                                                           |
// | The header field can appear multiple times in a server response,          |
// | or contain multiple version values separated by commas.                   |
// | Values can be repeated.                                                   |
// |                                                                           |
// | The version value is a decimal integer in the range 0 through 255, without|
// | leading zeroes except for the single value "0".                           |
// |                                                                           |
// | Examples:                                                                 |
// |   Sec-WebSocket-Version: 13                                               |
// |   Sec-WebSocket-Version: 13, 8, 7                                         |
// |   Sec-WebSocket-Version: 13                                               |
// |   Sec-WebSocket-Version: 8, 7                                             |
// +---------------------------------------------------------------------------+
// | RFC 6455 §11.3.5 Sec-WebSocket-Version (ABNF summary)                     |
// +---------------------------------------------------------------------------+
// +-----------------------+---------------------------------------------------+
// | Field                 | Definition                                        |
// +-----------------------+---------------------------------------------------+
// | Sec-WebSocket-Version | version                                           |
// | version               | DIGIT / ( NZDIGIT DIGIT ) /                       |
// |                       | ( "1" DIGIT DIGIT ) / ( "2" DIGIT DIGIT )         |
// | NZDIGIT               | "1" / "2" / "3" / "4" / "5" / "6" /               |
// |                       | "7" / "8" / "9"                                   |
// | DIGIT                 | "0" / "1" / "2" / "3" / "4" / "5" /               |
// |                       | "6" / "7" / "8" / "9"                             |
// +---------------------------------------------------------------------------+
// | Numeric range constraint                                                  |
// +---------------------------------------------------------------------------+
// | The ABNF shape admits 0..299 syntactically, but RFC 6455 constrains       |
// | version to the range 0..255 and forbids leading zeroes. Therefore, values |
// | such as "256", "299", "013", and "00" are invalid.                        |
// |                                                                           |
// | In RFC 6455-compliant client handshakes, the only valid protocol version  |
// | to send is "13".                                                          |
// +---------------------------------------------------------------------------+
// | HTTP list handling                                                        |
// +---------------------------------------------------------------------------+
// | RFC 6455 permits a server response to carry multiple supported versions   |
// | either as repeated Sec-WebSocket-Version field lines or as comma-separated|
// | values in one field line. After HTTP field-line combination, the effective|
// | field value can therefore be parsed as a comma-separated list of version  |
// | values.                                                                   |
// |                                                                           |
// | Sender syntax:                                                            |
// |                                                                           |
// |   #version = [ version *( OWS "," OWS version ) ]                         |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   #version = [ version ] *( OWS "," OWS [ version ] )                     |
// |                                                                           |
// | Senders MUST NOT generate empty list elements. Recipients MUST parse and  |
// | ignore a reasonable number of empty list elements.                        |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class sec_websocket_version {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    return helpers::for_each_list_element(sv, consume_version);
  }

 private:
  // +=========================================================================+
  // | [>] consume_version                                         ( private ) |
  // +=========================================================================+
  // | version = a decimal integer in 0..255 with no leading zeroes (except    |
  // | "0"). Although the raw ABNF shape admits 0..299, RFC 6455 constrains it |
  // | to 0..255, which is exactly a canonical RFC 3986 dec-octet.             |
  // +=========================================================================+
  static constexpr bool consume_version(std::string_view sv) {
    return helpers::is_dec_octet(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
