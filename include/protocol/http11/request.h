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

#ifndef martianlabs_doba_protocol_http11_request_h
#define martianlabs_doba_protocol_http11_request_h

#include <array>

#include "target.h"
#include "helpers.h"
#include "headers.h"
#include "common/hash_map.h"
#include "protocol/deserialization.h"

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// request                                                             ( class )
// -----------------------------------------------------------------------------
// This class holds for the http 1.1 request implementation.
// -----------------------------------------------------------------------------
// =============================================================================
class request {
 public:
  // ---------------------------------------------------------------------------
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  request() = default;
  request(const request&) = delete;
  request(request&&) noexcept = delete;
  ~request() = default;
  // ---------------------------------------------------------------------------
  // OPERATORs                                                        ( public )
  //
  request& operator=(const request&) = delete;
  request& operator=(request&&) noexcept = delete;
  // ---------------------------------------------------------------------------
  // METHODs                                                          ( public )
  //
  inline auto get_method() const { return method_; }
  inline auto get_target() const { return target_; }
  inline auto get_absolute_path() const { return absolute_path_; }
  inline auto get_query() const { return ""; }
  inline auto get_header(std::size_t i) const { return ""; }
  inline auto get_headers_length() const { return 0; }
  inline auto has_body() const { return false; }
  inline auto get_body_length() const { return 0; }
  // ---------------------------------------------------------------------------
  // STATIC-METHODs                                                   ( public )
  //
  static deserialization_result<request> deserialize(std::string_view buffer) {
    std::size_t bytes_used = 0;
    // Let's check if we have any bytes to process!
    if (buffer.empty()) return deserialization_status::kMoreBytesNeeded;
    // Let's create a new request instance!
    std::shared_ptr<request> req = std::make_shared<request>();
    // Let's try to find the end of the request-line part!
    std::size_t end_of_line = buffer.find("\r\n");
    if (end_of_line == std::string_view::npos) {
      return deserialization_status::kMoreBytesNeeded;
    }
    std::string_view request_line = buffer.substr(0, end_of_line);
    deserialization_status code = req->parse_request_line(request_line);
    if (code != deserialization_status::kSucceeded) return code;
    buffer.remove_prefix(end_of_line + 2);
    bytes_used += end_of_line + 2;  // due to "\r\n" characters.
    // Let's try to find the end of the headers part!
    std::size_t end_of_headers = buffer.find("\r\n\r\n");
    if (end_of_headers == std::string_view::npos) {
      return deserialization_status::kMoreBytesNeeded;
    }
    std::string_view headers = buffer.substr(0, end_of_headers + 2);
    if (headers.compare("\r\n")) {
      code = req->parse_headers(headers);
      if (code != deserialization_status::kSucceeded) return code;
    }
    bytes_used += end_of_headers + 4;  // due to "\r\n\r\n" characters.
    return deserialization_result<request>(req, bytes_used);
  }

 private:
  // +=========================================================================+
  // |                                                            request-line |
  // +=========================================================================+
  // | request-line = method SP request-target SP HTTP-version                 |
  // +-------------------------------------------------------------------------+
  // | source: https://datatracker.ietf.org/doc/html/rfc9110                   |
  // +-------------------------------------------------------------------------+
  deserialization_status parse_request_line(std::string_view sv) {
    // Let's try to decode [method] part!
    std::size_t end = sv.find(' ');
    if (end == std::string_view::npos) {
      return deserialization_status::kMoreBytesNeeded;
    }
    if (parse_method(sv.substr(0, end)) != deserialization_status::kSucceeded) {
      return deserialization_status::kInvalidSource;
    }
    sv.remove_prefix(end + 1);
    // Let's try to decode [request-target] part!
    end = sv.find(' ');
    if (end == std::string_view::npos) {
      return deserialization_status::kMoreBytesNeeded;
    }
    if (parse_request_target(sv.substr(0, end)) !=
        deserialization_status::kSucceeded) {
      return deserialization_status::kInvalidSource;
    }
    sv.remove_prefix(end + 1);
    // Let's try to decode [HTTP-version] part!
    if (parse_http_version(sv.substr(0, end)) !=
        deserialization_status::kSucceeded) {
      return deserialization_status::kInvalidSource;
    }
    return deserialization_status::kSucceeded;
  }
  // +=========================================================================+
  // |                                                                  method |
  // +=========================================================================+
  // | Field   | Definition                                                    |
  // +---------+---------------------------------------------------------------+
  // | method  | token                                                         |
  // | token   | 1*tchar                                                       |
  // | tchar   | "!" / "#" / "$" / "%" / "&" / "'" / "*" /                     |
  // |         | "+" / "-" / "." / "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA |
  // +---------+---------------------------------------------------------------+
  // | source: https://datatracker.ietf.org/doc/html/rfc9110                   |
  // +-------------------------------------------------------------------------+
  deserialization_status parse_method(std::string_view buffer) {
    std::size_t off = 0;
    while (off < buffer.size()) {
      if (!helpers::is_token(buffer[off++])) {
        return deserialization_status::kInvalidSource;
      }
    }
    method_ = buffer;
    return deserialization_status::kSucceeded;
  }
  // +=========================================================================+
  // |                                                          request-target |
  // +=========================================================================+
  // |                                                                         |
  // | request-target = origin-form                                            |
  // |                / absolute-form                                          |
  // |                / authority-form                                         |
  // |                / asterisk-form                                          |
  // |                                                                         |
  // +-------------------------------------------------------------------------+
  // | FORM 1: origin-form (most common)                                       |
  // |    origin-form = absolute-path [ "?" query ]                            |
  // +-------------------------------------------------------------------------+
  // | FORM 2: absolute-form (used via HTTP proxy)                             |
  // |    URI = scheme ":" hier-part [ "?" query ] [ "#" fragment ]            |
  // +-------------------------------------------------------------------------+
  // | FORM 3: authority-form (used with CONNECT)                              |
  // |    authority-form = host ":" port                                       |
  // +-------------------------------------------------------------------------+
  // | FORM 4: asterisk-form (used with OPTIONS)                               |
  // |    asterisk-form = "*"                                                  |
  // +=========================================================================+
  deserialization_status parse_request_target(std::string_view buffer) {
    if (method_ == method::kConnect) return parse_authority_form(buffer);
    if (method_ == method::kOptions) return parse_asterisk_form(buffer);
    if (parse_origin_form(buffer) != deserialization_status::kSucceeded) {
      return parse_absolute_form(buffer);
    }
    return deserialization_status::kSucceeded;
  }
  // +=========================================================================+
  // |                                                          authority-form |
  // +=========================================================================+
  // | Rule           | Definition                                             |
  // +----------------+--------------------------------------------------------+
  // | authority-form | authority                                              |
  // |                | (RFC 9112 §2.2)                                        |
  // | authority      | [ userinfo "@" ] host [ ":" port ]                     |
  // |                | (RFC 3986 §3.2)                                        |
  // |                |  userinfo is forbidden in HTTP `authority-form`        |
  // | host           | IP-literal / IPv4address / reg-name                    |
  // |                | (RFC 3986 §3.2.2)                                      |
  // | port           | *DIGIT                                                 |
  // |                | (RFC 3986 §3.2.3)                                      |
  // | userinfo       | *( unreserved / pct-encoded / sub-delims / ":" )       |
  // |                | (RFC 3986 §3.2.1) : forbidden in HTTP                  |
  // | unreserved     | ALPHA / DIGIT / "-" / "." / "_" / "~"                  |
  // |                | (RFC 3986 §2.3)                                        |
  // | pct-encoded    | "%" HEXDIG HEXDIG                                      |
  // |                | (RFC 3986 §2.1)                                        |
  // | sub-delims     | "!" / "$" / "&" / "'" / "(" / ")"                      |
  // |                | "*" / "+" / "," / ";" / "="                            |
  // |                | (RFC 3986 §2.2)                                        |
  // | IP-literal     | "[" ( IPv6address / IPvFuture ) "]"                    |
  // |                | (RFC 3986 §3.2.2)                                      |
  // | IPv4address    | dec-octet "." dec-octet "." dec-octet "." dec-octet    |
  // |                | (RFC 3986 §3.2.2)                                      |
  // | dec-octet      | DIGIT                 ; 0-9                            |
  // |                | / %x31-39 DIGIT       ; 10-99                          |
  // |                | / "1" 2DIGIT          ; 100-199                        |
  // |                | / "2" %x30-34 DIGIT   ; 200-249                        |
  // |                | / "25" %x30-35        ; 250-255                        |
  // |                | (RFC 3986 §3.2.2)                                      |
  // | reg-name       | *( unreserved / pct-encoded / sub-delims )             |
  // |                | (RFC 3986 §3.2.2)                                      |
  // +----------------+--------------------------------------------------------+
  deserialization_status parse_authority_form(std::string_view buffer) {
    // [to-do] -> add support for this!
    return deserialization_status::kInvalidSource;
  }
  // +=========================================================================+
  // |                                                           asterisk-form |
  // +=========================================================================+
  // | Field          | Definition                                             |
  // +----------------+--------------------------------------------------------+
  // | asterisk-form  | "*"                                                    |
  // +----------------+--------------------------------------------------------+
  deserialization_status parse_asterisk_form(std::string_view buffer) {
    // [to-do] -> add support for this!
    return deserialization_status::kInvalidSource;
  }
  // +=========================================================================+
  // |                                                             origin-form |
  // +=========================================================================+
  // | Field          | Definition                                             |
  // +----------------+--------------------------------------------------------+
  // | origin-form    | absolute-path [ "?" query ]                            |
  // +----------------+--------------------------------------------------------+
  deserialization_status parse_origin_form(std::string_view buffer) {
    std::size_t query_at = buffer.find('?');
    std::string_view query = query_at != std::string_view::npos
                                 ? buffer.substr(query_at + 1)
                                 : std::string_view();
    std::string_view path = query_at != std::string_view::npos
                                ? buffer.substr(0, query_at)
                                : buffer;
    // Let's try to decode [absolute-path] part!
    deserialization_status result = parse_absolute_path(path);
    if (result != deserialization_status::kSucceeded) return result;
    target_ = target::kOriginForm;
    // Let's try to decode [query] part!
    return query.size() ? parse_query_part(query)
                        : deserialization_status::kSucceeded;
  }
  // +=========================================================================+
  // |                                                           absolute-form |
  // +=========================================================================+
  // | Field          | Definition                                             |
  // +----------------+--------------------------------------------------------+
  // | absolute-form  | URI                                                    |
  // +----------------+--------------------------------------------------------+
  deserialization_status parse_absolute_form(std::string_view buffer) {
    // [to-do] -> add support for this!
    return deserialization_status::kInvalidSource;
  }
  // +=========================================================================+
  // |                                                          abosolute-path |
  // +=========================================================================+
  // +----------------+--------------------------------------------------------+
  // | Field          | Definition                                             |
  // +----------------+--------------------------------------------------------+
  // | abosulte-path  | 1*( "/" segment )                                      |
  // | segment        | *pchar                                                 |
  // | pchar          | unreserved / pct-encoded / sub-delims / ":" / "@"      |
  // | unreserved     | ALPHA / DIGIT / "-" / "." / "_" / "~"                  |
  // | pct-encoded    | "%" HEXDIG HEXDIG                                      |
  // | sub-delims     | "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" /        |
  // |                | "," / ";" / "="                                        |
  // +----------------+--------------------------------------------------------+
  // | source: https://datatracker.ietf.org/doc/html/rfc9110                   |
  // +----------------+--------------------------------------------------------+
  deserialization_status parse_absolute_path(std::string_view buffer) {
    std::size_t off = 0;
    if (off >= buffer.size()) return deserialization_status::kMoreBytesNeeded;
    if (buffer[off++] != '/') return deserialization_status::kInvalidSource;
    while (off < buffer.size()) {
      uint8_t c = static_cast<uint8_t>(buffer[off]);
      if (c == '%') {
        if (off + 2 >= buffer.size()) {
          return deserialization_status::kMoreBytesNeeded;
        }
        if (!helpers::is_hex_digit(buffer[off + 1]) ||
            !helpers::is_hex_digit(buffer[off + 2])) {
          return deserialization_status::kInvalidSource;
        }
        off += 3;
        continue;
      }
      if (!helpers::is_pchar(c) && c != '/') {
        return deserialization_status::kInvalidSource;
      }
      off++;
    }
    absolute_path_ = buffer;
    return deserialization_status::kSucceeded;
  }
  // +=========================================================================+
  // |                                                              query-part |
  // +=========================================================================+
  // +----------------+--------------------------------------------------------+
  // | Field          | Definition                                             |
  // +----------------+--------------------------------------------------------+
  // | query          | *( pchar / "/" / "?" )                                 |
  // | pchar          | unreserved / pct-encoded / sub-delims / ":" / "@"      |
  // | unreserved     | ALPHA / DIGIT / "-" / "." / "_" / "~"                  |
  // | pct-encoded    | "%" HEXDIG HEXDIG                                      |
  // | sub-delims     | "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" /        |
  // |                | "," / ";" / "="                                        |
  // +----------------+--------------------------------------------------------+
  // | source: https://datatracker.ietf.org/doc/html/rfc9110                   |
  // +----------------+--------------------------------------------------------+
  deserialization_status parse_query_part(std::string_view buffer) {
    std::size_t off = 0;
    while (off < buffer.size()) {
      uint8_t c = static_cast<uint8_t>(buffer[off]);
      if (c == '%') {
        if (off + 2 >= buffer.size()) {
          return deserialization_status::kMoreBytesNeeded;
        }
        if (!helpers::is_hex_digit(buffer[off + 1]) ||
            !helpers::is_hex_digit(buffer[off + 2])) {
          return deserialization_status::kInvalidSource;
        }
        off += 3;
        continue;
      }
      if (!helpers::is_pchar(c) && c != '/' && c != '?') {
        return deserialization_status::kInvalidSource;
      }
      off++;
    }
    query_part_ = buffer;
    return deserialization_status::kSucceeded;
  }
  // +=========================================================================+
  // |                                                            http-version |
  // +=========================================================================+
  // | HTTP-version   | HTTP-name "/" DIGIT "." DIGIT                          |
  // | HTTP-name      | %s"HTTP"                                               |
  // +----------------+--------------------------------------------------------+
  // | source: https://datatracker.ietf.org/doc/html/rfc9110 |                 |
  // +-------------------------------------------------------------------------+
  deserialization_status parse_http_version(std::string_view buffer) {
    if (buffer.size() < 8) {
      return deserialization_status::kMoreBytesNeeded;
    }
    if (buffer[0] != 'H' || buffer[1] != 'T' || buffer[2] != 'T' ||
        buffer[3] != 'P' || buffer[4] != '/' || !helpers::is_digit(buffer[5]) ||
        buffer[6] != '.' || !helpers::is_digit(buffer[7])) {
      return deserialization_status::kInvalidSource;
    }
    return deserialization_status::kSucceeded;
  }
  // +=========================================================================+
  // |                                                                 headers |
  // +=========================================================================+
  // +-----------------+-------------------------------------------------------+
  // | Rule            | Definition                                            |
  // +-----------------+-------------------------------------------------------+
  // | header-field    | field-name ":" OWS field-value OWS                    |
  // | field-name      | token                                                 |
  // | token           | 1*tchar                                               |
  // | tchar           | ALPHA / DIGIT / "!" / "#" / "$" / "%" / "&" / "'" /   |
  // |                 | "*" / "+" / "-" / "." / "^" / "_" / "`" / "|" / "~"   |
  // | field-value     | *( field-content )                                    |
  // | field-content   | field-vchar [ *( SP / HTAB ) field-vchar ]            |
  // | field-vchar     | VCHAR / obs-text                                      |
  // | OWS             | *( SP / HTAB )                                        |
  // | obs-fold        | CRLF 1*( SP / HTAB ) ; obsolete, not supported        |
  // +-----------------+-------------------------------------------------------+
  // | source: https://datatracker.ietf.org/doc/html/rfc9110                   |
  // +-------------------------------------------------------------------------+
  deserialization_status parse_headers(std::string_view buffer) {
    std::size_t off = 0, fn_start = 0;
    bool field_name_decoded = false;
    while (off < buffer.size()) {
      if (!field_name_decoded) {
        // [field-name] decoding..
        field_name_decoded = buffer[off++] == ':';
        continue;
      }
      // [field-value] decoding..
      std::string_view field_name = buffer.substr(fn_start, off - fn_start - 1);
      std::size_t end_of_header = buffer.find("\r\n", off);
      if (end_of_header == std::string_view::npos) {
        return deserialization_status::kMoreBytesNeeded;
      }
      for (std::size_t i = off; i < end_of_header; ++i) {
        if (!helpers::is_vchar(buffer[i]) && !helpers::is_obs_text(buffer[i]) &&
            !helpers::is_ows(buffer[i])) {
          return deserialization_status::kInvalidSource;
        }
      }
      std::string_view field_value =buffer.substr(off, end_of_header - off);
      helpers::ows_trim(field_value);
      headers_.emplace_back(field_name, field_value);
      fn_start = off = end_of_header + 2;
      field_name_decoded = false;
    }
    return deserialization_status::kSucceeded;
  }
  // ---------------------------------------------------------------------------
  // ATTRIBUTEs                                                      ( private )
  //
  std::string method_;
  std::string query_part_;
  std::string absolute_path_;
  target target_ = target::kUnknown;
  std::vector<std::pair<std::string, std::string>> headers_;
  std::vector<std::pair<std::string, std::string>> query_parameters_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
