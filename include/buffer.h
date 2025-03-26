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

#ifndef martianlabs_doba_buffer_h
#define martianlabs_doba_buffer_h

#include "platform.h"

namespace martianlabs::doba {
// =============================================================================
// buffer                                                          ( interface )
// -----------------------------------------------------------------------------
// This class will manage i/o buffers (either memory or file based).
// -----------------------------------------------------------------------------
// =============================================================================
class buffer {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  buffer() { initialize(); }
  buffer(const buffer& in) : buffer() {
    check_and_fix();
    if (buffer_) {
      used_ = in.used_;
      if (!memcpy(buffer_, in.buffer_, used_)) {
        // ((Error)) -> out of memory?
      }
    } else {
      // ((Error)) -> out of memory?
    }
  }
  buffer(buffer&& in) noexcept {
    used_ = in.used_;
    size_ = in.size_;
    buffer_ = in.buffer_;
    in.used_ = 0;
    in.size_ = 0;
    in.buffer_ = NULL;
  }
  virtual ~buffer() { cleanup(); }
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  buffer& operator=(const buffer& in) {
    check_and_fix();
    if (buffer_) {
      used_ = in.used_;
      if (!memcpy(buffer_, in.buffer_, used_)) {
        // ((Error)) -> out of memory?
      }
    } else {
      // ((Error)) -> out of memory?
    }
    return *this;
  }
  buffer& operator=(buffer&& in) noexcept {
    cleanup();
    initialize();
    if (in.buffer_) {
      used_ = in.used_;
      size_ = in.size_;
      buffer_ = in.buffer_;
      in.used_ = 0;
      in.size_ = 0;
      in.buffer_ = NULL;
    }
    return *this;
  }
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  template <typename T>
  buffer& append(const T& data) {
    check_and_fix();
    if (buffer_ && (used_ + sizeof(data)) < kDefaultMaxSizeOnMemoryInBytes) {
      if (!memcpy(&((char*)buffer_)[used_], &data, sizeof(data))) {
        // ((Error)) -> out of memory?
      }
      used_ += sizeof(data);
    } else {
      // ((Error)) -> out of memory?
    }
    return *this;
  }
  buffer& append(const void* data, const std::size_t& size) {
    check_and_fix();
    if (buffer_ && (used_ + size) < kDefaultMaxSizeOnMemoryInBytes) {
      if (!memcpy(&((char*)buffer_)[used_], data, size)) {
        // ((Error)) -> out of memory?
      }
      used_ += size;
    } else {
      // ((Error)) -> out of memory?
    }
    return *this;
  }
  buffer& append(const char* data) { return append(data, strlen(data)); }
  buffer& append(const std::string& data) { return append(data.c_str()); }
  std::size_t size() const { return size_; }
  std::size_t used() const { return used_; }
  const void* const data() const { return buffer_; }

 private:
  // ___________________________________________________________________________
  // CONSTANTs                                                       ( private )
  //
  static constexpr std::size_t kDefaultMaxSizeOnMemoryInBytes = 4096;
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  void initialize() {
    used_ = 0;
    size_ = 0;
    buffer_ = NULL;
  }
  void cleanup() { free(buffer_); }
  void check_and_fix() {
    if (!buffer_) {
      used_ = 0;
      size_ = kDefaultMaxSizeOnMemoryInBytes;
      buffer_ = malloc(size_);
    }
  }
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  void* buffer_;
  std::size_t used_;
  std::size_t size_;
};
}  // namespace martianlabs::doba

#endif