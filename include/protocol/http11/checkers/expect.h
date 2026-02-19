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

#ifndef martianlabs_doba_protocol_http11_checkers_expect_h
#define martianlabs_doba_protocol_http11_checkers_expect_h

#include "protocol/http11/constants.h"
#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::checkers {
// +===========================================================================+
// |                                                                    expect |
// +===========================================================================+
// | RFC 9110 (HTTP Semantics) - "Expect" header field (ABNF)                  |
// +---------------------------------------------------------------------------+
// |  Expect = #expectation                                                    |
// |  expectation = token [ "=" ( token / quoted-string ) parameters ]         |
// |                                                                           |
// | Notes (ABNF building blocks referenced above):                            |
// |                                                                           |
// |  Lists (#rule ABNF extension):                                            |
// |    1#element => element *( OWS "," OWS element )                          |
// |    #element  => [ element ] *( OWS "," OWS [ element ] )                  |
// |                                                                           |
// |  Tokens:                                                                  |
// |    token = 1*tchar                                                        |   
// |    tchar = "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+"                  |
// |          / "-" / "." / "^" / "_" / "`" / "|" / "~"                        |
// |          / DIGIT / ALPHA                                                  |
// |                                                                           |
// |  Quoted strings:                                                          |
// |    quoted-string = DQUOTE *( qdtext / quoted-pair ) DQUOTE                | 
// |    qdtext        = HTAB / SP / %x21 / %x23-5B / %x5D-7E / obs-text        |
// |    quoted-pair   = "\" ( HTAB / SP / VCHAR / obs-text )                   |
// |                                                                           |
// |  Parameters:                                                              |
// |    parameters      = *( OWS ";" OWS [ parameter ] )                       |
// |    parameter       = parameter-name "=" parameter-value                   | 
// |    parameter-name  = token                                                |
// |    parameter-value = ( token / quoted-string )                            |
// |                                                                           |
// | Additional rule stated next to the field definition:                      |
// |  - The Expect field value is case-insensitive.                            |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
static inline bool expect(std::string_view v) {
  for (auto token : v | std::views::split(constants::character::kComma)) {
    if (token.begin() == token.end()) continue;
    std::string_view value(&*token.begin(), std::ranges::distance(token));
    helpers::ows_trim(value);
    if (value.empty()) continue;
  }
  return true;
}
}  // namespace martianlabs::doba::protocol::http11::checkers

#endif
