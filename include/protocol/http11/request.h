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
  request() { buffer_ = (uint8_t*)malloc(kMaxRequestLength); }
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
  transport::process_result deserialize(void* buffer, uint32_t length) {
    // first of all, let's check if we're under limits..
    if ((kMaxRequestLength - cursor_) < length)
      return transport::process_result::kError;
    memcpy(&buffer_[cursor_], buffer, length);
    cursor_ += length;
    // let's detect the [end-of-headers] position..
    hdr_end_ = get(buffer_, cursor_, constants::string::kEndOfHeaders,
                   sizeof(constants::string::kEndOfHeaders) - 1);
    if (!hdr_end_.has_value()) return transport::process_result::kNeedMoreBytes;
    // let's detect the [end-of-request-line] position..
    rln_end_ = get(buffer_, hdr_end_.value(), constants::string::kCrLf,
                   sizeof(constants::string::kCrLf) - 1);
    if (!(hdr_end_.value() - rln_end_.value()))
      return transport::process_result::kError;
    // let's check [request-line] section..
    if (!check_request_line()) return transport::process_result::kError;
    // let's check [headers] section..
    if (!check_headers()) return transport::process_result::kError;
    // let's return result..
    return transport::process_result::kCompleted;
  }
  void reset() {
    cursor_ = 0;
    hdr_end_.reset();
    rln_end_.reset();
    mtd_end_.reset();
    pth_end_.reset();
    ver_end_.reset();
    headers_.clear();
  }
  const auto& get_headers() const { return headers_; }

 private:
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  inline bool check_request_line() {
    uint32_t i = 0;
    // +---------+-------------------------------------------------------------+
    // | Campo   | Definición                                                  |
    // +---------+-------------------------------------------------------------+
    // | method  | token                                                       |
    // | token   | 1*tchar                                                     |
    // | tchar   | "!" / "#" / "$" / "%" / "&" / "'" / "*" /                   |
    // |         | "+" / "-" / "." / "^" / "_" / "`" / "|" / "~"               |
    // |         | / DIGIT / ALPHA                                             |
    // +---------+-------------------------------------------------------------+
    // | source: https://datatracker.ietf.org/doc/html/rfc9110                 |
    // +----------------+------------------------------------------------------+
    while (i < rln_end_.value()) {
      if (buffer_[i] == constants::character::kSpace) break;
      if (!helpers::is_token(buffer_[i])) return false;
      buffer_[i++] = std::tolower(buffer_[i]);
    }
    mtd_end_ = i++;
    // +----------------+------------------------------------------------------+
    // | Campo          | Definición                                           |
    // +----------------+------------------------------------------------------+
    // | path           | segment *( "/" segment )                             |
    // | segment        | *pchar                                               |
    // | pchar          | unreserved / pct-encoded / sub-delims / ":" / "@"    |
    // | unreserved     | ALPHA / DIGIT / "-" / "." / "_" / "~"                |
    // | pct-encoded    | "%" HEXDIG HEXDIG                                    |
    // | sub-delims     | "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" /      |
    // |                | "," / ";" / "="                                      |
    // +----------------+------------------------------------------------------+
    // | source: https://datatracker.ietf.org/doc/html/rfc9110                 |
    // +----------------+------------------------------------------------------+
    if (buffer_[i++] != constants::character::kSlash) return false;
    while (i < rln_end_.value()) {
      if (buffer_[i] == constants::character::kSpace) break;
      if (buffer_[i] == constants::character::kPercent) {
        if (i + 2 >= rln_end_.value()) return false;
        if (!helpers::is_hex_digit(buffer_[i + 1]) ||
            !helpers::is_hex_digit(buffer_[i + 2]))
          return false;
        i += 3;
        continue;
      } else if (helpers::is_pchar(buffer_[i]) ||
                 buffer_[i] == constants::character::kSlash ||
                 buffer_[i] == constants::character::kQuestion)
        i++;
      else
        return false;
    }
    pth_end_ = i++;
    // +----------------+------------------------------------------------------+
    // | HTTP-version   | HTTP-name "/" DIGIT "." DIGIT                        |
    // | HTTP-name      | %s"HTTP"                                             |
    // +----------------+------------------------------------------------------+
    // | source: https://datatracker.ietf.org/doc/html/rfc9110                 |
    // +-----------------------------------------------------------------------+
    if ((rln_end_.value() - i) < kHttpVersionLen) return false;
    if (buffer_[i] != constants::character::kHUpperCase ||
        buffer_[i + 1] != constants::character::kTUpperCase ||
        buffer_[i + 2] != constants::character::kTUpperCase ||
        buffer_[i + 3] != constants::character::kPUpperCase ||
        buffer_[i + 4] != constants::character::kSlash ||
        !helpers::is_digit(buffer_[i + 5]) ||
        buffer_[i + 6] != constants::character::kDot ||
        !helpers::is_digit(buffer_[i + 7]))
      return false;
    ver_end_ = i + 8;
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
  inline bool check_headers() {
    uint32_t i = ver_end_.value() + sizeof(constants::string::kCrLf) - 1;
    uint32_t end = hdr_end_.value() + 2;
    while (i < end) {
      uint32_t beg = i;
      while (i < end && buffer_[i] != constants::character::kColon) {
        buffer_[i] = std::tolower(buffer_[i]);
        i++;
      }
      if (i >= end) return false;
      uint32_t colon = i;
      while (i < end && buffer_[i] != constants::character::kCr) i++;
      if (i >= end) return false;
      if (++i >= end || buffer_[i] != constants::character::kLf) return false;
      std::string_view name((const char*)&buffer_[beg], colon - beg);
      std::string_view value((const char*)&buffer_[colon + 1], (i - 2) - colon);
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
      headers_[name] = helpers::ows_ltrim(helpers::ows_rtrim(value));
      i++;
    }
    return true;
  }
  inline std::optional<uint32_t> get(const uint8_t* const str, uint32_t str_len,
                                     const uint8_t* const pattern,
                                     uint32_t pattern_len) {
    for (uint32_t i = 0; i < str_len; ++i) {
      uint32_t j = 0;
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
  static constexpr uint32_t kMaxRequestLength = 16384;  // 16kb.
  static constexpr uint32_t kHttpVersionLen = 8;
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  uint8_t* buffer_ = nullptr;
  uint32_t cursor_ = 0;
  std::optional<uint32_t> hdr_end_;  // headers section end.
  std::optional<uint32_t> rln_end_;  // request line end.
  std::optional<uint32_t> mtd_end_;  // method end.
  std::optional<uint32_t> pth_end_;  // path end.
  std::optional<uint32_t> ver_end_;  // http version end.
  std::unordered_map<std::string_view, std::string_view> headers_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
