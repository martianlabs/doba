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

#include "body.h"

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
  response_handler(char*& buffer, const std::size_t& core_size,
                   const std::size_t& body_size, std::size_t& core_cursor,
                   std::size_t& body_cursor)
      : buffer_{buffer},
        buffer_core_size_{core_size},
        buffer_body_size_{body_size},
        buffer_core_cursor_{core_cursor},
        buffer_body_cursor_{body_cursor},
        buffer_core_headers_start_{core_cursor} {}
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
    if ((buffer_core_size_ - buffer_core_cursor_) > (entry_length + 2)) {
      if (!key.size()) return *this;
      auto initial_cur = buffer_core_cursor_;
      for (const char& c : key) {
        if (!helpers::is_token(c)) {
          buffer_core_cursor_ = initial_cur;
          return *this;
        }
        buffer_[buffer_core_cursor_++] = helpers::tolower_ascii(c);
      }
      buffer_[buffer_core_cursor_++] = constants::character::kColon;
      for (const char& c : val) {
        if (!(helpers::is_vchar(c) || helpers::is_obs_text(c) ||
              c == constants::character::kSpace ||
              c == constants::character::kHTab)) {
          return *this;
        }
      }
      memcpy(&buffer_[buffer_core_cursor_], val.data(), v_sz);
      buffer_core_cursor_ += v_sz;
      buffer_[buffer_core_cursor_++] = constants::character::kCr;
      buffer_[buffer_core_cursor_++] = constants::character::kLf;
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
    std::size_t i = buffer_core_headers_start_, j = 0, k_start = i,
                k_sz = k.size();
    while (i < buffer_core_cursor_) {
      switch (buffer_[i]) {
        case constants::character::kColon:
          if (searching_for_key) {
            searching_for_key = false;
          }
          break;
        case constants::character::kCr:
          if (searching_for_key) return *this;
          if (matched && j == k_sz) {
            auto const off = i + 2;
            std::memmove(&buffer_[k_start], &buffer_[off],
                         buffer_core_cursor_ - off);
            buffer_core_cursor_ -= (off - k_start);
            std::memset(&buffer_[buffer_core_cursor_], 0,
                        buffer_core_size_ - buffer_core_cursor_);
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
                matched = buffer_[i] == helpers::tolower_ascii(k[j++]);
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
    if (buffer_body_size_ - buffer_body_cursor_ >= length) {
      std::memcpy(&buffer_[buffer_core_size_], buffer, length);
      buffer_body_cursor_ += length;
    } else {
      /*
      pepe
      */

      /*
      pepe fin
      */
    }
    return add_header(headers::kContentLength, length);
  }
  response_handler& set_body(std::string_view sv) {
    return set_body(sv.data(), sv.size());
  }
  template <typename T>
    requires std::is_arithmetic_v<T>
  response_handler& set_body(T&& val) {
    return set_body(std::to_string(val));
  }

 private:
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  char*& buffer_;
  std::size_t& buffer_core_cursor_;
  std::size_t& buffer_body_cursor_;
  const std::size_t& buffer_core_size_;
  const std::size_t& buffer_body_size_;
  std::size_t buffer_core_headers_start_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
