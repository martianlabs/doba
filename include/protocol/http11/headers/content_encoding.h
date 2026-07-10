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

#ifndef martianlabs_doba_protocol_http11_headers_content_encoding_h
#define martianlabs_doba_protocol_http11_headers_content_encoding_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                          content-encoding |
// +===========================================================================+
// | RFC 9110 §8.4 Content-Encoding                                            |
// +---------------------------------------------------------------------------+
// | The "Content-Encoding" header field indicates which content codings have  |
// | been applied to a representation beyond those inherent in its media type. |
// | It therefore identifies the decoding mechanisms required to obtain the    |
// | data in the media type indicated by Content-Type.                         |
// |                                                                           |
// | Content-Encoding is primarily used to compress or otherwise transform     |
// | representation data without changing the identity of its underlying media |
// | type.                                                                     |
// |                                                                           |
// | When one or more codings have been applied, the sender MUST generate a    |
// | Content-Encoding field listing them in the order in which they were       |
// | applied. A recipient decodes them in the reverse order.                   |
// |                                                                           |
// | For example:                                                              |
// |                                                                           |
// |   Content-Encoding: gzip                                                  |
// |   Content-Encoding: deflate, gzip                                         |
// |                                                                           |
// | In the second example, "deflate" was applied first and "gzip" second. The |
// | recipient therefore removes "gzip" first and then "deflate".              |
// |                                                                           |
// | The coding name "identity" is reserved for its special role in            |
// | Accept-Encoding and SHOULD NOT be included in Content-Encoding.           |
// |                                                                           |
// | Content-Encoding differs from Transfer-Encoding. Content codings are a    |
// | characteristic of the representation itself, whereas transfer codings are |
// | used for message transfer and framing. Representation metadata generally  |
// | describes the coded representation unless otherwise specified.            |
// |                                                                           |
// | A coding that is inherent in the media type is not repeated in            |
// | Content-Encoding. It is listed only when that coding has been applied as  |
// | an additional transformation.                                             |
// |                                                                           |
// | An origin server MAY respond with 415 (Unsupported Media Type) when a     |
// | request representation uses a content coding that the server cannot       |
// | accept.                                                                   |
// |                                                                           |
// | Content-coding names are case-insensitive and ought to be registered in   |
// | the HTTP Content Coding Registry. RFC 9110 defines the following codings: |
// |                                                                           |
// |   * compress                                                              |
// |   * deflate                                                               |
// |   * gzip                                                                  |
// |                                                                           |
// | Recipients SHOULD treat "x-compress" as equivalent to "compress" and      |
// | "x-gzip" as equivalent to "gzip".                                         |
// +---------------------------------------------------------------------------+
// | RFC 9110 §8.4 and §8.4.1 (ABNF summary)                                   |
// +---------------------------------------------------------------------------+
// +------------------+--------------------------------------------------------+
// | Field            | Definition                                             |
// +------------------+--------------------------------------------------------+
// | Content-Encoding | #content-coding                                        |
// | content-coding   | token                                                  |
// | token            | 1*tchar                                                |
// | tchar            | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" /  |
// |                  | "." / "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA      |
// | OWS              | *( SP / HTAB )                                         |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.1 list expansion                                            |
// +---------------------------------------------------------------------------+
// | Sender syntax:                                                            |
// |                                                                           |
// |   #content-coding =                                                       |
// |       [ content-coding *( OWS "," OWS content-coding ) ]                  |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   #content-coding =                                                       |
// |       [ content-coding ]                                                  |
// |       *( OWS "," OWS [ content-coding ] )                                 |
// |                                                                           |
// | Senders MUST NOT generate empty list elements. Recipients MUST parse and  |
// | ignore a reasonable number of empty list elements.                        |
// |                                                                           |
// | Since the rule is "#content-coding", rather than "1#content-coding", zero |
// | non-empty content-coding elements are permitted by the purely syntactic   |
// | ABNF.                                                                     |
// |                                                                           |
// | Parameters are not allowed by this grammar. Each list element consists    |
// | exclusively of a non-empty token.                                         |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class content_encoding {
 public:
  static constexpr bool check(std::string_view sv) {
    return helpers::for_each_list_element(sv, consume_content_coding);
  }

 private:
  // +=========================================================================+
  // | [>] consume_content_coding                                  ( private ) |
  // +=========================================================================+
  static constexpr bool consume_content_coding(std::string_view sv) {
    return helpers::is_token(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
