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
    buf_sz_ = kDefaultFullBufferMemorySize;
    bod_sz_ = kDefaultBodyBufferMemorySize;
    slh_sz_ = buf_sz_ - bod_sz_;
    buf_ = (char*)malloc(buf_sz_);
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
  inline void prepare(const char* rln, std::size_t rln_sz, const char* hdr,
                      std::size_t hdr_sz, std::optional<method> method,
                      std::optional<target> target) {
    if (!rln || !hdr || !rln_sz || !hdr_sz) return;
    memcpy(buf_, rln, rln_sz);
    memcpy(&buf_[rln_sz], hdr, hdr_sz);
    auto hdr_len = slh_sz_ - rln_sz;
    msg_.prepare(&buf_[rln_sz], hdr_len, &buf_[slh_sz_], bod_sz_, hdr_sz);
    method_ = method;
    target_ = target;
  }
  inline void reset() {
    method_.reset();
    target_.reset();
    msg_.reset();
  }
  inline auto get_method() const { return method_.value(); }
  inline auto get_target() const { return target_.value(); }
  inline auto get_headers() const { return msg_.get_headers(); }
  inline auto get_header(std::string_view k) const {
    return msg_.get_header(k);
  }
  inline request& add_header(std::string_view k, std::string_view v) {
    msg_.add_header(k, v);
    return *this;
  }
  template <typename T>
    requires std::is_arithmetic_v<T>
  inline request& add_header(std::string_view k, const T& v) {
    return add_header(k, std::to_string(v));
  }
  inline request& remove_header(std::string_view key) {
    msg_.remove_header(key);
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
  // ATTRIBUTEs                                                      ( private )
  //
  std::size_t buf_sz_ = 0;
  std::size_t bod_sz_ = 0;
  std::size_t slh_sz_ = 0;
  char* buf_ = nullptr;
  std::optional<method> method_;
  std::optional<target> target_;
  message msg_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
