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

#include "method.h"
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
    buffer_ = (uint8_t*)malloc(kMaxSizeInMemory);
    method_ = method::kUnknown;
  }
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
  inline transport::process_result deserialize(void* buffer, uint32_t length) {
    std::optional<uint32_t> hdr_end;  // headers section end.
    std::optional<uint32_t> rln_end;  // request line end.
    std::optional<uint32_t> mtd_end;  // method end.
    std::optional<uint32_t> pth_end;  // path end.
    std::optional<uint32_t> ver_end;  // http version end.
    // first of all, let's check if we're under limits..
    if ((kMaxSizeInMemory - cursor_) < length) {
      return transport::process_result::kError;
    }
    memcpy(&buffer_[cursor_], buffer, length);
    cursor_ += length;
    // let's detect the [end-of-headers] position..
    hdr_end = get(buffer_, cursor_, constants::string::kEndOfHeaders,
                  sizeof(constants::string::kEndOfHeaders) - 1);
    if (!hdr_end.has_value()) return transport::process_result::kNeedMoreBytes;
    // let's detect the [end-of-request-line] position..
    rln_end = get(buffer_, hdr_end.value(), constants::string::kCrLf,
                  sizeof(constants::string::kCrLf) - 1);
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
    // let's return result..
    return transport::process_result::kCompleted;
  }
  inline void reset() {
    cursor_ = 0;
    headers_.clear();
    path_.clear();
    method_ = method::kUnknown;
  }
  inline auto get_path() const { return path_; }
  inline auto get_method() const { return method_; }
  inline auto get_headers() const { return headers_; }

 private:
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  inline bool check_request_line(std::optional<uint32_t>& rln_end,
                                 std::optional<uint32_t>& mtd_end,
                                 std::optional<uint32_t>& pth_end,
                                 std::optional<uint32_t>& ver_end) {
    uint32_t i = 0;
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
      if (buffer_[i] == constants::character::kSpace) break;
      if (!helpers::is_token(buffer_[i])) return false;
      buffer_[i++] = buffer_[i];
    }
    mtd_end = i++;
    std::string_view method_str((const char*)buffer_, mtd_end.value());
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
    // +----------------+------------------------------------------------------+
    // | Field          | Definition                                           |
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
    char character = buffer_[i++];
    if (method_ == method::kConnect) {
      // [to-do] -> add support for this!
      // authority-form
      // wait for: host:port
    } else if (method_ == method::kOptions) {
      // [to-do] -> add support for this!
      // asterisk-form
    } else if (character == constants::character::kSlash) {
      // origin-form
      path_.push_back(character);
      while (i < rln_end.value()) {
        if (buffer_[i] == constants::character::kPercent) {
          if (i + 2 >= rln_end.value()) return false;
          if (!helpers::is_hex_digit(buffer_[i + 1]) ||
              !helpers::is_hex_digit(buffer_[i + 2]))
            return false;
          character = static_cast<char>(std::stoi(
              std::string((const char*)&buffer_[i + 1], 2), nullptr, 16));
          i += 3;
        } else if ((character = buffer_[i]) == constants::character::kSpace) {
          break;
        }
        i++;
        if (!helpers::is_pchar(character) &&
            character == constants::character::kSlash &&
            character == constants::character::kQuestion) {
          return false;
        }
        path_.push_back(character);
      }
    } else {
      // [to-do] -> add support for this!
      // absolute-form
      // parse it as URI..
    }
    pth_end = i++;
    // +----------------+------------------------------------------------------+
    // | HTTP-version   | HTTP-name "/" DIGIT "." DIGIT                        |
    // | HTTP-name      | %s"HTTP"                                             |
    // +----------------+------------------------------------------------------+
    // | source: https://datatracker.ietf.org/doc/html/rfc9110                 |
    // +-----------------------------------------------------------------------+
    if ((rln_end.value() - i) < kHttpVersionLen) return false;
    if (buffer_[i] != constants::character::kHUpperCase ||
        buffer_[i + 1] != constants::character::kTUpperCase ||
        buffer_[i + 2] != constants::character::kTUpperCase ||
        buffer_[i + 3] != constants::character::kPUpperCase ||
        buffer_[i + 4] != constants::character::kSlash ||
        !helpers::is_digit(buffer_[i + 5]) ||
        buffer_[i + 6] != constants::character::kDot ||
        !helpers::is_digit(buffer_[i + 7])) {
      return false;
    }
    ver_end = i + 8;
    version_ = std::string_view((const char*)&buffer_[pth_end.value() + 1],
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
  inline bool check_headers(std::optional<uint32_t>& ver_end,
                            std::optional<uint32_t>& hdr_end) {
    uint32_t i = ver_end.value() + sizeof(constants::string::kCrLf) - 1;
    uint32_t end = hdr_end.value() + 2;
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
  static constexpr uint32_t kMaxSizeInMemory = 16384;  // 16kb.
  static constexpr uint32_t kHttpVersionLen = 8;
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  uint8_t* buffer_ = nullptr;
  uint32_t cursor_ = 0;
  std::string path_;
  method method_;
  std::string_view version_;
  std::unordered_map<std::string_view, std::string_view> headers_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
