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

#include <string_view>

#include "constants.h"

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// response_handler                                                    ( class )
// -----------------------------------------------------------------------------
// This class holds for the http 1.1 response handler implementation.
// -----------------------------------------------------------------------------
// =============================================================================
template <typename RSty>
class response_handler {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  response_handler() {
    headers_ = (char*)malloc(constants::limits::kDefaultHeadersSectionSz);
    body_ = (char*)malloc(constants::limits::kDefaultBodySectionSz);
  }
  response_handler(const response_handler&) = delete;
  response_handler(response_handler&&) noexcept = delete;
  ~response_handler() {
    free(headers_);
    free(body_);
  }
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  response_handler& operator=(const response_handler&) = delete;
  response_handler& operator=(response_handler&&) noexcept = delete;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  response_handler& set_header(const std::string_view& k,
                               const std::string_view& v) {
    memcpy(&headers_[headers_cur_], k.data(), k.length());
    headers_cur_ += k.length();
    headers_[headers_cur_++] = constants::character::kColon;
    memcpy(&headers_[headers_cur_], v.data(), v.length());
    headers_cur_ += v.length();
    headers_[headers_cur_++] = constants::character::kCr;
    headers_[headers_cur_++] = constants::character::kLf;
    return *this;
  }
  response_handler& set_body(const std::string_view& body) {
    body_cur_ = body.length();
    memcpy(body_, body.data(), body_cur_);
    return *this;
  }

 private:
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  inline void reset() {
    headers_cur_ = 0;
    body_cur_ = 0;
  }
  inline const char* headers() const { return headers_; }
  inline std::size_t headers_length() const { return headers_cur_; }
  inline const char* body() const { return body_; }
  inline std::size_t body_length() const { return body_cur_; }
  // ___________________________________________________________________________
  // FRIEND-CLASSEs                                                  ( private )
  //
  friend typename RSty;
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  char* headers_ = nullptr;
  char* body_ = nullptr;
  std::size_t headers_cur_ = 0;
  std::size_t body_cur_ = 0;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
