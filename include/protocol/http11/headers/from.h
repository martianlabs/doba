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

#ifndef martianlabs_doba_protocol_http11_headers_from_h
#define martianlabs_doba_protocol_http11_headers_from_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                                      from |
// +===========================================================================+
// | RFC 9110 �10.1.2 From                                                     |
// +---------------------------------------------------------------------------+
// | The "From" header field contains an Internet email address for a human    |
// | user who controls the requesting user agent. The address ought to be      |
// | machine-usable, as defined by "mailbox" in RFC 5322 �3.4.                 |
// |                                                                           |
// | Non-robotic user agents rarely send From. A user agent SHOULD NOT send a  |
// | From header field without explicit configuration by the user, since that  |
// | might conflict with the user's privacy interests or the site's security   |
// | policy.                                                                   |
// |                                                                           |
// | A robotic user agent SHOULD send a valid From header field so that the    |
// | person responsible for running the robot can be contacted if problems     |
// | occur on servers, such as excessive, unwanted, or invalid requests.       |
// |                                                                           |
// | A server SHOULD NOT use the From header field for access control or       |
// | authentication, since its value is visible to recipients and observers    |
// | and is often recorded in log files and error reports.                     |
// |                                                                           |
// | Example:                                                                  |
// |   From: spider-admin@example.org                                          |
// +---------------------------------------------------------------------------+
// | RFC 9110 �10.1.2 From (ABNF summary)                                      |
// +---------------------------------------------------------------------------+
// +----------------+----------------------------------------------------------+
// | Field          | Definition                                               |
// +----------------+----------------------------------------------------------+
// | From           | mailbox                                                  |
// | mailbox        | name-addr / addr-spec                                    |
// | name-addr      | [ display-name ] angle-addr                              |
// | angle-addr     | [ CFWS ] "<" addr-spec ">" [ CFWS ] / obs-angle-addr     |
// | display-name   | phrase                                                   |
// | addr-spec      | local-part "@" domain                                    |
// | local-part     | dot-atom / quoted-string / obs-local-part                |
// | domain         | dot-atom / domain-literal / obs-domain                   |
// | dot-atom       | [ CFWS ] dot-atom-text [ CFWS ]                          |
// | dot-atom-text  | 1*atext *( "." 1*atext )                                 |
// | atext          | ALPHA / DIGIT / "!" / "#" / "$" / "%" / "&" / "'" /      |
// |                | "*" / "+" / "-" / "/" / "=" / "?" / "^" / "_" / "`" /    |
// |                | "{" / "|" / "}" / "~"                                    |
// | quoted-string  | [ CFWS ] DQUOTE *([ FWS ] qcontent) [ FWS ] DQUOTE       |
// |                | [ CFWS ]                                                 |
// | qcontent       | qtext / quoted-pair                                      |
// | qtext          | %d33 / %d35-91 / %d93-126 / obs-qtext                    |
// | quoted-pair    | "\" ( VCHAR / WSP ) / obs-qp                             |
// | domain-literal | [ CFWS ] "[" *([ FWS ] dtext) [ FWS ] "]" [ CFWS ]       |
// | dtext          | %d33-90 / %d94-126 / obs-dtext                           |
// | CFWS           | (1*([ FWS ] comment) [ FWS ]) / FWS                      |
// | FWS            | ([*WSP CRLF] 1*WSP) / obs-FWS                            |
// | WSP            | SP / HTAB                                                |
// +---------------------------------------------------------------------------+
// | RFC 5322 imported mailbox syntax                                          |
// +---------------------------------------------------------------------------+
// | HTTP does not define an independent email-address grammar for From.       |
// | Instead, it imports "mailbox" from RFC 5322 �3.4. A mailbox can be either |
// | a name-address form with angle brackets or a bare addr-spec.              |
// |                                                                           |
// | Valid shapes include:                                                     |
// |                                                                           |
// |   From: spider-admin@example.org                                          |
// |   From: "Spider Admin" <spider-admin@example.org>                         |
// |                                                                           |
// | The field is not a list field. Multiple comma-separated mailboxes are not |
// | permitted by the From ABNF in HTTP.                                       |
// |                                                                           |
// | For strict HTTP parsing, CR and LF inside the normalized field value      |
// | should already have been rejected by the generic HTTP field parser. RFC   |
// | 5322 FWS includes folding constructs, but HTTP field-values are parsed as |
// | HTTP field-values before applying this imported mailbox grammar.          |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class from {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // From = mailbox. HTTP does not define its own email-address grammar; it
    // imports "mailbox" from RFC 5322 �3.4, which is either a name-addr (an
    // optional display-name followed by an angle-addressed addr-spec) or a
    // bare addr-spec. From is not a list field, so a single mailbox must span
    // the whole (already normalized) field-value.
    return helpers::check_mailbox(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
