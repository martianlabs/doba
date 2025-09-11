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

 private:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                        ( private )
  //
  body() {}
  body(const char* const buffer, std::size_t length) {}
  body(const std::istream& input_stream) {}
  body(const std::ostream& output_stream) {}
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  char* memory_buffer_ = nullptr;
  std::size_t memory_size_ = 0;
  std::size_t memory_cursor_ = 0;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
