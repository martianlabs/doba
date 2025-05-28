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

#ifndef martianlabs_doba_buffex_h
#define martianlabs_doba_buffex_h

#include "platform.h"

namespace martianlabs::doba {
// =============================================================================
// buffex                                                          ( interface )
// -----------------------------------------------------------------------------
// This class will manage i/o buffers (either memory or file based).
// -----------------------------------------------------------------------------
// =============================================================================
class buffex {
  // ___________________________________________________________________________
  // ENUMs                                                           ( private )
  //
  enum class type { kMemory, kFile };
  // ___________________________________________________________________________
  // STRUCTs                                                         ( private )
  //
  struct memory_data {
    char* buffer_;
    std::size_t reader_cursor_;
    std::size_t writer_cursor_;
    std::size_t size_;
  };
  // ___________________________________________________________________________
  // UNIONs                                                          ( private )
  //
  union data {
    memory_data memory;
  };

 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  buffex(std::size_t max_bytes_to_allocate_in_memory = kDefaultMaxBytesInMem) {
    type_ = type::kMemory;
    std::memset(&data_, 0, sizeof(data));
  }
  buffex(const buffex& in) {
    type_ = type::kMemory;
    std::memset(&data_, 0, sizeof(data));
    if (this != &in) {
    }
  }
  buffex(buffex&& in) noexcept {}
  ~buffex() {}
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  buffex& operator=(const buffex& in) { return *this; }
  buffex& operator=(buffex&& in) noexcept { return *this; }
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  template <typename T>
  buffex& append(const T& val) {
    return append(&val, sizeof(T));
  }
  buffex& append(const char* data) { return append((void*)data, strlen(data)); }
  buffex& append(const std::string& data) { return append(data.c_str()); }
  buffex& append(void* data, const std::size_t& size) { return *this; }

 private:
  // ___________________________________________________________________________
  // CONSTANTs                                                       ( private )
  //
  static constexpr std::size_t kMemoryChunkInBytes = 8;
  static constexpr std::size_t kDefaultMaxBytesInMem = 4096;
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  type type_;
  data data_;
};
}  // namespace martianlabs::doba

#endif