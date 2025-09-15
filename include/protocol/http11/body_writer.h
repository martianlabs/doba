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

#ifndef martianlabs_doba_protocol_http11_body_writer_h
#define martianlabs_doba_protocol_http11_body_writer_h

#include <memory>
#include <istream>
#include <ostream>
#include <string_view>

#include "constants.h"

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// body_writer                                                         ( class )
// -----------------------------------------------------------------------------
// This class holds for a body writer implementation.
// -----------------------------------------------------------------------------
// =============================================================================
class body_writer {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  body_writer(const body_writer&) = delete;
  body_writer(body_writer&&) noexcept = delete;
  ~body_writer() { free(mem_buffer_); }
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  body_writer& operator=(const body_writer&) = delete;
  body_writer& operator=(body_writer&&) noexcept = delete;
  // ___________________________________________________________________________
  // STATIC-METHODs                                                   ( public )
  //
  static auto memory_writer(std::size_t length) {
    return std::shared_ptr<body_writer>(new body_writer(length));
  }
  static auto file_writer(std::shared_ptr<std::ofstream> file) {
    return std::shared_ptr<body_writer>(new body_writer(file));
  }
  // ___________________________________________________________________________
  // ENUMs                                                            ( public )
  //
  enum class buffer_type { kMemory, kFile };
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  buffer_type get_buffer_type() const { return buf_type_; }
  void write(const char* const buffer, std::size_t length) {
    if (buf_type_ == buffer_type::kMemory) {
      memcpy(&mem_buffer_[mem_cursor_], buffer, length);
      mem_cursor_ += length;
    } else {
      fil_handler_->write(buffer, length);
    }
  }

 private:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                        ( private )
  //
  body_writer(std::size_t length) {
    buf_type_ = buffer_type::kMemory;
    mem_size_ = length;
    mem_buffer_ = (char*)malloc(mem_size_);
    mem_cursor_ = 0;
  }
  body_writer(std::shared_ptr<std::ostream> output_stream) {
    buf_type_ = buffer_type::kFile;
    fil_handler_ = output_stream;
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
  std::shared_ptr<std::ostream> fil_handler_;
  buffer_type buf_type_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
