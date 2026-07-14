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

#ifndef martianlabs_doba_protocol_http11_headers_origin_h
#define martianlabs_doba_protocol_http11_headers_origin_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                                    origin |
// +===========================================================================+
// | RFC 6454 �7 Origin                                                        |
// +---------------------------------------------------------------------------+
// | The "Origin" header field indicates the origin or origins that caused the |
// | user agent to issue an HTTP request. It is mainly used by servers to make |
// | access-control decisions based on the security context of the request.    |
// |                                                                           |
// | The field value is either the literal value "null" or one or more         |
// | serialized origins. A serialized origin contains only the scheme, host,   |
// | and optional port; it does not include path, query, or fragment data.     |
// |                                                                           |
// | A user agent MAY include an Origin header field in any HTTP request. A    |
// | user agent MUST NOT include more than one Origin header field in any      |
// | HTTP request.                                                             |
// |                                                                           |
// | Whenever a user agent issues an HTTP request from a privacy-sensitive     |
// | context, it MUST send the value "null" in the Origin header field.        |
// |                                                                           |
// | If multiple origins contributed to causing the request,                   |
// | the user agent MAY list all of them. Multiple serialized origins are      |
// | separated by a single SP character, not by commas.                        |
// |                                                                           |
// | Examples:                                                                 |
// |   Origin: https://example.com                                             |
// |   Origin: http://example.com https://redirect.example                     |
// |   Origin: null                                                            |
// +---------------------------------------------------------------------------+
// | RFC 6454 �7.1 Origin (ABNF summary)                                       |
// +---------------------------------------------------------------------------+
// +---------------------+-----------------------------------------------------+
// | Field               | Definition                                          |
// +---------------------+-----------------------------------------------------+
// | Origin              | origin-list-or-null                                 |
// | origin              | "Origin:" OWS origin-list-or-null OWS               |
// | origin-list-or-null | %x6E %x75 %x6C %x6C / origin-list                   |
// |                     | ; "null"                                            |
// | origin-list         | serialized-origin *( SP serialized-origin )         |
// | serialized-origin   | scheme "://" host [ ":" port ]                      |
// | scheme              | ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )          |
// | host                | IP-literal / IPv4address / reg-name                 |
// | port                | *DIGIT                                              |
// | IP-literal          | "[" ( IPv6address / IPvFuture ) "]"                 |
// | IPvFuture           | "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )|
// | reg-name            | *( unreserved / pct-encoded / sub-delims )          |
// | pct-encoded         | "%" HEXDIG HEXDIG                                   |
// | unreserved          | ALPHA / DIGIT / "-" / "." / "_" / "~"               |
// | sub-delims          | "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" /     |
// |                     | "," / ";" / "="                                     |
// | OWS                 | *( SP / HTAB )                                      |
// +---------------------------------------------------------------------------+
// | RFC 3986 host/port notes                                                  |
// +---------------------------------------------------------------------------+
// | The <scheme>, <host>, and <port> productions are imported from RFC 3986.  |
// |                                                                           |
// | The host rule is syntactically ambiguous between IPv4address and reg-name;|
// | RFC 3986 resolves this using first-match-wins: if the host matches        |
// | IPv4address, it is treated as an IPv4 address literal rather than a       |
// | registered name.                                                          |
// |                                                                           |
// | The port production is *DIGIT, so an empty port is syntactically permitted|
// | by RFC 3986 when the ":" delimiter is present. Whether to reject an empty |
// | port is a semantic or application-level policy decision.                  |
// +---------------------------------------------------------------------------+
// | IMPORTANT: Origin is not an RFC 9110 "#rule" comma-separated list.        |
// | Multiple serialized origins, if present, are separated by SP only.        |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class origin {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // origin-list-or-null = "null" / origin-list. The "null" literal is a
    // case-sensitive octet sequence (%x6E %x75 %x6C %x6C).
    if (sv == "null") return true;
    // origin-list = serialized-origin *( SP serialized-origin ). Elements are
    // separated by exactly one SP (never OWS or commas), so empty elements
    // (leading, trailing, or consecutive SP) and an empty value are rejected.
    std::size_t last = 0;
    while (true) {
      const std::size_t sp = sv.find(' ', last);
      const std::string_view element =
          sv.substr(last, sp == std::string_view::npos ? sv.size() - last
                                                        : sp - last);
      if (!helpers::check_serialized_origin(element)) return false;
      if (sp == std::string_view::npos) return true;
      last = sp + 1;
    }
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
