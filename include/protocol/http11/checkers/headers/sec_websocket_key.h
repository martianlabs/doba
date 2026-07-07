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

#ifndef martianlabs_doba_protocol_http11_checkers_h_sec_websocket_key_h
#define martianlabs_doba_protocol_http11_checkers_h_sec_websocket_key_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::checkers::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                         sec-websocket-key |
// +===========================================================================+
// | RFC 6455 §11.3.1 Sec-WebSocket-Key                                        |
// +---------------------------------------------------------------------------+
// | The "Sec-WebSocket-Key" header field is used in the WebSocket opening     |
// | handshake. It is sent from the client to the server to provide part of    |
// | the information used by the server to prove that it received a valid      |
// | WebSocket opening handshake. This helps ensure that the server does not   |
// | accept connections from non-WebSocket clients that are being abused to    |
// | send data to unsuspecting WebSocket servers.                              |
// |                                                                           |
// | A client handshake request MUST include a Sec-WebSocket-Key header field. |
// | Its value MUST be a nonce consisting of a randomly selected 16-byte value |
// | that has been base64-encoded. The nonce MUST be selected randomly for     |
// | each connection.                                                          |
// |                                                                           |
// | The Sec-WebSocket-Key header field MUST NOT appear more than once in an   |
// | HTTP request.                                                             |
// |                                                                           |
// | The server uses the value as present in the header field, excluding any   |
// | leading or trailing whitespace, when constructing Sec-WebSocket-Accept.   |
// | Specifically, the server concatenates the encoded Sec-WebSocket-Key value |
// | with the WebSocket GUID:                                                  |
// |                                                                           |
// |   258EAFA5-E914-47DA-95CA-C5AB0DC85B11                                    |
// |                                                                           |
// | It then computes SHA-1 over that concatenated string and base64-encodes   |
// | the resulting 20-byte hash.                                               |
// |                                                                           |
// | For constructing Sec-WebSocket-Accept, the server does not base64-decode  |
// | Sec-WebSocket-Key. However, a valid WebSocket opening handshake requires  |
// | the Sec-WebSocket-Key value to be base64 data that decodes to exactly     |
// | 16 bytes.                                                                 |
// |                                                                           |
// | Examples:                                                                 |
// |   Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==                             |
// +---------------------------------------------------------------------------+
// | RFC 6455 §4.3 Sec-WebSocket-Key (ABNF summary)                            |
// +---------------------------------------------------------------------------+
// +------------------------+--------------------------------------------------+
// | Field                  | Definition                                       |
// +------------------------+--------------------------------------------------+
// | Sec-WebSocket-Key      | base64-value-non-empty                           |
// | base64-value-non-empty | ( 1*base64-data [ base64-padding ] ) /           |
// |                        | base64-padding                                   |
// | base64-data            | 4base64-character                                |
// | base64-padding         | ( 2base64-character "==" ) /                     |
// |                        | ( 3base64-character "=" )                        |
// | base64-character       | ALPHA / DIGIT / "+" / "/"                        |
// | ALPHA                  | %x41-5A / %x61-7A                                |
// | DIGIT                  | %x30-39                                          |
// +---------------------------------------------------------------------------+
// | RFC 6455 §4.1 Client handshake semantic constraints                       |
// +---------------------------------------------------------------------------+
// | The decoded Sec-WebSocket-Key value MUST be exactly 16 bytes long.        |
// | The nonce MUST be randomly selected for each connection.                  |
// |                                                                           |
// | The ABNF only validates the base64-shaped syntax. The 16-byte decoded     |
// | length and per-connection randomness are semantic handshake requirements. |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class sec_websocket_key {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // Sec-WebSocket-Key = base64-value-non-empty. Only the base64-shaped
    // syntax is validated here; the 16-byte decoded length and per-connection
    // randomness are semantic handshake requirements, not part of the ABNF.
    return helpers::check_base64_value(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::checkers::headers

#endif
