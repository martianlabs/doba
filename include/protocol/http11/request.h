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

#include <charconv>
#include <array>

#include "target.h"
#include "helpers.h"
#include "headers.h"
#include "common/hash_map.h"
#include "common/deserialize_result.h"

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// request                                                             ( class )
// -----------------------------------------------------------------------------
// This class holds for the http 1.1 request implementation.
// -----------------------------------------------------------------------------
// =============================================================================
class request {
  // ---------------------------------------------------------------------------
  // USINGs                                                          ( private )
  //
  using headers_content = std::pair<std::string_view, std::string_view>;
  using headers = std::array<headers_content, 64>;

 public:
  // ---------------------------------------------------------------------------
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  request(const request&) = delete;
  request(request&&) noexcept = delete;
  ~request() { delete[] buf_; }
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
  inline auto get_query() const { return query_part_; }
  inline auto get_header(std::size_t index) const { return headers_[index]; }
  inline auto get_headers_length() const { return headers_used_; }
  inline auto has_body() const { return false; }
  inline auto get_body_length() const { return 0; }
  // ---------------------------------------------------------------------------
  // STATIC-METHODs                                                   ( public )
  //
  static std::pair<common::deserialize_result, request*> deserialize(
      char* buffer, std::size_t length, std::size_t& bytes_used) {
    std::pair<common::deserialize_result, request*> result;
    target target = target::kUnknown;
    std::string_view query_part;
    std::string_view absolute_path;
    std::string_view method;
    headers hdrs;
    std::size_t i = 0, j = 0, hdrs_len = 0;
    result.first = check_request_line(buffer, length, method, target,
                                      absolute_path, query_part, i);
    if (result.first == common::deserialize_result::kSucceeded) {
      result.first = check_headers(buffer, i, length, hdrs, hdrs_len, j);
      if (result.first == common::deserialize_result::kSucceeded) {
        result.second = new request(buffer, j);
        result.second->method_ = method;
        result.second->target_ = target;
        result.second->absolute_path_ = absolute_path;
        result.second->query_part_ = query_part;
        result.second->headers_ = hdrs;
        result.second->headers_used_ = hdrs_len;
        bytes_used = j;
      }
    }
    return result;
  }

 private:
  // ---------------------------------------------------------------------------
  // CONSTRUCTORs/DESTRUCTORs                                        ( private )
  //
  request(const char* const buffer, std::size_t length) {
    if (char* alloc = new char[length]) {
      std::memcpy(alloc, buffer, length);
      buf_ = alloc;
      sze_ = length;
    }
  }
  // ---------------------------------------------------------------------------
  // STATIC-METHODs                                                  ( private )
  //
  static inline common::deserialize_result check_request_line(
      char* buffer, std::size_t len, std::string_view& method_decoded,
      target& target_decoded, std::string_view& absolute_path,
      std::string_view& query_part, std::size_t& bytes_decoded) {
    std::size_t off = 0, tmp = 0;
    static constexpr std::size_t kHttpVersionLen = 8;
    // +-----------------------------------------------------------------------+
    // |                                                                method |
    // +---------+-------------------------------------------------------------+
    // | Field   | Definition                                                  |
    // +---------+-------------------------------------------------------------+
    // | method  | token                                                       |
    // | token   | 1*tchar                                                     |
    // | tchar   | "!" / "#" / "$" / "%" / "&" / "'" / "*" /                   |
    // |         | "+" / "-" / "." / "^" / "_" / "`" / "|" / "~"               |
    // |         | / DIGIT / ALPHA                                             |
    // +---------+-------------------------------------------------------------+
    // | source: https://datatracker.ietf.org/doc/html/rfc9110                 |
    // +----------------+------------------------------------------------------+
    char* sp = nullptr;
    while (off < len) {
      uint8_t c = static_cast<uint8_t>(buffer[off]);
      if (c == constants::character::kSpace) {
        sp = &buffer[off++];
        break;
      }
      if (!helpers::is_token(c)) {
        return common::deserialize_result::kInvalidSource;
      }
      off++;
    }
    if (!sp) return common::deserialize_result::kMoreBytesNeeded;
    method_decoded = std::string_view(buffer, sp - buffer);
    // +=======================================================================+
    // | HTTP/1.1: request-target (RFC 9112 §2.2) – Valid Forms                |
    // +=======================================================================+
    // |                                                                       |
    // | request-target = origin-form                                          |
    // |                / absolute-form                                        |
    // |                / authority-form                                       |
    // |                / asterisk-form                                        |
    // |                                                                       |
    // |-----------------------------------------------------------------------|
    // | FORM 1: origin-form (most common)                                     |
    // |    origin-form = absolute-path [ "?" query ]                          |
    // |-----------------------------------------------------------------------|
    // | FORM 2: absolute-form (used via HTTP proxy)                           |
    // |    URI = scheme ":" hier-part [ "?" query ] [ "#" fragment ]          |
    // |-----------------------------------------------------------------------|
    // | FORM 3: authority-form (used with CONNECT)                            |
    // |    authority-form = host ":" port                                     |
    // |-----------------------------------------------------------------------|
    // | FORM 4: asterisk-form (used with OPTIONS)                             |
    // |    asterisk-form = "*"                                                |
    // +=======================================================================+
    if (method_decoded == constants::method::kConnect) {
      // +---------------------------------------------------------------------+
      // |                                                      authority-form |
      // +----------------+----------------------------------------------------+
      // | Rule           | Definition                                         |
      // +----------------+----------------------------------------------------+
      // | authority-form | authority                                          |
      // |                | (RFC 9112 §2.2)                                    |
      // | authority      | [ userinfo "@" ] host [ ":" port ]                 |
      // |                | (RFC 3986 §3.2)                                    |
      // |                |  userinfo is forbidden in HTTP `authority-form`    |
      // | host           | IP-literal / IPv4address / reg-name                |
      // |                | (RFC 3986 §3.2.2)                                  |
      // | port           | *DIGIT                                             |
      // |                | (RFC 3986 §3.2.3)                                  |
      // | userinfo       | *( unreserved / pct-encoded / sub-delims / ":" )   |
      // |                | (RFC 3986 §3.2.1) : forbidden in HTTP              |
      // | unreserved     | ALPHA / DIGIT / "-" / "." / "_" / "~"              |
      // |                | (RFC 3986 §2.3)                                    |
      // | pct-encoded    | "%" HEXDIG HEXDIG                                  |
      // |                | (RFC 3986 §2.1)                                    |
      // | sub-delims     | "!" / "$" / "&" / "'" / "(" / ")"                  |
      // |                | "*" / "+" / "," / ";" / "="                        |
      // |                | (RFC 3986 §2.2)                                    |
      // | IP-literal     | "[" ( IPv6address / IPvFuture ) "]"                |
      // |                | (RFC 3986 §3.2.2)                                  |
      // | IPv4address    | dec-octet "." dec-octet "." dec-octet "." dec-octet|
      // |                | (RFC 3986 §3.2.2)                                  |
      // | dec-octet      | DIGIT                 ; 0-9                        |
      // |                | / %x31-39 DIGIT       ; 10-99                      |
      // |                | / "1" 2DIGIT          ; 100-199                    |
      // |                | / "2" %x30-34 DIGIT   ; 200-249                    |
      // |                | / "25" %x30-35        ; 250-255                    |
      // |                | (RFC 3986 §3.2.2)                                  |
      // | reg-name       | *( unreserved / pct-encoded / sub-delims )         |
      // |                | (RFC 3986 §3.2.2)                                  |
      // +----------------+----------------------------------------------------+
      // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
      // [to-do] -> add support for this!
      // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    } else if (method_decoded == constants::method::kOptions) {
      // +---------------------------------------------------------------------+
      // |                                                       asterisk-form |
      // +----------------+----------------------------------------------------+
      // | Field          | Definition                                         |
      // +----------------+----------------------------------------------------+
      // | asterisk-form  | "*"                                                |
      // +----------------+----------------------------------------------------+
      // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
      // [to-do] -> add support for this!
      // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    } else if (decode_absolute_path(buffer, off, len, absolute_path, tmp)) {
      // +---------------------------------------------------------------------+
      // |                                                         origin-form |
      // +----------------+----------------------------------------------------+
      // | Field          | Definition                                         |
      // +----------------+----------------------------------------------------+
      // | origin-form    | absolute-path [ "?" query ]                        |
      // +----------------+----------------------------------------------------+
      target_decoded = target::kOriginForm;
      if ((off += tmp) >= len) {
        return common::deserialize_result::kMoreBytesNeeded;
      }
      if (buffer[off] == constants::character::kQuestion) {
        common::deserialize_result res =
            decode_query_part(buffer, ++off, len, query_part, tmp);
        if (res != common::deserialize_result::kSucceeded) {
          return res;
        }
        off += tmp;
      }
    } else {
      // +---------------------------------------------------------------------+
      // |                                                       absolute-form |
      // +----------------+----------------------------------------------------+
      // | Field          | Definition                                         |
      // +----------------+----------------------------------------------------+
      // | absolute-form  | URI                                                |
      // +----------------+----------------------------------------------------+
      // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
      // [to-do] -> add support for this!
      // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    }
    // +-----------------------------------------------------------------------+
    // |                                                          http-version |
    // +----------------+------------------------------------------------------+
    // | HTTP-version   | HTTP-name "/" DIGIT "." DIGIT                        |
    // | HTTP-name      | %s"HTTP"                                             |
    // +----------------+------------------------------------------------------+
    // | source: https://datatracker.ietf.org/doc/html/rfc9110 |               |
    // +-----------------------------------------------------------------------+
    if (off >= len) return common::deserialize_result::kMoreBytesNeeded;
    if (buffer[off++] != constants::character::kSpace) {
      return common::deserialize_result::kInvalidSource;
    }
    if (len - off < kHttpVersionLen) {
      return common::deserialize_result::kMoreBytesNeeded;
    }
    if (buffer[off + 0] != constants::character::kHUpperCase ||
        buffer[off + 1] != constants::character::kTUpperCase ||
        buffer[off + 2] != constants::character::kTUpperCase ||
        buffer[off + 3] != constants::character::kPUpperCase ||
        buffer[off + 4] != constants::character::kSlash ||
        !helpers::is_digit(buffer[off + 5]) ||
        buffer[off + 6] != constants::character::kDot ||
        !helpers::is_digit(buffer[off + 7])) {
      return common::deserialize_result::kInvalidSource;
    }
    off += kHttpVersionLen;
    if (len - off < 2) return common::deserialize_result::kMoreBytesNeeded;
    if (buffer[off + 0] != constants::character::kCr ||
        buffer[off + 1] != constants::character::kLf) {
      return common::deserialize_result::kInvalidSource;
    }
    bytes_decoded = (off += 2);
    return common::deserialize_result::kSucceeded;
  }
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
  static inline common::deserialize_result check_headers(
      char* buffer, std::size_t off, std::size_t len, headers& hdrs,
      std::size_t& hdrs_len, std::size_t& bytes_decoded) {
    bool end_of_headers_found = false;
    while (off < len) {
      // [end-of-headers] check..
      if (buffer[off] == constants::character::kCr) {
        if (off + 1 >= len) {
          return common::deserialize_result::kMoreBytesNeeded;
        }
        if (buffer[off + 1] != constants::character::kLf) {
          return common::deserialize_result::kInvalidSource;
        }
        end_of_headers_found = true;
        off += 2;
        break;
      }
      // [field-name] decoding..
      char* col = nullptr;
      char* fns = &buffer[off];
      while (off < len) {
        if (buffer[off] == constants::character::kColon) {
          col = &buffer[off++];
          break;
        }
        if (!helpers::is_token(buffer[off])) {
          return common::deserialize_result::kInvalidSource;
        }
        off++;
      }
      if (!col) {
        return common::deserialize_result::kMoreBytesNeeded;
      }
      if (col == fns) {
        // empty field-name not allowed!
        return common::deserialize_result::kInvalidSource;
      }
      std::string_view name(fns, col - fns);
      // [field-value] decoding..
      char* crlf = nullptr;
      char* fvs = &buffer[off];
      while (off < len) {
        if (buffer[off] == constants::character::kCr) {
          if (off + 1 >= len) {
            return common::deserialize_result::kMoreBytesNeeded;
          }
          if (buffer[off + 1] != constants::character::kLf) {
            return common::deserialize_result::kInvalidSource;
          }
          crlf = &buffer[off];
          break;
        }
        if (!helpers::is_vchar(buffer[off]) &&
            !helpers::is_obs_text(buffer[off]) &&
            !helpers::is_ows(buffer[off])) {
          return common::deserialize_result::kInvalidSource;
        }
        off++;
      }
      if (!crlf) {
        return common::deserialize_result::kMoreBytesNeeded;
      }
      std::string_view value = helpers::ows_rtrim(
          helpers::ows_ltrim(std::string_view(fvs, crlf - fvs)));
      hdrs[hdrs_len] = std::make_pair(name, value);
      hdrs_len++;
      off += 2;
    }
    if (end_of_headers_found) {
      bytes_decoded = off;
      return common::deserialize_result::kSucceeded;
    }
    return common::deserialize_result::kInvalidSource;
  }
  // +-------------------------------------------------------------------------+
  // |                                                           absolute-path |
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
  static inline bool decode_absolute_path(char* buffer, std::size_t off,
                                          std::size_t max,
                                          std::string_view& absolute_path,
                                          std::size_t& bytes_decoded) {
    std::size_t i = 0;
    char* p = &buffer[off];
    if (off >= max || p[i++] != constants::character::kSlash) return false;
    const std::size_t len = max - off;
    while (i < len) {
      uint8_t c = static_cast<uint8_t>(p[i]);
      if (c == constants::character::kSpace ||
          c == constants::character::kQuestion) {
        break;
      }
      if (c == constants::character::kPercent) {
        if (i + 2 >= len) return false;
        if (!helpers::is_hex_digit(static_cast<uint8_t>(p[i + 1])) ||
            !helpers::is_hex_digit(static_cast<uint8_t>(p[i + 2]))) {
          return false;
        }
        i += 3;
        continue;
      }
      if (!helpers::is_pchar(c) && c != constants::character::kSlash) {
        return false;
      }
      i++;
    }
    absolute_path = std::string_view(p, bytes_decoded = i);
    return true;
  }
  // +-------------------------------------------------------------------------+
  // |                                                              query-part |
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
  static inline common::deserialize_result decode_query_part(
      char* buffer, std::size_t off, std::size_t max,
      std::string_view& query_part, std::size_t& bytes_decoded) {
    if (off >= max) return common::deserialize_result::kMoreBytesNeeded;
    char* p = &buffer[off];
    std::size_t i = 0;
    const std::size_t len = max - off;
    while (i < len) {
      uint8_t c = static_cast<uint8_t>(p[i]);
      if (c == constants::character::kSpace) break;
      if (c == constants::character::kPercent) {
        if (i + 2 >= len) return common::deserialize_result::kMoreBytesNeeded;
        if (!helpers::is_hex_digit(static_cast<uint8_t>(p[i + 1])) ||
            !helpers::is_hex_digit(static_cast<uint8_t>(p[i + 2]))) {
          return common::deserialize_result::kInvalidSource;
        }
        i += 3;
        continue;
      }
      if (!helpers::is_pchar(c) && c != constants::character::kSlash &&
          c != constants::character::kQuestion) {
        return common::deserialize_result::kInvalidSource;
      }
      i++;
    }
    query_part = std::string_view(p, bytes_decoded = i);
    return common::deserialize_result::kSucceeded;
  }
  // ---------------------------------------------------------------------------
  // ATTRIBUTEs                                                      ( private )
  //
  char* buf_ = nullptr;
  std::size_t sze_ = 0;
  target target_ = target::kUnknown;
  std::string_view absolute_path_;
  std::string_view query_part_;
  std::string_view method_;
  headers headers_;
  std::size_t headers_used_ = 0;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
