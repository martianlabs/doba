//      _       _           
//   __| | ___ | |__   __ _ 
//  / _` |/ _ \| '_ \ / _` |
// | (_| | (_) | |_) | (_| |
//  \__,_|\___/|_.__/ \__,_|
// 
// Copyright 2025 martianLabs
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
  ~request() { free(buf_); }
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
      if (cursor_ - 1 >= 2 && buf_[i] == constants::character::kCr &&
          buf_[i + 1] == constants::character::kLf) {
        i += 2;
        break;
      }
    }
    std::size_t k_start = i, v_start = i;
    common::hash_map<std::string_view, std::string_view> out;
    while (i < cursor_) {
      if (cursor_ - 1 >= 4 && buf_[i] == constants::character::kCr &&
          buf_[i + 1] == constants::character::kLf &&
          buf_[i + 2] == constants::character::kCr &&
          buf_[i + 3] == constants::character::kLf) {
        break;
      }
      switch (buf_[i]) {
        case constants::character::kColon:
          if (searching_for_key) {
            searching_for_key = false;
            v_start = i + 1;
          }
          break;
        case constants::character::kCr:
          if (searching_for_key) return out;
          out.insert(std::make_pair(
              std::string_view(&buf_[k_start], (v_start - 1) - k_start),
              helpers::ows_ltrim(helpers::ows_rtrim(
                  std::string_view(&buf_[v_start], i - v_start)))));
          searching_for_key = true;
          k_start = i + 2;
          j = 0;
          i++;
          break;
        default:
          break;
      }
      i++;
    }
    return out;
  }
  // ___________________________________________________________________________
  // STATIC-METHODs                                                   ( public )
  //
  static auto from(const char* const buf, std::size_t len) {
    auto instance = std::shared_ptr<request>(new request(buf, len));
    if (instance) {
      std::size_t i = 0;
      if (instance->check_request_line(i)) {
        if (!instance->check_headers(i)) {
          instance.reset();
        }
      } else {
        instance.reset();
      }
    }
    return instance;
  }

 private:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                        ( private )
  //
  request(const char* const buf, std::size_t len) {
    buf_ = NULL;
    size_ = 0;
    cursor_ = 0;
    method_ = method::kUnknown;
    target_ = target::kUnknown;
    if (buf_ = (char*)malloc(len)) {
      memcpy(buf_, buf, len);
      cursor_ = size_ = len;
    }
  }
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  bool check_request_line(std::size_t& i) {
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
    while (i < cursor_) {
      if (buf_[i] == constants::character::kSpace) break;
      if (!helpers::is_token(buf_[i++])) return false;
    }
    if (i == cursor_) return false;
    std::string_view method(buf_, i++);
    switch (method.length()) {
      case 3:
        if (!method.compare(constants::method::kGet)) {
          method_ = method::kGet;
        } else if (!method.compare(constants::method::kPut)) {
          method_ = method::kPut;
        } else {
          return false;
        }
        break;
      case 4:
        if (!method.compare(constants::method::kHead)) {
          method_ = method::kHead;
        } else if (!method.compare(constants::method::kPost)) {
          method_ = method::kPost;
        } else {
          return false;
        }
        break;
      case 5:
        if (!method.compare(constants::method::kTrace)) {
          method_ = method::kTrace;
        } else {
          return false;
        }
        break;
      case 6:
        if (!method.compare(constants::method::kDelete)) {
          method_ = method::kDelete;
        } else {
          return false;
        }
        break;
      case 7:
        if (!method.compare(constants::method::kConnect)) {
          method_ = method::kConnect;
        } else if (!method.compare(constants::method::kOptions)) {
          method_ = method::kOptions;
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
    if (method_ == method::kConnect) {
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
    } else if (method_ == method::kOptions) {
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
    } else if (std::string path; try_to_get_absolute_path(i, path)) {
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
      target_ = target::kOriginForm;
      absolute_path_ = path;
      if (std::string query;  try_to_get_query_part(i, query)) {
        query_ = query;
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
    if (i >= cursor_) return false;
    if (buf_[i++] != constants::character::kSpace) return false;
    if (cursor_ - i < kHttpVersionLen) return false;
    if (buf_[i] != constants::character::kHUpperCase ||
        buf_[i + 1] != constants::character::kTUpperCase ||
        buf_[i + 2] != constants::character::kTUpperCase ||
        buf_[i + 3] != constants::character::kPUpperCase ||
        buf_[i + 4] != constants::character::kSlash ||
        !helpers::is_digit(buf_[i + 5]) ||
        buf_[i + 6] != constants::character::kDot ||
        !helpers::is_digit(buf_[i + 7])) {
      return false;
    }
    i += kHttpVersionLen;
    if (cursor_ - i < 2) return false;
    if (buf_[i] != constants::character::kCr ||
        buf_[i + 1] != constants::character::kLf) {
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
  bool check_headers(std::size_t& i) {
    std::size_t clength_count = 0;
    std::size_t tencoding_count = 0;
    int host_count = 0;
    while (i < cursor_) {
      std::size_t beg = i;
      while (i < cursor_ && buf_[i] != constants::character::kColon) {
        buf_[i++] = helpers::tolower_ascii(buf_[i]);
      }
      if (i >= cursor_) return false;
      std::size_t colon = i;
      while (i < cursor_ && buf_[i] != constants::character::kCr) i++;
      if (i++ >= cursor_) return false;
      if (i >= cursor_ || buf_[i] != constants::character::kLf) return false;
      if (i++ >= cursor_) return false;
      std::string_view name((const char*)&buf_[beg], colon - beg);
      std::string_view value((const char*)&buf_[colon + 1], (i - 3) - colon);
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

        /*
        pepe
        */

        /*
        auto [p, e] = std::from_chars(first, last, body_size_expected_);
        if (e != std::errc{}) return false;
        body_processing_ = body_size_expected_ > 0;
        */

        /*
        pepe fin
        */

      } else if (!name.compare(headers::kTransferEncoding)) {
        if ((++tencoding_count > 1) || (clength_count > 0)) return false;
      } else if (!name.compare(headers::kHost)) {
        host_count++;
      }
      if (cursor_ - i >= 2) {
        if (buf_[i] == constants::character::kCr &&
            buf_[i + 1] == constants::character::kLf) {
          i += 2;
          break;
        }
      }
      i++;
    }
    return host_count == 1;
  }
  bool try_to_get_absolute_path(std::size_t& i, std::string& path) {
    char ch;
    if (i >= cursor_) return false;
    if (buf_[i] != constants::character::kSlash) return false;
    while (i < cursor_) {
      if (buf_[i] == constants::character::kPercent) {
        if (i + 2 >= cursor_) return false;
        if (!helpers::is_hex_digit(buf_[i + 1]) ||
            !helpers::is_hex_digit(buf_[i + 2])) {
          return false;
        }
        auto ptr = (const char*)&buf_[i + 1];
        ch = static_cast<char>(std::stoi(std::string(ptr, 2), nullptr, 16));
        i += 3;
      } else {
        ch = buf_[i];
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
  bool try_to_get_query_part(std::size_t& i, std::string& query) {
    char ch;
    if (i >= cursor_) return false;
    if (buf_[i] != constants::character::kQuestion) return false;
    while (i < cursor_) {
      if (buf_[i] == constants::character::kPercent) {
        if (i + 2 >= cursor_) return false;
        if (!helpers::is_hex_digit(buf_[i + 1]) ||
            !helpers::is_hex_digit(buf_[i + 2])) {
          return false;
        }
        auto ptr = (const char*)&buf_[i + 1];
        ch = static_cast<char>(std::stoi(std::string(ptr, 2), nullptr, 16));
        i += 3;
      } else {
        ch = buf_[i];
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
  char* buf_;
  std::size_t size_;
  std::size_t cursor_;
  method method_;
  target target_;
  std::string absolute_path_;
  std::string query_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
