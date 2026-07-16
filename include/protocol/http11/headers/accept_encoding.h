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

#ifndef martianlabs_doba_protocol_http11_headers_accept_encoding_h
#define martianlabs_doba_protocol_http11_headers_accept_encoding_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                           accept-encoding |
// +===========================================================================+
// | RFC 9110 �12.5.3 Accept-Encoding                                          |
// +---------------------------------------------------------------------------+
// | The "Accept-Encoding" header field indicates preferences regarding the    |
// | use of content codings.                                                   |
// |                                                                           |
// | In a request, it identifies the content codings acceptable in the         |
// | response. In a response, it identifies the content codings preferred for  |
// | the content of a subsequent request to the same resource.                 |
// |                                                                           |
// | The special "identity" token means that no content coding is applied. The |
// | wildcard "*" matches any available content coding not explicitly listed.  |
// | Content-coding names are case-insensitive.                                |
// |                                                                           |
// | Each coding MAY have a quality value (qvalue). A qvalue of 0 means "not   |
// | acceptable"; a missing weight has the default value 1. A higher non-zero  |
// | qvalue indicates a greater preference.                                    |
// |                                                                           |
// | If the field is absent from a request, any content coding is considered   |
// | acceptable. An empty field value means that the user agent does not want  |
// | any content coding applied to the response.                               |
// |                                                                           |
// | A representation without content coding is acceptable unless "identity"   |
// | is explicitly excluded with "identity;q=0", or indirectly excluded by     |
// | "*;q=0" without a more specific entry for "identity".                     |
// |                                                                           |
// | A server that rejects request content because its content coding is not   |
// | supported ought to send 415 (Unsupported Media Type) and include          |
// | Accept-Encoding. It MUST NOT include Accept-Encoding in a 415 response    |
// | when the failure is unrelated to content coding.                          |
// |                                                                           |
// | Examples:                                                                 |
// |   Accept-Encoding: compress, gzip                                         |
// |   Accept-Encoding:                                                        |
// |   Accept-Encoding: *                                                      |
// |   Accept-Encoding: compress;q=0.5, gzip;q=1.0                             |
// |   Accept-Encoding: gzip;q=1.0, identity; q=0.5, *;q=0                     |
// +---------------------------------------------------------------------------+
// | RFC 9110 �12.5.3 Accept-Encoding (ABNF summary)                           |
// +---------------------------------------------------------------------------+
// +-----------------+---------------------------------------------------------+
// | Field           | Definition                                              |
// +-----------------+---------------------------------------------------------+
// | Accept-Encoding | #( codings [ weight ] )                                 |
// | codings         | content-coding / "identity" / "*"                       |
// | content-coding  | token                                                   |
// | weight          | OWS ";" OWS "q=" qvalue                                 |
// | qvalue          | ( "0" [ "." 0*3DIGIT ] )                                |
// |                 | / ( "1" [ "." 0*3("0") ] )                              |
// | token           | 1*tchar                                                 |
// | tchar           | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" /         |
// |                 | "-" / "." / "^" / "_" / "`" / "|" / "~" / DIGIT /       |
// |                 | ALPHA                                                   |
// | OWS             | *( SP / HTAB )                                          |
// +---------------------------------------------------------------------------+
// | RFC 9110 �5.6.1 list expansion                                            |
// +---------------------------------------------------------------------------+
// | element = codings [ weight ]                                              |
// |                                                                           |
// | Sender syntax:                                                            |
// |                                                                           |
// |   #element = [ element *( OWS "," OWS element ) ]                         |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   #element = [ element ] *( OWS "," OWS [ element ] )                     |
// |                                                                           |
// | Senders MUST NOT generate empty list elements. Recipients MUST parse and  |
// | ignore a reasonable number of empty list elements.                        |
// |                                                                           |
// | Since Accept-Encoding uses "#", rather than "1#", zero non-empty elements |
// | are permitted by the purely syntactic ABNF. An empty field value has the  |
// | specific semantic meaning described above.                                |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is normalized (outer OWS already removed).         |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class accept_encoding {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    return helpers::for_each_list_element(sv, consume_coding_element);
  }

 private:
  // +=========================================================================+
  // | [>] consume_coding_element                                  ( private ) |
  // +=========================================================================+
  static constexpr bool consume_coding_element(std::string_view sv) {
    // A coding element is a coding followed by an optional weight.
    const std::string_view coding = helpers::consume_token(sv);
    if (coding.empty()) return false;
    const std::size_t off = coding.size();
    if (off == sv.size()) return true;
    // Any remaining content must be exactly one weight.
    return helpers::consume_weight(sv.substr(off));
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
