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
  request() { setup(); }
  request(const char* request_line, std::size_t request_line_size,
          const char* headers, std::size_t headers_size,
          method method, target target) {
    setup();
    memcpy(buf_, request_line, request_line_size);
    memcpy(&buf_[request_line_size], headers, headers_size);
    message_.prepare(&buf_[request_line_size], slh_sz_ - request_line_size,
                     &buf_[slh_sz_], bod_sz_, headers_size);
    method_ = method;
    target_ = target;
  }
  request(const request&) = delete;
  request(request&&) noexcept = delete;
  ~request() { cleanup(); }
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  request& operator=(const request&) = delete;
  request& operator=(request&&) noexcept = delete;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  inline method get_method() const { return method_.value(); }
  inline target get_target() const { return target_.value(); }
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
  // ___________________________________________________________________________
  // CONSTANTs                                                        ( public )
  //
  static constexpr std::size_t kDefaultFullBufferMemorySize = 8192;  // 8kb.
  static constexpr std::size_t kDefaultBodyBufferMemorySize = 4096;  // 4kb.

 private:
  // ___________________________________________________________________________
  // CONSTANTs                                                       ( private )
  //
  static constexpr std::size_t kHttpVersionLen = 8;
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  void setup() {
    buf_sz_ = kDefaultFullBufferMemorySize;
    bod_sz_ = kDefaultBodyBufferMemorySize;
    slh_sz_ = buf_sz_ - bod_sz_;
    buf_ = (char*)malloc(buf_sz_);
  }
  void cleanup() { free(buf_); }
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  std::size_t buf_sz_;
  std::size_t bod_sz_;
  std::size_t slh_sz_;
  char* buf_;
  std::optional<method> method_;
  std::optional<target> target_;
  message message_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
