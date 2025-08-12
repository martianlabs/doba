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

#ifndef martianlabs_doba_protocol_http11_body_h
#define martianlabs_doba_protocol_http11_body_h

#include <string_view>

#include "constants.h"

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// body                                                                ( class )
// -----------------------------------------------------------------------------
// This class holds for the http 1.1 body implementation.
// -----------------------------------------------------------------------------
// =============================================================================
class body {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  body() = default;
  body(const body&) = delete;
  body(body&&) noexcept = delete;
  ~body() = default;
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  body& operator=(const body&) = delete;
  body& operator=(body&&) noexcept = delete;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  inline void prepare(char* buffer, std::size_t size) {
    buf_ = buffer;
    buf_sz_ = size;
    cur_ = 0;
  }
  inline void reset() {
    buf_ = nullptr;
    buf_sz_ = 0;
    cur_ = 0;
  }
  void add(const char* buffer, std::size_t size) {
    if ((buf_sz_ - cur_) > size) {
      memcpy(&buf_[cur_], buffer, size);
      cur_ += size;
    }
  }
  inline std::size_t length() const { return cur_; }

 private:
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  char* buf_ = nullptr;
  std::size_t buf_sz_ = 0;
  std::size_t cur_ = 0;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
