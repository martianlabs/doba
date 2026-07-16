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

#ifndef martianlabs_doba_protocol_http11_headers_set_cookie_h
#define martianlabs_doba_protocol_http11_headers_set_cookie_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                               set-cookie  |
// +===========================================================================+
// | RFC 6265 �4.1 Set-Cookie                                                  |
// +---------------------------------------------------------------------------+
// | The "Set-Cookie" response header field is used to send a cookie from the  |
// | server to the user agent. The user agent stores the cookie together with  |
// | its attributes and can return the cookie in subsequent requests using the |
// | Cookie request header field.                                              |
// |                                                                           |
// | A Set-Cookie field-value contains exactly one cookie. It begins with a    |
// | cookie name/value pair, followed by zero or more cookie attributes.       |
// | Attributes are separated from the cookie-pair by semicolons.              |
// |                                                                           |
// | A server can send multiple cookies in a single response,                  |
// | but it does so by sending multiple Set-Cookie header fields,              |
// | not by combining them into one comma-separated field value.               |
// |                                                                           |
// | Set-Cookie does not use the HTTP list syntax. In particular, commas can   |
// | appear inside the Expires attribute date, so a recipient MUST NOT treat a |
// | comma as a cookie separator.                                              |
// |                                                                           |
// | The cookie-name is a token and therefore cannot be empty.                 |
// | The cookie-value can be empty and can optionally be wrapped in DQUOTE,    |
// | but the allowed cookie octets exclude CTLs, whitespace,                   |
// | DQUOTE, comma, semicolon, and backslash.                                  |
// |                                                                           |
// | Standard attributes include Expires, Max-Age, Domain, Path, Secure, and   |
// | HttpOnly. Unknown or extension attributes are syntactically accepted as   |
// | extension-av. Their semantics are outside pure ABNF validation.           |
// |                                                                           |
// | Examples:                                                                 |
// |   Set-Cookie: SID=31d4d96e407aad42; Path=/; Secure; HttpOnly              |
// |   Set-Cookie: lang=en-US; Expires=Wed, 09 Jun 2021 10:18:14 GMT           |
// |   Set-Cookie: theme=light; Path=/; SameSite=Lax                           |
// +---------------------------------------------------------------------------+
// | RFC 6265 �4.1.1 Set-Cookie (ABNF summary)                                 |
// +---------------------------------------------------------------------------+
// +-------------------+-------------------------------------------------------+
// | Field             | Definition                                            |
// +-------------------+-------------------------------------------------------+
// | Set-Cookie        | set-cookie-string                                     |
// | set-cookie-string | cookie-pair *( ";" SP cookie-av )                     |
// | cookie-pair       | cookie-name "=" cookie-value                          |
// | cookie-name       | token                                                 |
// | cookie-value      | *cookie-octet / ( DQUOTE *cookie-octet DQUOTE )       |
// | cookie-octet      | %x21 / %x23-2B / %x2D-3A / %x3C-5B / %x5D-7E          |
// |                   | ; US-ASCII excluding CTLs, whitespace, DQUOTE, comma, |
// |                   | ; semicolon, and backslash                            |
// | cookie-av         | expires-av / max-age-av / domain-av / path-av /       |
// |                   | secure-av / httponly-av / extension-av                |
// | expires-av        | "Expires=" sane-cookie-date                           |
// | sane-cookie-date  | <rfc1123-date>                                        |
// | max-age-av        | "Max-Age=" non-zero-digit *DIGIT                      |
// | non-zero-digit    | %x31-39                                               |
// | domain-av         | "Domain=" domain-value                                |
// | domain-value      | <subdomain>                                           |
// | path-av           | "Path=" path-value                                    |
// | path-value        | <any CHAR except CTLs or ";">                         |
// | secure-av         | "Secure"                                              |
// | httponly-av       | "HttpOnly"                                            |
// | extension-av      | <any CHAR except CTLs or ";">                         |
// | token             | 1*tchar                                               |
// | tchar             | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" / |
// |                   | "." / "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA     |
// | SP                | %x20                                                  |
// | DQUOTE            | %x22                                                  |
// +---------------------------------------------------------------------------+
// | RFC 9110 �5.3 Set-Cookie field-line exception                             |
// +---------------------------------------------------------------------------+
// | Set-Cookie is a well-known exception to the normal HTTP field combination |
// | rule. Multiple Set-Cookie field lines cannot be safely combined into a    |
// | single comma-separated field value.                                       |
// |                                                                           |
// | Correct:                                                                  |
// |                                                                           |
// |   Set-Cookie: SID=31d4d96e407aad42; Path=/; Secure; HttpOnly              |
// |   Set-Cookie: lang=en-US; Path=/; Domain=site.example                     |
// |                                                                           |
// | Incorrect:                                                                |
// |                                                                           |
// |   Set-Cookie: SID=31d4d96e407aad42; Path=/; Secure; HttpOnly,             |
// |               lang=en-US; Path=/; Domain=site.example                     |
// |                                                                           |
// | Therefore, do not apply RFC 9110 #rule list expansion to Set-Cookie.      |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class set_cookie {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // Set-Cookie = set-cookie-string = cookie-pair *( "; " cookie-av ). It is
    // not an RFC 9110 #list: the separator is exactly "; " (semicolon + one
    // SP), commas are legal inside a cookie-av (e.g. the Expires date), and a
    // leading cookie-pair is mandatory, so an empty field value is rejected.
    return consume_set_cookie_string(sv);
  }

 private:
  // +=========================================================================+
  // | [>] consume_set_cookie_string                                ( private )|
  // +=========================================================================+
  // | set-cookie-string = cookie-pair *( "; " cookie-av )                     |
  // +-------------------------------------------------------------------------+
  // | Splits sv on the exact 2-octet "; " separator. The first segment must   |
  // | be a cookie-pair (helpers::is_cookie_pair, which requires a non-empty   |
  // | token cookie-name, so an empty field value is rejected); every later    |
  // | segment must be a cookie-av (helpers::is_cookie_av, which rejects empty |
  // | segments from a leading, trailing, or doubled separator). Splitting on  |
  // | the literal "; " is unambiguous because a cookie-av-octet excludes ";", |
  // | so no cookie-av can contain the separator.                              |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_set_cookie_string(std::string_view sv) {
    std::size_t start = 0;
    std::size_t i = 0;
    bool first = true;
    while (i < sv.size()) {
      // The one and only separator is the exact 2-octet sequence "; ".
      if (sv[i] == ';' && i + 1 < sv.size() && sv[i + 1] == ' ') {
        const std::string_view segment = sv.substr(start, i - start);
        // The leading segment is a cookie-pair; every later one is a cookie-av.
        if (first ? !helpers::is_cookie_pair(segment)
                  : !helpers::is_cookie_av(segment))
          return false;
        first = false;
        i += 2;
        start = i;
        continue;
      }
      i++;
    }
    // The final segment (or the only one when there is no separator).
    const std::string_view segment = sv.substr(start);
    return first ? helpers::is_cookie_pair(segment)
                 : helpers::is_cookie_av(segment);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
