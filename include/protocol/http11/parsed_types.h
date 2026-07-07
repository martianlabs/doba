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


#ifndef martianlabs_doba_protocol_http11_parsed_types_h
#define martianlabs_doba_protocol_http11_parsed_types_h

#include <string_view>
#include <vector>

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parsed types                                                          |
// +---------------------------------------------------------------------------+
// | The structures below are the neutral, protocol-agnostic products of the   |
// | syntactic layer. A syntactic checker's producer overload                  |
// | (check(std::string_view, parsed_T&)) validates a field value exactly as   |
// | the pure check(std::string_view) overload does and, on success, fills the |
// | matching parsed_T so that the semantic layer never has to re-parse.       |
// |                                                                           |
// | Every member is a zero-copy std::string_view (or a std::vector of them)   |
// | that points back into the original field-value buffer; the buffer MUST    |
// | outlive any parsed_T that references it. No normalization, decoding, or   |
// | ownership transfer happens here: interpretation is the semantic layer's   |
// | responsibility.                                                           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parsed_host_port                                            ( struct ) |
// +---------------------------------------------------------------------------+
// | RFC 3986 §3.2.2 / §3.2.3 — host [ ":" port ]                              |
// +---------------------------------------------------------------------------+
// | The zero-copy product of a "host [ ":" port ]" value (the shape shared by  |
// | the Host header and an authority-form request-target). "host" is the      |
// | uri-host substring (bracketed colons of an IP-literal are kept intact);   |
// | "type" classifies it; "port" is the raw *DIGIT run after the ":" (empty   |
// | when no port is present). Both "host" and "port" may be empty because     |
// | reg-name and port each use the "*" repetition operator.                   |
// | "scheme" carries the absolute-form request-target scheme (e.g. "http") so |
// | routing can normalize an omitted port against the scheme default; it is   |
// | empty for the Host header and for authority-form targets, which carry no  |
// | scheme.                                                                   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct parsed_host_port {
  std::string_view host;
  std::string_view port;
  helpers::host_type type = helpers::host_type::kUnknown;
  std::string_view scheme;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parsed_token_list                                          ( struct ) |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.1 — #token                                                  |
// +---------------------------------------------------------------------------+
// | The zero-copy product of a comma-separated token list (the shape shared   |
// | by Connection, Trailer, Upgrade, and X-Forwarded-Proto). "elements" holds |
// | every non-empty list element, already OWS-trimmed and in the order they   |
// | appeared. An empty list yields an empty "elements" vector.                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct parsed_token_list {
  std::vector<std::string_view> elements;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parsed_parameter_list                                      ( struct ) |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.1 / §5.6.6 — #( token *( ";" parameter ) )                  |
// +---------------------------------------------------------------------------+
// | The zero-copy product of a comma-separated list whose elements each carry |
// | an optional set of ";"-separated parameters (the shape shared by          |
// | Transfer-Encoding and TE). Every element keeps its raw text, including    |
// | any parameters, so the semantic layer decides which parameters to read.   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct parsed_parameter_list {
  std::vector<std::string_view> elements;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parsed_scalar                                              ( struct ) |
// +---------------------------------------------------------------------------+
// | The zero-copy product of a single scalar field value (the shape shared by |
// | Expect and, before numeric conversion, Max-Forwards). "value" is the raw  |
// | field-value with no OWS around it, exactly as validated by the checker.   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct parsed_scalar {
  std::string_view value;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parsed_via_element                                         ( struct ) |
// +---------------------------------------------------------------------------+
// | RFC 9110 §7.6.3 — Via                                                     |
// +---------------------------------------------------------------------------+
// | received-protocol = [ protocol-name "/" ] protocol-version                |
// | received-by       = pseudonym / ( uri-host [ ":" port ] )                 |
// +---------------------------------------------------------------------------+
// | The zero-copy product of a single Via list element. "received_protocol"   |
// | and "received_by" are the raw substrings of the two mandatory parts, and  |
// | "comment" is the optional trailing comment (empty when absent).           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct parsed_via_element {
  std::string_view received_protocol;
  std::string_view received_by;
  std::string_view comment;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parsed_via_list                                            ( struct ) |
// +---------------------------------------------------------------------------+
// | RFC 9110 §7.6.3 — 1#( received-protocol RWS received-by [ RWS comment ] ) |
// +---------------------------------------------------------------------------+
// | The zero-copy product of a Via header: every list element in order.       |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct parsed_via_list {
  std::vector<parsed_via_element> elements;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parsed_forwarded_pair                                      ( struct ) |
// +---------------------------------------------------------------------------+
// | RFC 7239 §4 — forwarded-pair = token "=" value                            |
// +---------------------------------------------------------------------------+
// | The zero-copy product of a single Forwarded name=value pair. "name" is    |
// | the raw parameter token and "value" is the raw token or quoted-string     |
// | exactly as it appeared (quotes and escapes are preserved).                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct parsed_forwarded_pair {
  std::string_view name;
  std::string_view value;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parsed_forwarded_element                                   ( struct ) |
// +---------------------------------------------------------------------------+
// | RFC 7239 §4 — forwarded-element = [ pair ] *( ";" [ pair ] )              |
// +---------------------------------------------------------------------------+
// | The zero-copy product of a single Forwarded list element: its ";"-        |
// | separated name=value pairs, in order.                                     |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct parsed_forwarded_element {
  std::vector<parsed_forwarded_pair> pairs;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parsed_forwarded_list                                      ( struct ) |
// +---------------------------------------------------------------------------+
// | RFC 7239 §4 — Forwarded = 1#forwarded-element                             |
// +---------------------------------------------------------------------------+
// | The zero-copy product of a Forwarded header: every list element in order. |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct parsed_forwarded_list {
  std::vector<parsed_forwarded_element> elements;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parsed_host_port_list                                      ( struct ) |
// +---------------------------------------------------------------------------+
// | de-facto X-Forwarded-Host — #( uri-host [ ":" port ] )                    |
// +---------------------------------------------------------------------------+
// | The zero-copy product of a comma-separated list of "host [ ":" port ]"    |
// | items (the shape of X-Forwarded-Host), each already parsed into its       |
// | host/port parts.                                                          |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct parsed_host_port_list {
  std::vector<parsed_host_port> elements;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
