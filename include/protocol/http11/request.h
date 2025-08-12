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
#include "message.h"
#include "helpers.h"

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
  request() {
    buf_sz_ = kDefaultResponseFullSizeInMemory;
    bod_sz_ = kDefaultResponseBodySizeInMemory;
    slh_sz_ = buf_sz_ - bod_sz_;
    buf_ = (uint8_t*)malloc(buf_sz_);
  }
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
  inline transport::process_result deserialize(void* buf, std::size_t len) {
    static const auto eoh_len = sizeof(constants::string::kEndOfHeaders) - 1;
    static const auto crlf_len = sizeof(constants::string::kCrLf) - 1;
    const char* body_data = nullptr;
    std::size_t body_data_start = 0, body_bytes = 0;
    if (!body_expected_) {
      std::optional<std::size_t> hdr_end;  // headers section end.
      std::optional<std::size_t> rln_end;  // request line end.
      std::optional<std::size_t> mtd_end;  // method end.
      std::optional<std::size_t> pth_end;  // path end.
      std::optional<std::size_t> ver_end;  // http version end.
      // first of all, let's check if we're under limits..
      if ((slh_sz_ - cur_) < len) return transport::process_result::kError;
      memcpy(&buf_[cur_], buf, len);
      cur_ += len;
      // let's detect the [end-of-headers] position..
      hdr_end = get(buf_, cur_, constants::string::kEndOfHeaders, eoh_len);
      if (!hdr_end.has_value()) {
        return transport::process_result::kNeedMoreBytes;
      }
      // let's detect the [end-of-request-line] position..
      rln_end = get(buf_, hdr_end.value(), constants::string::kCrLf, crlf_len);
      if (!(hdr_end.value() - rln_end.value())) {
        return transport::process_result::kError;
      }
      // let's check [request-line] section..
      if (!check_request_line(rln_end, mtd_end, pth_end, ver_end)) {
        return transport::process_result::kError;
      }
      // let's check [headers] section..
      if (!check_headers(ver_end, hdr_end)) {
        return transport::process_result::kError;
      }
      // let's check now for body bytes..
      body_data_start = hdr_end.value() + eoh_len;
      if (body_expected_) {
        if (body_size_expected_) {
          body_data = (const char*)&buf_[body_data_start];
          body_bytes = cur_ - body_data_start;
        }
      }
      // let's prepare internal structures..
      auto const off = rln_end.value() + 2;
      auto hdr_buf_size = buf_sz_ - off - bod_sz_;
      auto hdr_buf = &buf_[off];
      auto bod_buf = &buf_[off + hdr_buf_size];
      message_.prepare((char*)hdr_buf, hdr_buf_size, (char*)bod_buf, bod_sz_,
                       hdr_end.value() - rln_end.value());
      // let's add body bytes (if any)..
      if (body_bytes) {
        message_.add_body(body_data, body_bytes);
        body_size_received_ += body_bytes;
        body_expected_ = body_size_received_ < body_size_expected_;
      }
    }
    // let's return result..
    return !body_expected_ ? transport::process_result::kCompleted
                           : transport::process_result::kNeedMoreBytes;
  }
  inline void reset() {
    cur_ = 0;
    body_expected_ = false;
    body_size_expected_ = 0;
    body_size_received_ = 0;
    slh_decoding_ = true;
    message_.reset();
    target_.reset();
    method_.reset();
  }
  inline auto const& get_target() const { return target_.value(); }
  inline auto const& get_method() const { return method_.value(); }
  inline hash_map<std::string_view, std::string_view> get_headers() const {
    return message_.get_headers();
  }
  inline std::optional<std::string_view> get_header(std::string_view k) const {
    return message_.get_header(k);
  }
  inline request& add_header(std::string_view k, std::string_view v) {
    message_.add_header(k, v);
    return *this;
  }
  template <typename T>
    requires std::is_arithmetic_v<T>
  inline request& add_header(std::string_view k, const T& v) {
    return add_header(k, std::to_string(v));
  }
  inline request& remove_header(std::string_view key) {
    message_.remove_header(key);
    return *this;
  }

 private:
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  inline bool check_request_line(std::optional<std::size_t>& rln_end,
                                 std::optional<std::size_t>& mtd_end,
                                 std::optional<std::size_t>& pth_end,
                                 std::optional<std::size_t>& ver_end) {
    std::size_t i = 0;
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
    while (i < rln_end.value()) {
      if (buf_[i] == constants::character::kSpace) break;
      if (!helpers::is_token(buf_[i])) return false;
      buf_[i++] = buf_[i];
    }
    mtd_end = i++;
    std::string_view method_str((const char*)buf_, mtd_end.value());
    switch (method_str.length()) {
      case 3:
        if (!method_str.compare(constants::method::kGet)) {
          method_ = method::kGet;
        } else if (!method_str.compare(constants::method::kPut)) {
          method_ = method::kPut;
        } else {
          return false;
        }
        break;
      case 4:
        if (!method_str.compare(constants::method::kHead)) {
          method_ = method::kHead;
        } else if (!method_str.compare(constants::method::kPost)) {
          method_ = method::kPost;
        } else {
          return false;
        }
        break;
      case 5:
        if (!method_str.compare(constants::method::kTrace)) {
          method_ = method::kTrace;
        } else {
          return false;
        }
        break;
      case 6:
        if (!method_str.compare(constants::method::kDelete)) {
          method_ = method::kDelete;
        } else {
          return false;
        }
        break;
      case 7:
        if (!method_str.compare(constants::method::kConnect)) {
          method_ = method::kConnect;
        } else if (!method_str.compare(constants::method::kOptions)) {
          method_ = method::kOptions;
        } else {
          return false;
        }
        break;
      default:
        return false;
        break;
    }
    if (i >= rln_end.value()) return false;
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
    }
 else if (auto abs = try_get_absolute_path(i, rln_end.value()); abs) {
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
   target_ = target::create_as_origin_form(
     abs.value(), try_get_query_part(i, rln_end.value()));
    }
 else {
   // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
   // [to-do] -> add support for this!
   // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
   // absolute-form
   // parse it as URI..
    }
    pth_end = i++;
    // +----------------+------------------------------------------------------+
    // | HTTP-version   | HTTP-name "/" DIGIT "." DIGIT                        |
    // | HTTP-name      | %s"HTTP"                                             |
    // +----------------+------------------------------------------------------+
    // | source: https://datatracker.ietf.org/doc/html/rfc9110 |               |
    // +-----------------------------------------------------------------------+
    if ((rln_end.value() - i) < kHttpVersionLen) return false;
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
    ver_end = i + 8;
    version_ = std::string_view((const char*)&buf_[pth_end.value() + 1],
      ver_end.value() - pth_end.value() - 1);
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
  inline bool check_headers(std::optional<std::size_t>& ver_end,
                            std::optional<std::size_t>& hdr_end) {
    std::size_t clength_count = 0;
    std::size_t tencoding_count = 0;
    std::size_t i = ver_end.value() + sizeof(constants::string::kCrLf) - 1;
    std::size_t end = hdr_end.value() + 2;
    while (i < end) {
      std::size_t beg = i;
      while (i < end && buf_[i] != constants::character::kColon) {
        buf_[i++] = helpers::tolower_ascii(buf_[i]);
      }
      if (i >= end) return false;
      std::size_t colon = i;
      while (i < end && buf_[i] != constants::character::kCr) i++;
      if (i >= end) return false;
      if (++i >= end || buf_[i] != constants::character::kLf) return false;
      std::string_view name((const char*)&buf_[beg], colon - beg);
      std::string_view value((const char*)&buf_[colon + 1], (i - 2) - colon);
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
        if (!helpers::is_digit(ows_value)) return false;
        auto first = ows_value.data();
        auto last = ows_value.data() + ows_value.size();
        auto [p, e] = std::from_chars(first, last, body_size_expected_);
        if (e != std::errc{}) return false;
        body_expected_ = true;
      } else if (!name.compare(headers::kTransferEncoding)) {
        if ((++tencoding_count > 1) || (clength_count > 0)) return false;
      }
      i++;
    }
    return true;
  }
  inline std::optional<std::string> try_get_absolute_path(std::size_t& i,
                                                          std::size_t len) {
    std::string path;
    char current_character;
    if (buf_[i] != constants::character::kSlash) return std::nullopt;
    while (i < len) {
      if (buf_[i] == constants::character::kPercent) {
        if (i + 2 >= len) return std::nullopt;
        if (!helpers::is_hex_digit(buf_[i + 1]) ||
            !helpers::is_hex_digit(buf_[i + 2]))
          return std::nullopt;
        current_character = static_cast<char>(
            std::stoi(std::string((const char*)&buf_[i + 1], 2), nullptr, 16));
        i += 3;
      } else {
        current_character = buf_[i];
        if (current_character == constants::character::kSpace ||
            current_character == constants::character::kQuestion) {
          break;
        }
      }
      if (!helpers::is_pchar(current_character) &&
          current_character != constants::character::kSlash) {
        return std::nullopt;
      }
      path.push_back(current_character);
      i++;
    }
    return path;
  }
  inline std::optional<std::string> try_get_query_part(std::size_t& i,
                                                       std::size_t len) {
    std::string query;
    char current_character;
    if (buf_[i] != constants::character::kQuestion) return std::nullopt;
    while (i < len) {
      if (buf_[i] == constants::character::kPercent) {
        if (i + 2 >= len) return std::nullopt;
        if (!helpers::is_hex_digit(buf_[i + 1]) ||
            !helpers::is_hex_digit(buf_[i + 2]))
          return std::nullopt;
        current_character = static_cast<char>(
            std::stoi(std::string((const char*)&buf_[i + 1], 2), nullptr, 16));
        i += 3;
      } else {
        current_character = buf_[i];
        if (current_character == constants::character::kSpace) break;
      }
      if (!helpers::is_pchar(current_character) &&
          current_character != constants::character::kSlash &&
          current_character != constants::character::kQuestion) {
        return std::nullopt;
      }
      query.push_back(current_character);
      i++;
    }
    return query;
  }
  inline std::optional<std::size_t> get(const uint8_t* const str,
                                        std::size_t str_len,
                                        const uint8_t* const pattern,
                                        std::size_t pattern_len) {
    for (std::size_t i = 0; i < str_len; ++i) {
      std::size_t j = 0;
      while (j < pattern_len) {
        if (i + j == str_len) return {};
        if (str[i + j] != pattern[j]) break;
        j++;
      }
      if (j == pattern_len) return i;
    }
    return {};
  }
  // ___________________________________________________________________________
  // CONSTANTs                                                       ( private )
  //
  static constexpr std::size_t kDefaultResponseFullSizeInMemory = 8192;  // 8kb.
  static constexpr std::size_t kDefaultResponseBodySizeInMemory = 4096;  // 4kb.
  static constexpr std::size_t kHttpVersionLen = 8;
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  bool body_expected_ = false;
  std::size_t body_size_expected_ = 0;
  std::size_t body_size_received_ = 0;
  bool slh_decoding_ = true;
  uint8_t* buf_ = nullptr;
  std::size_t buf_sz_ = 0;
  std::size_t slh_sz_ = 0;
  std::size_t bod_sz_ = 0;
  std::size_t cur_ = 0;
  std::optional<method> method_;
  std::optional<target> target_;
  std::string_view version_;
  message message_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
