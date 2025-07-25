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

#ifndef martianlabs_doba_protocol_http11_headers_h
#define martianlabs_doba_protocol_http11_headers_h

#include <string_view>

#include "constants.h"

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// headers                                                             ( class )
// -----------------------------------------------------------------------------
// This class holds for the http 1.1 headers implementation.
// -----------------------------------------------------------------------------
// =============================================================================
template <typename PAty>
class headers {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  headers() = default;
  headers(const headers&) = delete;
  headers(headers&&) noexcept = delete;
  ~headers() = default;
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  headers& operator=(const headers&) = delete;
  headers& operator=(headers&&) noexcept = delete;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  void add(const std::string_view& k, const std::string_view& v) {
    std::size_t entry_length = k.length() + v.length() + 3;
    if ((size_ - length_) > (entry_length + 2)) {
      memcpy(&buffer_[length_], k.data(), k.length());
      length_ += k.length();
      buffer_[length_++] = constants::character::kColon;
      memcpy(&buffer_[length_], v.data(), v.length());
      length_ += v.length();
      buffer_[length_++] = constants::character::kCr;
      buffer_[length_++] = constants::character::kLf;
    }
  }
  inline std::size_t length() const { return length_; }

 private:
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  inline void reset() {
    buffer_ = nullptr;
    size_ = 0;
    length_ = 0;
  }
  inline void prepare(char* buffer, const std::size_t& size) {
    buffer_ = buffer;
    size_ = size;
  }
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  char* buffer_ = nullptr;
  std::size_t size_ = 0;
  std::size_t length_ = 0;
  // ___________________________________________________________________________
  // FRIEND-CLASSEs                                                  ( private )
  //
  friend typename PAty;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
