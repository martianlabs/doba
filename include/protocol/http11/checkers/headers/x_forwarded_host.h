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

#ifndef martianlabs_doba_protocol_http11_checkers_h_x_forwarded_host_h
#define martianlabs_doba_protocol_http11_checkers_h_x_forwarded_host_h

#include "protocol/http11/helpers.h"
#include "protocol/http11/parsed_types.h"

namespace martianlabs::doba::protocol::http11::checkers::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                          x-forwarded-host |
// +===========================================================================+
// | Non-standard / de-facto X-Forwarded-Host                                  |
// +---------------------------------------------------------------------------+
// | The "X-Forwarded-Host" header field is a de-facto request header used by  |
// | proxies, reverse proxies, load balancers, and CDNs to preserve the        |
// | original Host value requested by the client before the request is         |
// | forwarded to an origin server.                                            |
// |                                                                           |
// | Unlike "Forwarded", X-Forwarded-Host is not standardized by an IETF RFC.  |
// | The standardized equivalent is the "host" parameter of the RFC 7239       |
// | "Forwarded" header field.                                                 |
// |                                                                           |
// | The field value identifies a host, optionally followed by a port. It does |
// | not include a URI scheme, userinfo, path, query, fragment, or quoted-     |
// | string syntax.                                                            |
// |                                                                           |
// | Examples:                                                                 |
// |   X-Forwarded-Host: example.com                                           |
// |   X-Forwarded-Host: example.com:8443                                      |
// |   X-Forwarded-Host: [2001:db8::1]:443                                     |
// |                                                                           |
// | This field is security-sensitive. Applications SHOULD only trust it when  |
// | it was added or sanitized by a trusted forwarding boundary.               |
// +---------------------------------------------------------------------------+
// | RFC 9110 §7.2 Host / RFC 3986 §3.2.2 host (ABNF summary)                  |
// +---------------------------------------------------------------------------+
// +------------------+--------------------------------------------------------+
// | Field            | Definition                                             |
// +------------------+--------------------------------------------------------+
// | X-Forwarded-Host | x-forwarded-host                                       |
// | x-forwarded-host | uri-host [ ":" port ]                                  |
// | uri-host         | IP-literal / IPv4address / reg-name                    |
// | IP-literal       | "[" ( IPv6address / IPvFuture ) "]"                    |
// | IPvFuture        | "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )   |
// | IPv6address      | <IPv6address, see RFC 3986 §3.2.2>                     |
// | IPv4address      | dec-octet "." dec-octet "." dec-octet "." dec-octet    |
// | dec-octet        | DIGIT / %x31-39 DIGIT / "1" 2DIGIT /                   |
// |                  | "2" %x30-34 DIGIT / "25" %x30-35                       |
// | reg-name         | *( unreserved / pct-encoded / sub-delims )             |
// | port             | *DIGIT                                                 |
// | pct-encoded      | "%" HEXDIG HEXDIG                                      |
// | unreserved       | ALPHA / DIGIT / "-" / "." / "_" / "~"                  |
// | sub-delims       | "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" /        |
// |                  | "," / ";" / "="                                        |
// | OWS              | *( SP / HTAB )                                         |
// +------------------+--------------------------------------------------------+
// +---------------------------------------------------------------------------+
// | Parsing notes                                                             |
// +---------------------------------------------------------------------------+
// | This field is not a list field. Do not apply RFC 9110 "#rule" list        |
// | expansion and do not split on comma as part of the ABNF for this header.  |
// |                                                                           |
// | The grammar intentionally mirrors the Host field value: uri-host [ ":"    |
// | port ]. As a consequence, the URI reg-name grammar permits comma as a     |
// | sub-delimiter.                                                            |
// |                                                                           |
// | Multiple X-Forwarded-Host field lines, or comma-combined values produced  |
// | by generic field-line merging, are outside this de-facto field syntax and |
// | should be handled by local policy.                                        |
// |                                                                           |
// | The purely syntactic URI host grammar allows an empty reg-name. A strict  |
// | implementation for this header may choose to reject an empty value as a   |
// | semantic policy decision.                                                 |
// |                                                                           |
// | IMPORTANT: field-value is supposed to be normalized (no OWS around        |
// | value).                                                                   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class x_forwarded_host {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv, parsed_host_port& out) {
    // The producer overload validates exactly as the pure check() does and, on
    // success, fills the parsed uri-host, port, and host type through the same
    // shared helpers::check_host_port so both paths never diverge.
    return helpers::check_host_port(sv, out.host, out.port, out.type);
  }
};
}  // namespace martianlabs::doba::protocol::http11::checkers::headers

#endif
