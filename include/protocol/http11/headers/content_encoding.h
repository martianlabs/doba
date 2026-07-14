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
// Copyright 2025 martianLabs
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied. See the License for the specific language governing
// permissions and limitations under the License.

#ifndef martianlabs_doba_protocol_http11_headers_content_encoding_h
#define martianlabs_doba_protocol_http11_headers_content_encoding_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                          content-encoding |
// +===========================================================================+
// | RFC 9110 �8.4 Content-Encoding                                            |
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
// | RFC 9110 �8.4 and �8.4.1 (ABNF summary)                                   |
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
// | RFC 9110 �5.6.1 list expansion                                            |
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
