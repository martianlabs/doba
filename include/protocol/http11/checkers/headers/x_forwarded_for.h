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

#ifndef martianlabs_doba_protocol_http11_checkers_h_x_forwarded_for_h
#define martianlabs_doba_protocol_http11_checkers_h_x_forwarded_for_h

#include "protocol/http11/helpers.h"
#include "protocol/http11/parsed_types.h"

namespace martianlabs::doba::protocol::http11::checkers::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                           x-forwarded-for |
// +===========================================================================+
// | De-facto X-Forwarded-For                                                  |
// +---------------------------------------------------------------------------+
// | The "X-Forwarded-For" header field is a de-facto request header used by   |
// | proxies, reverse proxies, load balancers, and gateways to identify the    |
// | originating client IP address and the forwarding chain seen by the HTTP   |
// | request.                                                                  |
// |                                                                           |
// | Each proxy that appends to the field normally adds the IP address of the  |
// | peer from which it received the request. The leftmost element is commonly |
// | interpreted as the original client address, while each following element  |
// | represents a subsequent proxy hop.                                        |
// |                                                                           |
// | X-Forwarded-For is not defined by an RFC and has no single normative ABNF.|
// | The standardized replacement is the "Forwarded" header field defined by   |
// | RFC 7239.                                                                 |
// |                                                                           |
// | Because this field can be supplied directly by a client, its contents MUST|
// | NOT be trusted unless the request was received through a trusted proxy    |
// | chain that sanitizes or overwrites untrusted incoming values.             |
// |                                                                           |
// | This checker treats the value as a comma-separated list of node           |
// | identifiers. A node is accepted as an IPv4 address, an IPv6 address, an   |
// | optional bracketed IP literal, or the literal "unknown".                  |
// |                                                                           |
// | Non-standard variants carrying ports, such as "203.0.113.1:12345" or      |
// | "[2001:db8::1]:12345", are intentionally not included in this strict      |
// | syntactic profile.                                                        |
// |                                                                           |
// | Examples:                                                                 |
// |   X-Forwarded-For: 203.0.113.195                                          |
// |   X-Forwarded-For: 203.0.113.195, 70.41.3.18, 150.172.238.178             |
// |   X-Forwarded-For: 2001:db8:85a3::8a2e:370:7334                           |
// |   X-Forwarded-For: unknown, 203.0.113.10                                  |
// +---------------------------------------------------------------------------+
// | Practical de-facto syntax (ABNF summary)                                  |
// +---------------------------------------------------------------------------+
// +-------------------+-------------------------------------------------------+
// | Field             | Definition                                            |
// +-------------------+-------------------------------------------------------+
// | X-Forwarded-For   | #xff-node                                             |
// | xff-node          | IPv4address / IPv6address / IP-literal / "unknown"    |
// | IP-literal        | "[" ( IPv6address / IPvFuture ) "]"                   |
// | IPvFuture         | "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )  |
// | IPv4address       | dec-octet "." dec-octet "." dec-octet "." dec-octet   |
// | dec-octet         | DIGIT / %x31-39 DIGIT / "1" 2DIGIT /                  |
// |                   | "2" %x30-34 DIGIT / "25" %x30-35                      |
// | IPv6address       | 6( h16 ":" ) ls32 / "::" 5( h16 ":" ) ls32 /          |
// |                   | [ h16 ] "::" 4( h16 ":" ) ls32 /                      |
// |                   | [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32 /        |
// |                   | [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32 /        |
// |                   | [ *3( h16 ":" ) h16 ] "::" h16 ":" ls32 /             |
// |                   | [ *4( h16 ":" ) h16 ] "::" ls32 /                     |
// |                   | [ *5( h16 ":" ) h16 ] "::" h16 /                      |
// |                   | [ *6( h16 ":" ) h16 ] "::"                            |
// | h16               | 1*4HEXDIG                                             |
// | ls32              | ( h16 ":" h16 ) / IPv4address                         |
// | unreserved        | ALPHA / DIGIT / "-" / "." / "_" / "~"                 |
// | sub-delims        | "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / ","   |
// |                   | / ";" / "="                                           |
// | OWS               | *( SP / HTAB )                                        |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.1 list expansion, applied as practical list syntax          |
// +---------------------------------------------------------------------------+
// | Sender syntax:                                                            |
// |                                                                           |
// |   #xff-node = [ xff-node *( OWS "," OWS xff-node ) ]                      |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   #xff-node = [ xff-node ] *( OWS "," OWS [ xff-node ] )                  |
// |                                                                           |
// | Senders SHOULD NOT generate empty list elements. Recipients MAY parse and |
// | ignore a reasonable number of empty list elements for robustness.         |
// |                                                                           |
// | Since the rule is "#xff-node", rather than "1#xff-node", zero non-empty   |
// | xff-node elements are permitted by this purely syntactic profile.         |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class x_forwarded_for {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static bool check(std::string_view sv, parsed_token_list& out) {
    // The producer overload validates each xff-node exactly as the pure
    // check() does and captures every non-empty node in order.
    return helpers::for_each_list_element(
        sv, [&out](std::string_view element) {
          if (!consume_xff_node(element)) return false;
          out.elements.push_back(element);
          return true;
        });
  }

 private:
  // +=========================================================================+
  // | [>] consume_xff_node                                        ( private ) |
  // +=========================================================================+
  // | xff-node = IPv4address / IPv6address / IP-literal / "unknown"           |
  // +-------------------------------------------------------------------------+
  // | Ports and general reg-name / host names are intentionally excluded from |
  // | this strict profile; IPvFuture is accepted only inside an IP-literal.   |
  // +=========================================================================+
  static constexpr bool consume_xff_node(std::string_view sv) {
    return helpers::is_ip_v4_address(sv) || helpers::is_ip_v6_address(sv) ||
           helpers::is_ip_literal(sv) || sv == "unknown";
  }
};
}  // namespace martianlabs::doba::protocol::http11::checkers::headers

#endif
