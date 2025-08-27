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
    mtd_ = method::kUnknown;
    tgt_ = target::kUnknown;
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
  request& set_method(method method) {
    mtd_ = method;
    return *this;
  }
  request& set_target(target target) {
    tgt_ = target;
    return *this;
  }
  request& set_absolute_path(std::string_view path) {
    abs_ = path;
    return *this;
  }
  bool add_body(const char* buf, std::size_t size) {
    return msg_.add_body(buf, size);
  }
  bool add_body(std::string_view sv) {
    return msg_.add_body(sv.data(), sv.size());
  }
  request& add_header(std::string_view key, std::string_view value) {
    msg_.add_header(key, value);
    return *this;
  }
  template <typename T>
    requires std::is_arithmetic_v<T>
  request& add_header(std::string_view k, const T& v) {
    return add_header(k, std::to_string(v));
  }
  request& remove_header(std::string_view key) {
    msg_.remove_header(key);
    return *this;
  }
  auto get_method() const { return mtd_; }
  auto get_target() const { return tgt_; }
  auto get_absolute_path() const { return abs_; }
  auto get_headers() const { return msg_.get_headers(); }
  auto get_header(std::string_view key) const { return msg_.get_header(key); }
  // ___________________________________________________________________________
  // CONSTANTs                                                        ( public )
  //
  static constexpr std::size_t kDefaultFullBufferMemorySize = 4096;  // 4kb.
  static constexpr std::size_t kDefaultBodyBufferMemorySize = 2048;  // 2kb.

 private:
  // ___________________________________________________________________________
  // CONSTANTs                                                       ( private )
  //
  static constexpr std::size_t kHttpVersionLen = 8;
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  void set(const char* r, std::size_t r_sz, const char* h, std::size_t h_sz) {
    if (!r || !h || !r_sz || !h_sz) return;
    memcpy(buf_, r, r_sz);
    memcpy(&buf_[r_sz], h, h_sz);
    msg_.set(&buf_[r_sz], slh_sz_ - r_sz, &buf_[slh_sz_], bod_sz_, h_sz);
  }
  void reset() {
    mtd_ = method::kUnknown;
    tgt_ = target::kUnknown;
    abs_.clear();
    msg_.reset();
  }
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  std::size_t buf_sz_ = 0;
  std::size_t bod_sz_ = 0;
  std::size_t slh_sz_ = 0;
  char* buf_ = nullptr;
  method mtd_;
  target tgt_;
  std::string abs_;
  message msg_;
  // ___________________________________________________________________________
  // FRIENDs                                                         ( private )
  //
  friend class decoder;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
