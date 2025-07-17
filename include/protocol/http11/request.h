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
    hdr_end_ = get(buffer_, cursor_, constants::strings::kEndOfHeaders,
                   sizeof(constants::strings::kEndOfHeaders) - 1);
    if (!hdr_end_.has_value()) return transport::process_result::kNeedMoreBytes;
    // let's detect the [end-of-request-line] position..
    rln_end_ = get(buffer_, hdr_end_.value(), constants::strings::kCrLf,
                   sizeof(constants::strings::kCrLf) - 1);
    if (!(hdr_end_.value() - rln_end_.value()))
      return transport::process_result::kError;
    // let's check [request-line] section..
    if (!check_request_line()) return transport::process_result::kError;
    // let's check [headers] section..
    if (!check_headers()) return transport::process_result::kError;
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

 private:
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  bool check_request_line() {
    uint32_t i = 0;
    // check [method]..
    while (i < rln_end_.value()) {
      if (buffer_[i] == constants::characters::kSpace)
        break;
      else if (!helpers::is_token(buffer_[i]))
        return false;
      i++;
    }
    switch (i) {
      case 3:
        if (!(buffer_[0] == constants::methods::kGet[0] &&
              buffer_[1] == constants::methods::kGet[1] &&
              buffer_[2] == constants::methods::kGet[2]) &&
            !(buffer_[0] == constants::methods::kPut[0] &&
              buffer_[1] == constants::methods::kPut[1] &&
              buffer_[2] == constants::methods::kPut[2]))
          return false;
        break;
      case 4:
        if (!(buffer_[0] == constants::methods::kHead[0] &&
              buffer_[1] == constants::methods::kHead[1] &&
              buffer_[2] == constants::methods::kHead[2] &&
              buffer_[3] == constants::methods::kHead[3]) &&
            !(buffer_[0] == constants::methods::kPost[0] &&
              buffer_[1] == constants::methods::kPost[1] &&
              buffer_[2] == constants::methods::kPost[2] &&
              buffer_[3] == constants::methods::kPost[3]))
          return false;
        break;
      case 5:
        if (!(buffer_[0] == constants::methods::kTrace[0] &&
              buffer_[1] == constants::methods::kTrace[1] &&
              buffer_[2] == constants::methods::kTrace[2] &&
              buffer_[3] == constants::methods::kTrace[3] &&
              buffer_[4] == constants::methods::kTrace[4]))
          return false;
        break;
      case 6:
        if (!(buffer_[0] == constants::methods::kDelete[0] &&
              buffer_[1] == constants::methods::kDelete[1] &&
              buffer_[2] == constants::methods::kDelete[2] &&
              buffer_[3] == constants::methods::kDelete[3] &&
              buffer_[4] == constants::methods::kDelete[4] &&
              buffer_[5] == constants::methods::kDelete[5]))
          return false;
        break;
      case 7:
        if (!(buffer_[0] == constants::methods::kConnect[0] &&
              buffer_[1] == constants::methods::kConnect[1] &&
              buffer_[2] == constants::methods::kConnect[2] &&
              buffer_[3] == constants::methods::kConnect[3] &&
              buffer_[4] == constants::methods::kConnect[4] &&
              buffer_[5] == constants::methods::kConnect[5] &&
              buffer_[6] == constants::methods::kConnect[6]) &&
            !(buffer_[0] == constants::methods::kOptions[0] &&
              buffer_[1] == constants::methods::kOptions[1] &&
              buffer_[2] == constants::methods::kOptions[2] &&
              buffer_[3] == constants::methods::kOptions[3] &&
              buffer_[4] == constants::methods::kOptions[4] &&
              buffer_[5] == constants::methods::kOptions[5] &&
              buffer_[6] == constants::methods::kOptions[6]))
          return false;
        break;
      default:
        return false;
    }
    mtd_end_ = i++;
    // check [path]..
    if (buffer_[i++] != constants::characters::kSlash) return false;
    while (i < rln_end_.value()) {
      if (buffer_[i] == constants::characters::kSpace) break;
      if (buffer_[i] == constants::characters::kPercent) {
        if (i + 2 >= rln_end_.value()) return false;
        if (!helpers::is_hex_digit(buffer_[i + 1]) ||
            !helpers::is_hex_digit(buffer_[i + 2]))
          return false;
        i += 3;
        continue;
      } else if (helpers::is_pchar(buffer_[i]) ||
                 buffer_[i] == constants::characters::kSlash ||
                 buffer_[i] == constants::characters::kQuestion)
        i++;
      else
        return false;
    }
    pth_end_ = i++;
    // check [http-version]..
    if ((rln_end_.value() - i) < kHttpVersionLen) return false;
    if (buffer_[i] != constants::characters::kHUpperCase ||
        buffer_[i + 1] != constants::characters::kTUpperCase ||
        buffer_[i + 2] != constants::characters::kTUpperCase ||
        buffer_[i + 3] != constants::characters::kPUpperCase ||
        buffer_[i + 4] != constants::characters::kSlash ||
        !helpers::is_digit(buffer_[i + 5]) ||
        buffer_[i + 6] != constants::characters::kDot ||
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
  bool check_headers() {
    bool fname_opt = true;  // true ? searching for field-name else field-value.
    uint32_t i = ver_end_.value() + sizeof(constants::strings::kCrLf) - 1;
    uint32_t fn_beg = i, fn_end = i;
    uint32_t fv_beg = i, fv_end = i;
    uint32_t end = hdr_end_.value() + 2;
    while (i < end) {
      if (fname_opt) {
        if (buffer_[i] == constants::characters::kColon) {
          fname_opt = false;
          fn_end = i;
          fv_beg = fv_end = i + 1;
        } else if (!helpers::is_token(buffer_[i]))
          return false;
      } else if (buffer_[i] == constants::characters::kCr) {
        if (++i >= end) return false;
        if (buffer_[i] != constants::characters::kLf) return false;
        fname_opt = true;
        fv_end = i - 1;
        headers_.push_back(std::make_tuple(fn_beg, fn_end, fv_beg, fv_end));
        fn_beg = fn_end = i + 1;
      } else if (!(helpers::is_vchar(buffer_[i]) ||
                   helpers::is_obs_text(buffer_[i]) ||
                   buffer_[i] == constants::characters::kSpace ||
                   buffer_[i] == constants::characters::kTab))
        return false;
      i++;
    }
    return true;
  }
  std::optional<uint32_t> get(const uint8_t* const str, uint32_t str_len,
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
  static constexpr uint32_t kMaxRequestLength = 16384; // 16kb.
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
  std::vector<std::tuple<uint32_t, uint32_t, uint32_t, uint32_t>> headers_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
