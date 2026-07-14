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

#ifndef martianlabs_doba_protocol_http11_headers_server_h
#define martianlabs_doba_protocol_http11_headers_server_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                                    server |
// +===========================================================================+
// | RFC 9110 �10.2.4 Server                                                   |
// +---------------------------------------------------------------------------+
// | The "Server" header field contains information about the software used by |
// | the origin server to handle the request. It is commonly used by clients to|
// | identify the scope of interoperability problems, work around known server |
// | limitations, or collect analytics about server and operating system use.  |
// |                                                                           |
// | An origin server MAY generate a Server header field in its responses.     |
// |                                                                           |
// | The field value consists of one or more product identifiers,              |
// | each followed by zero or more comments. Together, these identify          |
// | the origin server software and its significant subproducts.               |
// | By convention, product identifiers are listed in decreasing               |
// | order of significance.                                                    |
// |                                                                           |
// | Each product identifier consists of a product name and an                 |
// | optional version. A sender SHOULD limit generated product identifiers to  |
// | what is necessary to identify the product. A sender MUST NOT generate     |
// | advertising or other nonessential information within product identifiers. |
// |                                                                           |
// | An origin server SHOULD NOT generate a Server field containing needlessly |
// | fine-grained detail and SHOULD limit the addition of third-party          |
// | subproducts. Overly detailed values can reveal implementation             |
// | details that might make known vulnerabilities easier to identify.         |
// |                                                                           |
// | Examples:                                                                 |
// |   Server: CERN/3.0 libwww/2.17                                            |
// |   Server: Apache                                                          |
// |   Server: doba/1.0                                                        |
// |   Server: doba/1.0 (Windows)                                              |
// +---------------------------------------------------------------------------+
// | RFC 9110 �10.2.4 Server (ABNF summary)                                    |
// +---------------------------------------------------------------------------+
// +-----------------+---------------------------------------------------------+
// | Field           | Definition                                              |
// +-----------------+---------------------------------------------------------+
// | Server          | product *( RWS ( product / comment ) )                  |
// | product         | token [ "/" product-version ]                           |
// | product-version | token                                                   |
// | comment         | "(" *( ctext / quoted-pair / comment ) ")"              |
// | ctext           | HTAB / SP / %x21-27 / %x2A-5B / %x5D-7E / obs-text      |
// | quoted-pair     | "\" ( HTAB / SP / VCHAR / obs-text )                    |
// | RWS             | 1*( SP / HTAB )                                         |
// | token           | 1*tchar                                                 |
// | tchar           | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" /   |
// |                 | "." / "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA       |
// | obs-text        | %x80-FF                                                 |
// | VCHAR           | %x21-7E                                                 |
// +---------------------------------------------------------------------------+
// | Notes                                                                     |
// +---------------------------------------------------------------------------+
// | Server is not a list-based field. It does not use the RFC 9110 �5.6.1     |
// | #rule syntax, so comma-separated list expansion does not apply.           |
// |                                                                           |
// | The field value is syntactically invalid if it does not begin with a      |
// | product. Comments are only allowed after the first product and each       |
// | subsequent product or comment must be separated by RWS.                   |
// |                                                                           |
// | Comments can be nested because comment is recursively defined.            |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class server {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // Server = product *( RWS ( product / comment ) ). It is not an RFC 9110
    // �5.6.1 "#" list: the value MUST begin with a product, items are
    // separated by RWS (not commas), and this is the same product-list shape
    // shared with the User-Agent header field (RFC 9110 �10.1.5).
    return helpers::check_product_list(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
