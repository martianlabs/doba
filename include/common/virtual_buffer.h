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

#ifndef martianlabs_doba_common_reference_buffer_h
#define martianlabs_doba_common_reference_buffer_h

#include <memory>
#include <istream>

namespace martianlabs::doba::common {
// =============================================================================
// reference_buffer                                                    ( class )
// -----------------------------------------------------------------------------
// This class holds for a generic buffer holding memory/stream references.
// -----------------------------------------------------------------------------
// =============================================================================
class reference_buffer {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  reference_buffer(std::shared_ptr<std::stringstream> ss) {
    stream_ = ss;
    memory_buffer_ = nullptr;
    memory_buffer_size_ = 0;
  }
  reference_buffer(const char* buffer, std::size_t size) {
    memory_buffer_ = buffer;
    memory_buffer_size_ = size;
    stream_ = nullptr;
  }
  reference_buffer(const reference_buffer&) = default;
  reference_buffer(reference_buffer&&) noexcept = default;
  ~reference_buffer() = default;
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  reference_buffer& operator=(const reference_buffer&) = default;
  reference_buffer& operator=(reference_buffer&&) noexcept = default;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  std::optional<std::size_t> read(char* dst, std::size_t len) {
    if (memory_buffer_ != nullptr) {
      auto n = len < memory_buffer_size_ ? len : memory_buffer_size_;
      memcpy(dst, memory_buffer_, n);
      memory_buffer_size_ -= n;
      memory_buffer_ += n;
      return n;
    } else {
      auto n = stream_->read(dst, len).gcount();
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
