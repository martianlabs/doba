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
//                                                                    ( macros )
// -----------------------------------------------------------------------------
// Useful macros.
// -----------------------------------------------------------------------------
// =============================================================================
#define IS_GET_METHOD                           \
  (buffer_[0] == constants::methods::kGet[0] && \
   buffer_[1] == constants::methods::kGet[1] && \
   buffer_[2] == constants::methods::kGet[2])
#define IS_PUT_METHOD                           \
  (buffer_[0] == constants::methods::kPut[0] && \
   buffer_[1] == constants::methods::kPut[1] && \
   buffer_[2] == constants::methods::kPut[2])
#define IS_HEAD_METHOD                           \
  (buffer_[0] == constants::methods::kHead[0] && \
   buffer_[1] == constants::methods::kHead[1] && \
   buffer_[2] == constants::methods::kHead[2] && \
   buffer_[3] == constants::methods::kHead[3])
#define IS_POST_METHOD                           \
  (buffer_[0] == constants::methods::kPost[0] && \
   buffer_[1] == constants::methods::kPost[1] && \
   buffer_[2] == constants::methods::kPost[2] && \
   buffer_[3] == constants::methods::kPost[3])
#define IS_TRACE_METHOD                           \
  (buffer_[0] == constants::methods::kTrace[0] && \
   buffer_[1] == constants::methods::kTrace[1] && \
   buffer_[2] == constants::methods::kTrace[2] && \
   buffer_[3] == constants::methods::kTrace[3] && \
   buffer_[4] == constants::methods::kTrace[4])
#define IS_DELETE_METHOD                           \
  (buffer_[0] == constants::methods::kDelete[0] && \
   buffer_[1] == constants::methods::kDelete[1] && \
   buffer_[2] == constants::methods::kDelete[2] && \
   buffer_[3] == constants::methods::kDelete[3] && \
   buffer_[4] == constants::methods::kDelete[4] && \
   buffer_[5] == constants::methods::kDelete[5])
#define IS_CONNECT_METHOD                           \
  (buffer_[0] == constants::methods::kConnect[0] && \
   buffer_[1] == constants::methods::kConnect[1] && \
   buffer_[2] == constants::methods::kConnect[2] && \
   buffer_[3] == constants::methods::kConnect[3] && \
   buffer_[4] == constants::methods::kConnect[4] && \
   buffer_[5] == constants::methods::kConnect[5] && \
   buffer_[6] == constants::methods::kConnect[6])
#define IS_OPTIONS_METHOD                           \
  (buffer_[0] == constants::methods::kOptions[0] && \
   buffer_[1] == constants::methods::kOptions[1] && \
   buffer_[2] == constants::methods::kOptions[2] && \
   buffer_[3] == constants::methods::kOptions[3] && \
   buffer_[4] == constants::methods::kOptions[4] && \
   buffer_[5] == constants::methods::kOptions[5] && \
   buffer_[6] == constants::methods::kOptions[6])
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
  request() { buffer_ = (char*)malloc(kMaxRequestLength); }
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
    if ((kMaxRequestLength - cursor_) < length) {
      return transport::process_result::kError;
    }
    memcpy(&buffer_[cursor_], buffer, length);
    cursor_ += length;
    // let's detect the [end-of-headers] position..
    headers_end_ = get(buffer_, cursor_, kEndOfHeaders, kEndOfHeadersLen);
    if (!headers_end_.has_value()) {
      return transport::process_result::kMoreBytesNeeded;
    }
    // let's detect the [end-of-request-line] position..
    request_line_end_ = get(buffer_, headers_end_.value(), kCrLf, kCrLfLen);
    if (!(headers_end_.value() - request_line_end_.value())) {
      return transport::process_result::kError;
    }
    // let's check [request-line] section..
    if (!check_request_line()) {
      return transport::process_result::kError;
    }
    // let's check [headers] section..
    if (!check_headers()) {
      return transport::process_result::kError;
    }

    /*
    pepe
    */

    // printf("%.*s", int(buffer_cur_), buffer_);

    /*
    pepe fin
    */

    return transport::process_result::kCompleted;
  }
  void reset() {
    cursor_ = 0;
    headers_end_.reset();
    request_line_end_.reset();
    method_end_.reset();
    path_end_.reset();
    http_version_end_.reset();
  }

 private:
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  bool check_request_line() {
    uint32_t i = 0;
    // check [method]..
    while (i < request_line_end_.value()) {
      if (IS_SP(buffer_[i])) break;
      if (!IS_TOKEN(buffer_[i])) return false;
      i++;
    }
    switch (i) {
      case 3:
        if (!IS_GET_METHOD && !IS_PUT_METHOD) return false;
        break;
      case 4:
        if (!IS_HEAD_METHOD && !IS_POST_METHOD) return false;
        break;
      case 5:
        if (!IS_TRACE_METHOD) return false;
        break;
      case 6:
        if (!IS_DELETE_METHOD) return false;
        break;
      case 7:
        if (!IS_CONNECT_METHOD && !IS_OPTIONS_METHOD) return false;
        break;
      default:
        return false;
    }
    method_end_ = i++;
    // check [path]..
    while (i < request_line_end_.value()) {
      if (IS_SP(buffer_[i])) break;
      i++;
    }
    path_end_ = i++;
    // check [http-version]..
    while (i < request_line_end_.value()) {
      if (IS_CR(buffer_[i])) break;
      i++;
    }
    http_version_end_ = i;
    return true;
  }
  bool check_headers() const { return true; }
  std::optional<uint32_t> get(const char* const str, uint32_t str_len,
                              const char* const pattern, uint32_t pattern_len) {
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
  static constexpr uint32_t kMaxRequestLength = 8192;
  static constexpr char kCrLf[] = "\r\n";
  static constexpr uint32_t kCrLfLen = sizeof(kCrLf) - 1;
  static constexpr char kEndOfHeaders[] = "\r\n\r\n";
  static constexpr uint32_t kEndOfHeadersLen = sizeof(kEndOfHeaders) - 1;
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  char* buffer_ = nullptr;
  uint32_t cursor_ = 0;
  std::optional<uint32_t> headers_end_;
  std::optional<uint32_t> request_line_end_;
  std::optional<uint32_t> method_end_;
  std::optional<uint32_t> path_end_;
  std::optional<uint32_t> http_version_end_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
