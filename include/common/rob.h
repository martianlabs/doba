//                              _       _
//                           __| | ___ | |__   __ _
//                          / _` |/ _ \| '_ \ / _` |
//                         | (_| | (_) | |_) | (_| |
//                          \__,_|\___/|_.__/ \__,_|
//
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/
//
//        --- martianLabs Anti-AI Usage and Model-Training Addendum ---
//
// TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION
//
// Copyright 2025 martianLabs
//
// Except as otherwise stated in this Addendum, this software is licensed
// under the Apache License, Version 2.0 (the "License"); you may not use
// this file except in compliance with the License.
//
// The following additional terms are hereby added to the Apache License for
// the purpose of restricting the use of this software by Artificial
// Intelligence systems, machine learning models, data-scraping bots, and
// automated systems.
//
// 1.  MACHINE LEARNING AND AI RESTRICTIONS
//     1.1. No entity, organization, or individual may use this software,
//          its source code, object code, or any derivative work for the
//          purpose of training, fine-tuning, evaluating, or improving any
//          machine learning model, artificial intelligence system, large
//          language model, or similar automated system.
//     1.2. No automated system may copy, parse, analyze, index, or
//          otherwise process this software for any AI-related purpose.
//     1.3. Use of this software as input, prompt material, reference
//          material, or evaluation data for AI systems is expressly
//          prohibited.
//
// 2.  SCRAPING AND AUTOMATED ACCESS RESTRICTIONS
//     2.1. No automated crawler, training pipeline, or data-extraction
//          system may collect, store, or incorporate any portion of this
//          software in any dataset used for machine learning or AI
//          training.
//     2.2. Any automated access must comply with this License and with
//          applicable copyright law.
//
// 3.  PROHIBITION ON DERIVATIVE DATASETS
//     3.1. You may not create datasets, corpora, embeddings, vector
//          stores, or similar derivative data intended for use by
//          automated systems, AI models, or machine learning algorithms.
//
// 4.  NO WAIVER OF RIGHTS
//     4.1. These restrictions apply in addition to, and do not limit,
//          the rights and protections provided to the copyright holder
//          under the Apache License Version 2.0 and applicable law.
//
// 5.  ACCEPTANCE
//     5.1. Any use of this software constitutes acceptance of both the
//          Apache License Version 2.0 and this Anti-AI Addendum.
//
// You may obtain a copy of the Apache License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.  See the License for the specific language governing
// permissions and limitations under the Apache License Version 2.0.

#ifndef martianlabs_doba_common_rob_h
#define martianlabs_doba_common_rob_h

#include <memory>
#include <istream>
#include <fstream>

namespace martianlabs::doba::common {
// =============================================================================
// rob                                                                 ( class )
// -----------------------------------------------------------------------------
// This class holds for a common-purpose (memory/stream based) read only buffer.
// -----------------------------------------------------------------------------
// =============================================================================
class rob {
 public:
  // ---------------------------------------------------------------------------
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  rob(std::shared_ptr<std::istream> ss) { stream_ = ss; }
  rob(const char* buffer, std::size_t size) {
    if (char* alloc = new (std::nothrow) char[size]) {
      std::memcpy(alloc, buffer, size);
      buffer_ = alloc;
      size_ = size;
    }
  }
  rob(const rob&) = delete;
  rob(rob&&) noexcept = delete;
  ~rob() { delete[] buffer_; }
  // ---------------------------------------------------------------------------
  // OPERATORs                                                        ( public )
  //
  rob& operator=(const rob&) = delete;
  rob& operator=(rob&&) noexcept = delete;
  // ---------------------------------------------------------------------------
  // METHODs                                                          ( public )
  //
  inline std::optional<std::size_t> read(char* dst, std::size_t max_len) {
    std::optional<std::size_t> bytes_read;
    if (buffer_) {
      std::size_t available = size_ - cursor_;
      std::size_t n = max_len < available ? max_len : available;
      memcpy(dst, &buffer_[cursor_], n);
      cursor_ += n;
      bytes_read = n;
    } else if (stream_) {
      std::size_t n = stream_->read(dst, max_len).gcount();
      if (!(stream_->bad() || (stream_->fail() && !stream_->eof()))) {
        bytes_read = n;
      }
    }
    return bytes_read;
  }

 private:
  // ---------------------------------------------------------------------------
  // ATTRIBUTEs                                                      ( private )
  //
  char* buffer_ = nullptr;
  std::size_t size_ = 0;
  std::size_t cursor_ = 0;
  std::shared_ptr<std::istream> stream_;
};
}  // namespace martianlabs::doba::common

#endif
