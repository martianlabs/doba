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

#include <memory>
#include <istream>
#include <ostream>
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
  ~body() { free(mem_buffer_); }
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  body& operator=(const body&) = delete;
  body& operator=(body&&) noexcept = delete;
  // ___________________________________________________________________________
  // STATIC-METHODs                                                   ( public )
  //
  static auto memory_reader(const char* const buffer, std::size_t length) {
    return std::shared_ptr<body>(new body(buffer, length));
  }
  static auto file_reader(std::shared_ptr<std::ifstream> file) {
    return std::shared_ptr<body>(new body(file));
  }
  // ___________________________________________________________________________
  // ENUMs                                                            ( public )
  //
  enum class ios_type { kReader, kWriter };
  enum class buf_type { kMemory, kFile };
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  ios_type get_ios_type() const { return ios_type_; }
  buf_type get_buf_type() const { return buf_type_; }
  std::shared_ptr<std::istream> file_data() const { return fil_handler_; }
  const char* const memory_data() const { return mem_buffer_; }
  std::size_t memory_size() const { return mem_size_; }

 private:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                        ( private )
  //
  body() {
    ios_type_ = ios_type::kWriter;
    buf_type_ = buf_type::kMemory;
    mem_size_ = constants::limits::kDefaultBodyMsgMaxSizeInRam;
    mem_buffer_ = (char*)malloc(mem_size_);
    mem_cursor_ = 0;
  }
  body(const char* const buffer, std::size_t length) {
    ios_type_ = ios_type::kReader;
    buf_type_ = buf_type::kMemory;
    mem_size_ = length;
    if (mem_buffer_ = (char*)malloc(mem_size_)) {
      memcpy(mem_buffer_, buffer, mem_size_);
    }
    mem_cursor_ = 0;
  }
  body(std::shared_ptr<std::ostream> output_stream) {
    ios_type_ = ios_type::kWriter;
    buf_type_ = buf_type::kFile;
    mem_buffer_ = NULL;
    mem_size_ = 0;
    mem_cursor_ = 0;
  }
  body(std::shared_ptr<std::istream> input_stream) {
    ios_type_ = ios_type::kReader;
    buf_type_ = buf_type::kFile;
    fil_handler_ = input_stream;
    mem_buffer_ = NULL;
    mem_size_ = 0;
    mem_cursor_ = 0;
  }
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  char* mem_buffer_;
  std::size_t mem_size_;
  std::size_t mem_cursor_;
  std::shared_ptr<std::istream> fil_handler_;
  ios_type ios_type_;
  buf_type buf_type_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
