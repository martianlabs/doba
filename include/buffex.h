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
 public:
  // ___________________________________________________________________________
  // ENUMs                                                            ( public )
  //
  enum class mapping { kMemory = 0, kFile = 1 };
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  buffex(const mapping& user_mapping = mapping::kMemory) {
    initialize(user_mapping);
  }
  buffex(const buffex& in) : buffex(in.mp_) {
    switch (mp_) {
      case mapping::kMemory: {
        m_sz_ = in.m_sz_;
        m_rd_ = in.m_rd_;
        m_wr_ = in.m_wr_;
        void* new_buffer = malloc(m_sz_);
        if (new_buffer) {
          m_bf_ = (char*)new_buffer;
          if (!memcpy(m_bf_, in.m_bf_, m_wr_)) {
            // ((Error)) -> out of memory!
            throw std::runtime_error("out of memory!");
          }
        } else {
          // ((Error)) -> out of memory!
          throw std::runtime_error("out of memory!");
        }
        break;
      }
      case mapping::kFile:
        break;
    }
  }
  buffex(buffex&& in) noexcept {
    m_bf_ = in.m_bf_;
    m_rd_ = in.m_rd_;
    m_wr_ = in.m_wr_;
    m_sz_ = in.m_sz_;
    mp_ = in.mp_;
    in.initialize(mapping::kMemory);
  }
  virtual ~buffex() { cleanup(); }
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  buffex& operator=(const buffex& in) {
    if (mp_ != in.mp_) {
      // ((Error)) -> source object is not compatible for copy!
      throw std::logic_error("source object is not compatible for copy");
    }

    /*
    pepe
    */

    /*
    cleanup();
    initialize(in.mp_);
    */

    /*
    pepe fin
    */

    switch (mp_) {
      case mapping::kMemory:
        m_sz_ = in.m_sz_;
        m_rd_ = in.m_rd_;
        m_wr_ = in.m_wr_;
        if (m_bf_ = (char*)malloc(m_sz_)) {
          if (!memcpy(m_bf_, in.m_bf_, m_wr_)) {
            // ((Error)) -> out of memory!
            throw std::runtime_error("out of memory!");
          }
        } else {
          // ((Error)) -> out of memory!
          throw std::runtime_error("out of memory!");
        }
        break;
      case mapping::kFile:
        break;
    }
    return *this;
  }
  buffex& operator=(buffex&& in) noexcept {
    m_bf_ = in.m_bf_;
    m_rd_ = in.m_rd_;
    m_wr_ = in.m_wr_;
    m_sz_ = in.m_sz_;
    mp_ = in.mp_;
    in.m_bf_ = nullptr;
    in.m_rd_ = 0;
    in.m_wr_ = 0;
    in.m_sz_ = 0;
    in.mp_ = mapping::kMemory;
    return *this;
  }
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  void reset() {
    switch (mp_) {
      case mapping::kMemory:
        m_rd_ = 0;
        m_wr_ = 0;
        break;
      case mapping::kFile:
        break;
    }
  }
  bool read(const std::size_t& bytes, void* o_ptr, std::size_t& o_len) {
    bool result = false;
    switch (mp_) {
      case mapping::kMemory: {
        std::size_t len = ((m_rd_ + bytes) < m_wr_) ? bytes : (m_wr_ - m_rd_);
        if (len) {
          if (!memcpy(o_ptr, &m_bf_[m_rd_], len)) {
            // ((Error)) -> out of memory!
            throw std::runtime_error("out of memory!");
          }
          m_rd_ += len;
          o_len = len;
          result = true;
        }
        break;
      }
      case mapping::kFile:
        break;
    }
    return result;
  }
  template <typename T>
  buffex& append(const T& val) {
    return append(&val, sizeof(T));
  }
  buffex& append(const char* data) { return append((void*)data, strlen(data)); }
  buffex& append(const std::string& data) { return append(data.c_str()); }
  buffex& append(void* data, const std::size_t& size) {
    if (data && size) {
      check_storage(size);
      switch (mp_) {
        case mapping::kMemory:
          if (!memcpy(&(m_bf_)[m_wr_], data, size)) {
            // ((Error)) -> out of memory!
            throw std::runtime_error("out of memory!");
          }
          m_wr_ += size;
          break;
        case mapping::kFile:
          break;
      }
    }
    return *this;
  }

 private:
  // ___________________________________________________________________________
  // CONSTANTs                                                       ( private )
  //
  static constexpr std::size_t kMemoryChunkSize = 8;
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  void initialize(const mapping& user_mapping) {
    m_rd_ = 0;
    m_wr_ = 0;
    m_sz_ = 0;
    m_bf_ = nullptr;
    mp_ = user_mapping;
  }
  void cleanup() {
    switch (mp_) {
      case mapping::kMemory:
        free(m_bf_);
        break;
      case mapping::kFile:
        break;
    }
  }
  void check_storage(const std::size_t& needed_bytes) {
    switch (mp_) {
      case mapping::kMemory: {
        std::size_t multiplier = needed_bytes / kMemoryChunkSize;
        multiplier += multiplier % kMemoryChunkSize;
        std::size_t bytes_to_allocate = kMemoryChunkSize * multiplier;
        if ((m_sz_ - m_wr_) < needed_bytes) {
          void* new_buffer = realloc(m_bf_, m_sz_ + bytes_to_allocate);
          if (new_buffer) {
            m_bf_ = (char*)new_buffer;
            m_sz_ += bytes_to_allocate;
          } else {
            // ((Error)) -> out of memory!
            throw std::runtime_error("out of memory!");
          }
        }
        break;
      }
      case mapping::kFile:
        break;
    }
  }
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  char* m_bf_;
  std::size_t m_rd_;
  std::size_t m_wr_;
  std::size_t m_sz_;
  mapping mp_;
};
}  // namespace martianlabs::doba

#endif