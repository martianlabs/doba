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

#include "method.h"
#include "target.h"
#include "helpers.h"
#include "headers.h"
#include "common/hash_map.h"

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// request                                                             ( class )
// -----------------------------------------------------------------------------
// This class holds for the http 1.1 request implementation.
// -----------------------------------------------------------------------------
// =============================================================================
class request {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  request(const request&) = delete;
  request(request&&) noexcept = delete;
  ~request() { free(buffer_); }
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  request& operator=(const request&) = delete;
  request& operator=(request&&) noexcept = delete;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  auto get_method() const { return method_; }
  auto get_target() const { return target_; }
  auto get_absolute_path() const { return absolute_path_; }
  auto get_query() const { return query_; }
  auto get_headers() const {
    bool searching_for_key = true;
    std::size_t i = 0, j = 0;
    for (; i < cursor_; i++) {
      if (cursor_ - 1 >= 2 && buffer_[i] == constants::character::kCr &&
          buffer_[i + 1] == constants::character::kLf) {
        i += 2;
        break;
      }
    }
    std::size_t k_start = i, v_start = i;
    common::hash_map<std::string_view, std::string_view> out;
    while (i < cursor_) {
      switch (buffer_[i]) {
        case constants::character::kColon:
          if (searching_for_key) {
            searching_for_key = false;
            v_start = i + 1;
          }
          break;
        case constants::character::kCr:
          if (searching_for_key) return out;
          out.insert(std::make_pair(
              std::string_view(&buffer_[k_start], (v_start - 1) - k_start),
              helpers::ows_ltrim(helpers::ows_rtrim(
                  std::string_view(&buffer_[v_start], i - v_start)))));
          searching_for_key = true;
          k_start = i + 2;
          j = 0;
          i++;
          break;
        default:
          break;
      }
      if (cursor_ - 1 >= 4 && buffer_[i] == constants::character::kCr &&
          buffer_[i + 1] == constants::character::kLf &&
          buffer_[i + 2] == constants::character::kCr &&
          buffer_[i + 3] == constants::character::kLf) {
        break;
      }
      i++;
    }
    return out;
  }
  auto has_body() const { return has_body_; }
  auto get_body_length() const { return body_length_; }
  // ___________________________________________________________________________
  // STATIC-METHODs                                                   ( public )
  //
  static inline auto from(char* buf, std::size_t len) {
    method method = method::kUnknown;
    target target = target::kUnknown;
    std::shared_ptr<request> req;
    bool ebd = false;
    std::size_t ebdlen = 0;
    std::string absolute_path;
    std::string query;
    std::size_t i = 0;
    if (check_request_line(buf, len, method, target, absolute_path, query, i)) {
      if (check_headers(buf, len, i, ebd, ebdlen)) {
        req = std::shared_ptr<request>(new request(
            buf, len, method, target, absolute_path, query, ebd, ebdlen));
      }
    }
    return req;
  }

 private:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                        ( private )
  //
  request(const char* const buf, std::size_t len, method method, target target,
          const std::string& absolute_path, const std::string& query,
          bool has_body, std::size_t body_length) {
    size_ = 0;
    cursor_ = 0;
    method_ = method;
    target_ = target;
    body_length_ = body_length;
    if (has_body_ = has_body) {
      if (body_length_) {

        /*
        pepe
        */

        /*
        pepe fin
        */

      } else {
        // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
        // [to-do] -> add support for this!
        // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
      }
    }
    absolute_path_ = absolute_path;
    query_ = query;
    if (buffer_ = (char*)malloc(len)) {
      memcpy(buffer_, buf, len);
      cursor_ = size_ = len;
    }
  }
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  static inline bool check_request_line(char* buffer, const std::size_t& cursor,
                                        method& method, target& target,
                                        std::string& absolute_path,
                                        std::string& query, std::size_t& i) {
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
    constexpr std::size_t kHttpVersionLen = 8;
    while (i < cursor) {
      if (buffer[i] == constants::character::kSpace) break;
      if (!helpers::is_token(buffer[i++])) return false;
    }
    if (i == cursor) return false;
    std::string_view str_method(buffer, i++);
    switch (str_method.length()) {
      case 3:
        if (!str_method.compare(constants::method::kGet)) {
          method = method::kGet;
        } else if (!str_method.compare(constants::method::kPut)) {
          method = method::kPut;
        } else {
          return false;
        }
        break;
      case 4:
        if (!str_method.compare(constants::method::kHead)) {
          method = method::kHead;
        } else if (!str_method.compare(constants::method::kPost)) {
          method = method::kPost;
        } else {
          return false;
        }
        break;
      case 5:
        if (!str_method.compare(constants::method::kTrace)) {
          method = method::kTrace;
        } else {
          return false;
        }
        break;
      case 6:
        if (!str_method.compare(constants::method::kDelete)) {
          method = method::kDelete;
        } else {
          return false;
        }
        break;
      case 7:
        if (!str_method.compare(constants::method::kConnect)) {
          method = method::kConnect;
        } else if (!str_method.compare(constants::method::kOptions)) {
          method = method::kOptions;
        } else {
          return false;
        }
        break;
      default:
        return false;
        break;
    }
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
    if (method == method::kConnect) {
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
    } else if (method == method::kOptions) {
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
    } else if (std::string path;
               try_to_get_absolute_path(buffer, cursor, i, path)) {
      // +-------------------------------------------------------------------+
      // |                                                       origin-form |
      // +----------------+--------------------------------------------------+
      // | Field          | Definition                                       |
      // +----------------+--------------------------------------------------+
      // | path           | segment *( "/" segment )                         |
      // | segment        | *pchar                                           |
      // | pchar          | unreserved / pct-encoded / sub-delims / ":" / "@"|
      // | unreserved     | ALPHA / DIGIT / "-" / "." / "_" / "~"            |
      // | pct-encoded    | "%" HEXDIG HEXDIG                                |
      // | sub-delims     | "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" /  |
      // |                | "," / ";" / "="                                  |
      // +----------------+--------------------------------------------------+
      // | source: https://datatracker.ietf.org/doc/html/rfc9110             |
      // +----------------+--------------------------------------------------+
      target = target::kOriginForm;
      absolute_path = path;
      if (std::string qry; try_to_get_query_part(buffer, cursor, i, qry)) {
        query = qry;
      }
    } else {
      // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
      // [to-do] -> add support for this!
      // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
      // absolute-form
      // parse it as URI..
    }
    // +----------------+------------------------------------------------------+
    // | HTTP-version   | HTTP-name "/" DIGIT "." DIGIT                        |
    // | HTTP-name      | %s"HTTP"                                             |
    // +----------------+------------------------------------------------------+
    // | source: https://datatracker.ietf.org/doc/html/rfc9110 |               |
    // +-----------------------------------------------------------------------+
    if (i >= cursor) return false;
    if (buffer[i++] != constants::character::kSpace) return false;
    if (cursor - i < kHttpVersionLen) return false;
    if (buffer[i] != constants::character::kHUpperCase ||
        buffer[i + 1] != constants::character::kTUpperCase ||
        buffer[i + 2] != constants::character::kTUpperCase ||
        buffer[i + 3] != constants::character::kPUpperCase ||
        buffer[i + 4] != constants::character::kSlash ||
        !helpers::is_digit(buffer[i + 5]) ||
        buffer[i + 6] != constants::character::kDot ||
        !helpers::is_digit(buffer[i + 7])) {
      return false;
    }
    i += kHttpVersionLen;
    if (cursor - i < 2) return false;
    if (buffer[i] != constants::character::kCr ||
        buffer[i + 1] != constants::character::kLf) {
      return false;
    }
    i += 2;
    return true;
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
  static inline bool check_headers(char* buffer, const std::size_t& cursor,
                                   std::size_t& i, bool& exp_bdy,
                                   std::size_t& exp_bdy_len) {
    std::size_t clength_count = 0;
    std::size_t tencoding_count = 0;
    int host_count = 0;
    while (i < cursor) {
      std::size_t beg = i;
      while (i < cursor && buffer[i] != constants::character::kColon) {
        buffer[i++] = helpers::tolower_ascii(buffer[i]);
      }
      if (i >= cursor) return false;
      std::size_t colon = i;
      while (i < cursor && buffer[i] != constants::character::kCr) i++;
      if (i++ >= cursor) return false;
      if (i >= cursor || buffer[i] != constants::character::kLf) return false;
      if (i++ >= cursor) return false;
      std::string_view name((const char*)&buffer[beg], colon - beg);
      std::string_view value((const char*)&buffer[colon + 1], (i - 3) - colon);
      // [field-name] validation..
      if (!name.length()) return false;
      for (auto j = 0; j < name.length(); j++) {
        if (!helpers::is_token(name[j])) return false;
      }
      // [field-value] validation..
      for (auto k = 0; k < value.length(); k++) {
        if (!(helpers::is_vchar(value[k]) || helpers::is_obs_text(value[k]) ||
              value[k] == constants::character::kSpace ||
              value[k] == constants::character::kHTab)) {
          return false;
        }
      }
      // [checks] needed by protocol..
      if (!name.compare(headers::kContentLength)) {
        if ((++clength_count > 1) || (tencoding_count > 0)) return false;
        auto ows_value = helpers::ows_ltrim(helpers::ows_rtrim(value));
        if (!ows_value.length() || !helpers::is_digit(ows_value)) return false;
        auto first = ows_value.data();
        auto last = ows_value.data() + ows_value.size();
        auto [p, e] = std::from_chars(first, last, exp_bdy_len);
        if (e != std::errc{}) return false;
        exp_bdy = true;
      } else if (!name.compare(headers::kTransferEncoding)) {
        if ((++tencoding_count > 1) || (clength_count > 0)) return false;
      } else if (!name.compare(headers::kHost)) {
        host_count++;
      }
      if (cursor - i >= 2) {
        if (buffer[i] == constants::character::kCr &&
            buffer[i + 1] == constants::character::kLf) {
          i += 2;
          break;
        }
      }
    }
    return host_count == 1;
  }
  static inline bool try_to_get_absolute_path(char* buffer,
                                              const std::size_t& cursor,
                                              std::size_t& i,
                                              std::string& path) {
    char ch;
    if (i >= cursor) return false;
    if (buffer[i] != constants::character::kSlash) return false;
    while (i < cursor) {
      if (buffer[i] == constants::character::kPercent) {
        if (i + 2 >= cursor) return false;
        if (!helpers::is_hex_digit(buffer[i + 1]) ||
            !helpers::is_hex_digit(buffer[i + 2])) {
          return false;
        }
        auto ptr = (const char*)&buffer[i + 1];
        ch = static_cast<char>(std::stoi(std::string(ptr, 2), nullptr, 16));
        i += 3;
      } else {
        ch = buffer[i];
        if (ch == constants::character::kSpace ||
            ch == constants::character::kQuestion) {
          break;
        }
      }
      if (!helpers::is_pchar(ch) && ch != constants::character::kSlash) {
        return false;
      }
      path.push_back(ch);
      i++;
    }
    return true;
  }
  static inline bool try_to_get_query_part(char* buffer,
                                           const std::size_t& cursor,
                                           std::size_t& i, std::string& query) {
    char ch;
    if (i >= cursor) return false;
    if (buffer[i] != constants::character::kQuestion) return false;
    while (i < cursor) {
      if (buffer[i] == constants::character::kPercent) {
        if (i + 2 >= cursor) return false;
        if (!helpers::is_hex_digit(buffer[i + 1]) ||
            !helpers::is_hex_digit(buffer[i + 2])) {
          return false;
        }
        auto ptr = (const char*)&buffer[i + 1];
        ch = static_cast<char>(std::stoi(std::string(ptr, 2), nullptr, 16));
        i += 3;
      } else {
        ch = buffer[i];
        if (ch == constants::character::kSpace) break;
      }
      if (!helpers::is_pchar(ch) && ch != constants::character::kSlash &&
          ch != constants::character::kQuestion) {
        return false;
      }
      query.push_back(ch);
      i++;
    }
    return true;
  }
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  char* buffer_;
  std::size_t size_;
  std::size_t cursor_;
  method method_;
  target target_;
  std::string absolute_path_;
  std::string query_;
  bool has_body_ = false;
  std::size_t body_length_ = 0;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
