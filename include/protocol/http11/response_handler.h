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

#ifndef martianlabs_doba_protocol_http11_response_handler_h
#define martianlabs_doba_protocol_http11_response_handler_h

#include <memory>

#include "body_reader.h"

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// response_handler                                                    ( class )
// -----------------------------------------------------------------------------
// This class holds for the http 1.1 response actions implementation.
// -----------------------------------------------------------------------------
// =============================================================================
class response_handler {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  response_handler(char* const buf, const std::size_t& sze, std::size_t& cur)
      : buf_{buf}, size_{sze}, cursor_{cur}, headers_start_{cur} {}
  response_handler(const response_handler&) = delete;
  response_handler(response_handler&&) noexcept = delete;
  ~response_handler() = default;
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  response_handler& operator=(const response_handler&) = delete;
  response_handler& operator=(response_handler&&) noexcept = delete;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  response_handler& add_header(std::string_view key, std::string_view val) {
    std::size_t k_sz = key.size(), v_sz = val.size();
    std::size_t entry_length = k_sz + v_sz + 3;
    if ((size_ - cursor_) > (entry_length + 2)) {
      if (!key.size()) return *this;
      auto initial_cur = cursor_;
      for (const char& c : key) {
        if (!helpers::is_token(c)) {
          cursor_ = initial_cur;
          return *this;
        }
        buf_[cursor_++] = helpers::tolower_ascii(c);
      }
      buf_[cursor_++] = constants::character::kColon;
      for (const char& c : val) {
        if (!(helpers::is_vchar(c) || helpers::is_obs_text(c) ||
              c == constants::character::kSpace ||
              c == constants::character::kHTab)) {
          return *this;
        }
      }
      memcpy(&buf_[cursor_], val.data(), v_sz);
      cursor_ += v_sz;
      buf_[cursor_++] = constants::character::kCr;
      buf_[cursor_++] = constants::character::kLf;
    }
    return *this;
  }
  template <typename T>
    requires std::is_arithmetic_v<T>
  response_handler& add_header(std::string_view key, const T& val) {
    return add_header(key, std::to_string(val));
  }
  response_handler& remove_header(std::string_view k) {
    bool matched = true;
    bool searching_for_key = true;
    std::size_t i = headers_start_, j = 0, k_start = i, k_sz = k.size();
    while (i < cursor_) {
      switch (buf_[i]) {
        case constants::character::kColon:
          if (searching_for_key) {
            searching_for_key = false;
          }
          break;
        case constants::character::kCr:
          if (searching_for_key) return *this;
          if (matched && j == k_sz) {
            auto const off = i + 2;
            std::memmove(&buf_[k_start], &buf_[off], cursor_ - off);
            cursor_ -= (off - k_start);
            std::memset(&buf_[cursor_], 0, size_ - cursor_);
            return *this;
          }
          searching_for_key = true;
          matched = true;
          k_start = i + 2;
          j = 0;
          i++;
          break;
        default:
          if (searching_for_key) {
            if (matched) {
              if (j < k_sz) {
                matched = buf_[i] == helpers::tolower_ascii(k[j++]);
              } else {
                matched = false;
              }
            }
          }
          break;
      }
      i++;
    }
    return *this;
  }
  response_handler& set_body(const char* const buffer, std::size_t length) {
    body_reader_ = body_reader::memory_reader(buffer, length);
    return *this;
  }
  response_handler& set_body(std::string_view sv) {
    body_reader_ = body_reader::memory_reader(sv.data(), sv.size());
    return *this;
  }
  response_handler& set_body(std::shared_ptr<std::ifstream> input_file_stream) {
    body_reader_ = body_reader::file_reader(input_file_stream);
    return *this;
  }
  template <typename T>
    requires std::is_arithmetic_v<T>
  response_handler& set_body(T&& val) {
    return set_body(std::to_string(val));
  }
  std::shared_ptr<body_reader> get_body_reader() const { return body_reader_; }

 private:
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  char* const buf_;
  std::size_t& cursor_;
  const std::size_t& size_;
  std::size_t headers_start_;
  std::shared_ptr<body_reader> body_reader_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
