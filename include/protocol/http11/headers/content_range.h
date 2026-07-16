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

#ifndef martianlabs_doba_protocol_http11_headers_content_range_h
#define martianlabs_doba_protocol_http11_headers_content_range_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                             content-range |
// +===========================================================================+
// | RFC 9110 �14.4 Content-Range                                              |
// +---------------------------------------------------------------------------+
// | The "Content-Range" header field identifies which range of a selected     |
// | representation is enclosed in a message or body part.                     |
// |                                                                           |
// | It is sent in a single-part 206 (Partial Content) response, in each body  |
// | part of a multipart 206 response, and in a 416 (Range Not Satisfiable)    |
// | response to describe the selected representation.                         |
// |                                                                           |
// | A multipart 206 response MUST NOT carry Content-Range in its top-level    |
// | header section. Each enclosed body part MUST carry its own Content-Range  |
// | field.                                                                    |
// |                                                                           |
// | If a 206 response uses a range unit that the recipient does not           |
// | understand, the recipient MUST NOT combine the received content with a    |
// | stored representation. A proxy SHOULD forward the response downstream.    |
// |                                                                           |
// | Content-Range can also be used as a request modifier for a partial PUT    |
// | when defined by private agreement. A server MUST ignore it on request     |
// | methods for which Content-Range support is not defined.                   |
// |                                                                           |
// | For byte ranges, a sender SHOULD provide the complete representation      |
// | length unless that length is unknown or difficult to determine. An        |
// | asterisk ("*") in place of complete-length indicates that the length was  |
// | unknown.                                                                  |
// |                                                                           |
// | A byte range response is invalid when last-pos is less than first-pos, or |
// | when complete-length is less than or equal to last-pos. These semantic    |
// | constraints are additional to the ABNF.                                   |
// |                                                                           |
// | A server producing a 416 response to a byte-range request SHOULD send the |
// | unsatisfied form. In that form, complete-length is the current length of  |
// | the selected representation.                                              |
// |                                                                           |
// | Examples:                                                                 |
// |   Content-Range: bytes 0-499/1234                                         |
// |   Content-Range: bytes 500-1233/*                                         |
// |   Content-Range: bytes */1234                                             |
// +---------------------------------------------------------------------------+
// | RFC 9110 ��14.1 and 14.4 Content-Range (ABNF summary)                     |
// +---------------------------------------------------------------------------+
// +--------------------+------------------------------------------------------+
// | Field              | Definition                                           |
// +--------------------+------------------------------------------------------+
// | Content-Range      | range-unit SP ( range-resp / unsatisfied-range )     |
// | range-resp         | incl-range "/" ( complete-length / "*" )             |
// | incl-range         | first-pos "-" last-pos                               |
// | unsatisfied-range  | "*/" complete-length                                 |
// | complete-length    | 1*DIGIT                                              |
// | range-unit         | bytes-unit / other-range-unit                        |
// | bytes-unit         | "bytes"                                              |
// | other-range-unit   | token                                                |
// | first-pos          | 1*DIGIT                                              |
// | last-pos           | 1*DIGIT                                              |
// | token              | 1*tchar                                              |
// | tchar              | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" /      |
// |                    | "-" / "." / "^" / "_" / "`" / "|" / "~" / DIGIT /    |
// |                    | ALPHA                                                |
// | DIGIT              | %x30-39                                              |
// | SP                 | %x20                                                 |
// +---------------------------------------------------------------------------+
// | The SP after range-unit is mandatory and represents exactly one ASCII     |
// | space. The grammar does not allow OWS in its place or around "-", "/", or |
// | "*".                                                                      |
// |                                                                           |
// | The ABNF accepts arbitrarily long decimal numerals. Implementations that  |
// | convert them to an integer type MUST prevent overflow.                    |
// |                                                                           |
// | The ABNF alone accepts values such as "bytes 500-499/1234" and "bytes     |
// | 0-499/499". Ordering and complete-length constraints require separate     |
// | semantic validation.                                                      |
// |                                                                           |
// | IMPORTANT: field-value is supposed to be normalized (no OWS around        |
// | value).                                                                   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class content_range {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    std::size_t off = 0;
    // -------------------------------------------------------------------------
    // range-unit = token
    // -------------------------------------------------------------------------
    const std::string_view range_unit = helpers::consume_token(sv);
    if (range_unit.empty()) return false;
    off += range_unit.size();
    // -------------------------------------------------------------------------
    // SP
    //
    // Exactly one ASCII space is required after range-unit. OWS and HTAB are
    // not permitted by the ABNF.
    // -------------------------------------------------------------------------
    if (off >= sv.size() || sv[off++] != ' ') return false;
    // -------------------------------------------------------------------------
    // range-resp / unsatisfied-range
    // -------------------------------------------------------------------------
    if (off >= sv.size()) return false;
    const std::string_view remainder = sv.substr(off);
    // -------------------------------------------------------------------------
    // unsatisfied-range starts with '*':
    //
    //   unsatisfied-range = "*/" complete-length
    // -------------------------------------------------------------------------
    if (remainder.front() == '*') {
      return consume_unsatisfied_range(remainder);
    }
    // -------------------------------------------------------------------------
    // Otherwise, the value must be:
    //
    //   range-resp = incl-range "/" ( complete-length / "*" )
    // -------------------------------------------------------------------------
    return consume_range_resp(remainder);
  }

 private:
  // +=========================================================================+
  // | [>] consume_unsatisfied_range                               ( private ) |
  // +=========================================================================+
  // | unsatisfied-range = "*/" complete-length                                |
  // | complete-length   = 1*DIGIT                                             |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_unsatisfied_range(std::string_view sv) {
    // Minimum valid value: "*/0"
    if (sv.size() < 3) return false;
    // -------------------------------------------------------------------------
    // "*/"
    // -------------------------------------------------------------------------
    if (sv[0] != '*' || sv[1] != '/') return false;
    // -------------------------------------------------------------------------
    // complete-length = 1*DIGIT
    // -------------------------------------------------------------------------
    return helpers::is_digits(sv.substr(2));
  }
  // +=========================================================================+
  // | [>] consume_range_resp                                      ( private ) |
  // +=========================================================================+
  // | range-resp = incl-range "/" ( complete-length / "*" )                   |
  // | incl-range = first-pos "-" last-pos                                     |
  // | first-pos  = 1*DIGIT                                                    |
  // | last-pos   = 1*DIGIT                                                    |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_range_resp(std::string_view sv) {
    std::size_t off = 0;
    // -------------------------------------------------------------------------
    // first-pos = 1*DIGIT
    // -------------------------------------------------------------------------
    if (off >= sv.size() || !helpers::is_digit(sv[off])) return false;
    do {
      off++;
    } while (off < sv.size() && helpers::is_digit(sv[off]));
    // -------------------------------------------------------------------------
    // "-"
    // -------------------------------------------------------------------------
    if (off >= sv.size() || sv[off++] != '-') return false;
    // -------------------------------------------------------------------------
    // last-pos = 1*DIGIT
    // -------------------------------------------------------------------------
    if (off >= sv.size() || !helpers::is_digit(sv[off])) return false;
    do {
      off++;
    } while (off < sv.size() && helpers::is_digit(sv[off]));
    // -------------------------------------------------------------------------
    // "/"
    // -------------------------------------------------------------------------
    if (off >= sv.size() || sv[off++] != '/') return false;
    // -------------------------------------------------------------------------
    // complete-length / "*"
    // -------------------------------------------------------------------------
    if (off >= sv.size()) return false;
    if (sv[off] == '*') {
      off++;
    } else {
      // -----------------------------------------------------------------------
      // complete-length = 1*DIGIT
      // -----------------------------------------------------------------------
      if (!helpers::is_digit(sv[off])) return false;
      do {
        off++;
      } while (off < sv.size() && helpers::is_digit(sv[off]));
    }
    // -------------------------------------------------------------------------
    // No trailing characters are permitted.
    // -------------------------------------------------------------------------
    return off == sv.size();
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
