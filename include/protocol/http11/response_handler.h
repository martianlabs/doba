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

#ifndef martianlabs_doba_protocol_http11_response_handler_h
#define martianlabs_doba_protocol_http11_response_handler_h

#include <memory>

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// response_handler                                                    ( class )
// -----------------------------------------------------------------------------
// This class holds for the http 1.1 response actions implementation.
// -----------------------------------------------------------------------------
// =============================================================================
class response_handler {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  response_handler(char* const buf, const std::size_t& sze, std::size_t& cur)
      : buf_{buf}, size_{sze}, cursor_{cur} {}
  response_handler(const response_handler&) = delete;
  response_handler(response_handler&&) noexcept = delete;
  ~response_handler() = default;
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  response_handler& operator=(const response_handler&) = delete;
  response_handler& operator=(response_handler&&) noexcept = delete;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  response_handler& add_header(std::string_view key, std::string_view val) {
    std::size_t k_sz = key.size(), v_sz = val.size();
    std::size_t entry_length = k_sz + v_sz + 3;
    if ((size_ - cursor_) > (entry_length + 2)) {
      if (!key.size()) return *this;
      auto initial_cur = cursor_;
      for (const char& c : key) {
        if (!helpers::is_token(c)) {
          cursor_ = initial_cur;
          return *this;
        }
        buf_[cursor_++] = helpers::tolower_ascii(c);
      }
      buf_[cursor_++] = constants::character::kColon;
      for (const char& c : val) {
        if (!(helpers::is_vchar(c) || helpers::is_obs_text(c) ||
              c == constants::character::kSpace ||
              c == constants::character::kHTab)) {
          return *this;
        }
      }
      memcpy(&buf_[cursor_], val.data(), v_sz);
      cursor_ += v_sz;
      buf_[cursor_++] = constants::character::kCr;
      buf_[cursor_++] = constants::character::kLf;
    }
    return *this;
  }
  template <typename T>
    requires std::is_arithmetic_v<T>
  response_handler& add_header(std::string_view key, T&& val) {
    return add_header(key, std::to_string(val));
  }
  response_handler& add_body(std::string_view body) { return *this; }

 private:
  // ___________________________________________________________________________
  // CONSTANTs                                                       ( private )
  //
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  char* const buf_;
  std::size_t& cursor_;
  const std::size_t& size_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
