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

#ifndef martianlabs_doba_protocol_http11_headers_via_h
#define martianlabs_doba_protocol_http11_headers_via_h

#include "protocol/http11/connection.h"
#include "protocol/http11/helpers.h"
#include "protocol/http11/parsed_types.h"
#include "protocol/http11/policies.h"
#include "protocol/http11/verdict.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                                       via |
// +===========================================================================+
// | RFC 9110 §7.6.3 Via                                                       |
// +---------------------------------------------------------------------------+
// | The "Via" header field indicates the presence of intermediate protocols   |
// | and recipients between the user agent and the server on requests, or      |
// | between the origin server and the client on responses.                    |
// |                                                                           |
// | Examples:                                                                 |
// |   Via: 1.0 fred, 1.1 p.example.net                                        |
// |   Via: HTTP/1.1 proxy.example.com (proxy software)                        |
// +---------------------------------------------------------------------------+
// | RFC 9110 §7.6.3 Via (ABNF summary)                                        |
// +---------------------------------------------------------------------------+
// +-------------------+-------------------------------------------------------+
// | Field             | Definition                                            |
// +-------------------+-------------------------------------------------------+
// | Via               | #( received-protocol RWS received-by [ RWS comment ] )|
// | received-protocol | [ protocol-name "/" ] protocol-version                |
// | received-by       | pseudonym [ ":" port ]                                |
// | pseudonym         | token                                                 |
// | port              | *DIGIT                                                |
// | comment           | "(" *( ctext / quoted-pair / comment ) ")"            |
// +-------------------+-------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class via {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static bool check(std::string_view sv, parsed_via_list& out) {
    // The producer overload validates each via-member exactly as the pure
    // check() does and captures its received-protocol, received-by, and
    // optional comment for every non-empty element, in order.
    return helpers::for_each_list_element(sv, [&out](std::string_view element) {
      parsed_via_element parsed;
      if (!consume_via_member(element, &parsed)) return false;
      out.elements.push_back(parsed);
      return true;
    });
  }
  // +=========================================================================+
  // | [>] interpret                                                ( public ) |
  // +=========================================================================+
  static constexpr verdict interpret(const parsed_via_list& parsed_list,
                                     http11::connection&,
                                     const policies& policies) {
    if (policies.max_forwarding_hops != 0 &&
        parsed_list.elements.size() > policies.max_forwarding_hops) {
      return verdict::kReject;
    }
    return verdict::kAccept;
  }

 private:
  // +=========================================================================+
  // | [>] consume_via_member                                      ( private ) |
  // +=========================================================================+
  // | via-member = received-protocol RWS received-by [ RWS comment ]          |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_via_member(std::string_view sv,
                                           parsed_via_element* out = nullptr) {
    std::size_t off = 0;
    // received-protocol = [ protocol-name "/" ] protocol-version.
    const std::size_t protocol_start = off;
    if (!consume_received_protocol(sv, off)) return false;
    if (out)
      out->received_protocol = sv.substr(protocol_start, off - protocol_start);
    // RWS = 1*( SP / HTAB ) is mandatory between received-protocol and
    // received-by.
    if (!consume_rws(sv, off)) return false;
    // received-by = pseudonym [ ":" port ].
    const std::size_t received_by_start = off;
    if (!consume_received_by(sv, off)) return false;
    if (out)
      out->received_by = sv.substr(received_by_start, off - received_by_start);
    if (off >= sv.size()) return true;
    // [ RWS comment ] is optional, but when a comment is present it MUST be
    // preceded by RWS.
    if (!consume_rws(sv, off)) return false;
    const std::string_view comment = helpers::consume_comment(sv.substr(off));
    if (comment.empty()) return false;
    if (out) out->comment = comment;
    off += comment.size();
    return off == sv.size();
  }
  // +=========================================================================+
  // | [>] consume_received_protocol                               ( private ) |
  // +=========================================================================+
  static constexpr bool consume_received_protocol(std::string_view sv,
                                                  std::size_t& off) {
    // A leading token is either protocol-version, or protocol-name when it is
    // followed by "/".
    const std::string_view first = helpers::consume_token(sv.substr(off));
    if (first.empty()) return false;
    off += first.size();
    // No "/" means the token was protocol-version; received-protocol is done.
    if (off >= sv.size() || sv[off] != '/') return true;
    // A "/" means the token was protocol-name; protocol-version is mandatory.
    ++off;
    const std::string_view version = helpers::consume_token(sv.substr(off));
    if (version.empty()) return false;
    off += version.size();
    return true;
  }
  // +=========================================================================+
  // | [>] consume_received_by                                     ( private ) |
  // +=========================================================================+
  static constexpr bool consume_received_by(std::string_view sv,
                                            std::size_t& off) {
    // pseudonym is a token (never uri-host / IP-literal); it is mandatory.
    const std::string_view pseudonym = helpers::consume_token(sv.substr(off));
    if (pseudonym.empty()) return false;
    off += pseudonym.size();
    // An optional ":" introduces port = *DIGIT (zero or more digits allowed).
    if (off >= sv.size() || sv[off] != ':') return true;
    ++off;
    while (off < sv.size() && helpers::is_digit(sv[off])) ++off;
    return true;
  }
  // +=========================================================================+
  // | [>] consume_rws                                             ( private ) |
  // +=========================================================================+
  // | RWS = 1*( SP / HTAB ). Consumes at least one whitespace octet, returning|
  // | false when none is present.                                             |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_rws(std::string_view sv, std::size_t& off) {
    const std::size_t rws_start = off;
    while (off < sv.size() && helpers::is_ows(sv[off])) ++off;
    return off != rws_start;
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
