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

#ifndef martianlabs_doba_common_rorb_h
#define martianlabs_doba_common_rorb_h

#include <memory>
#include <istream>
#include <fstream>

namespace martianlabs::doba::common {
// =============================================================================
// rorb                                                                ( class )
// -----------------------------------------------------------------------------
// This class holds for a generic read only reference buffer.
// -----------------------------------------------------------------------------
// =============================================================================
class rorb {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  rorb(std::shared_ptr<std::istream> ss) {
    stream_ = ss;
    memory_buffer_ = nullptr;
    memory_buffer_size_ = 0;
  }
  rorb(const char* buffer, std::size_t size) {
    memory_buffer_ = buffer;
    memory_buffer_size_ = size;
    stream_ = nullptr;
  }
  rorb(const rorb&) = default;
  rorb(rorb&&) noexcept = default;
  ~rorb() = default;
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  rorb& operator=(const rorb&) = default;
  rorb& operator=(rorb&&) noexcept = default;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  std::optional<std::size_t> read(char* dst, std::size_t max_len) {
    if (memory_buffer_ != nullptr) {
      auto n = max_len < memory_buffer_size_ ? max_len : memory_buffer_size_;
      memcpy(dst, memory_buffer_, n);
      memory_buffer_size_ -= n;
      memory_buffer_ += n;
      return n;
    } else {
      auto n = stream_->read(dst, max_len).gcount();
      if (!(stream_->bad() || (stream_->fail() && !stream_->eof()))) {
        return n;
      }
    }
    return std::nullopt;
  }

 private:
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  const char* memory_buffer_ = nullptr;
  std::size_t memory_buffer_size_ = 0;
  std::shared_ptr<std::istream> stream_ = nullptr;
};
}  // namespace martianlabs::doba::common

#endif
