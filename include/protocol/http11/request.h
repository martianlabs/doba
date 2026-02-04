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

#ifndef martianlabs_doba_protocol_http11_request_h
#define martianlabs_doba_protocol_http11_request_h

#include <array>

#include "target.h"
#include "helpers.h"
#include "headers.h"
#include "internal/headers_markers.h"
#include "common/hash_map.h"
#include "common/deserialize_result.h"

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// request                                                             ( class )
// -----------------------------------------------------------------------------
// This class holds for the http 1.1 request implementation.
// -----------------------------------------------------------------------------
// =============================================================================
class request {
 public:
  // ---------------------------------------------------------------------------
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  request(const request&) = delete;
  request(request&&) noexcept = delete;
  ~request() { delete[] buffer_; }
  // ---------------------------------------------------------------------------
  // OPERATORs                                                        ( public )
  //
  request& operator=(const request&) = delete;
  request& operator=(request&&) noexcept = delete;
  // ---------------------------------------------------------------------------
  // METHODs                                                          ( public )
  //
  inline void set_method(internal::marker method) {
    if (method.start >= length_ || method.start+method.length >= length_ ) {
      throw std::out_of_range("out of bounds!");
    }
    method_ = std::string_view(&buffer_[method.start], method.length);
  }
  inline void set_target(target target) { target_ = target; }
  inline void set_absolute_path(internal::marker abs_path) {
    if (abs_path.start >= length_ ||
        abs_path.start + abs_path.length >= length_) {
      throw std::out_of_range("out of bounds!");
    }
    abs_path_ = std::string_view(&buffer_[abs_path.start], abs_path.length);
  }
  inline void set_query_part(internal::marker qry_part) {
    if (qry_part.start >= length_ ||
        qry_part.start + qry_part.length >= length_) {
      throw std::out_of_range("out of bounds!");
    }
    qry_part_ = std::string_view(&buffer_[qry_part.start], qry_part.length);
  }
  inline void set_headers(internal::headers_markers hdrs, std::size_t len) {
    for (auto i = 0; i < len; i++) {
      if (hdrs.data_[i].name.start >= length_ ||
          hdrs.data_[i].name.start + hdrs.data_[i].name.length >= length_) {
        throw std::out_of_range("out of bounds!");
      }
      if (hdrs.data_[i].value.start >= length_ ||
          hdrs.data_[i].value.start + hdrs.data_[i].value.length >= length_) {
        throw std::out_of_range("out of bounds!");
      }
      headers_.add(std::string_view(&buffer_[hdrs.data_[i].name.start],
                                    hdrs.data_[i].name.length),
                   std::string_view(&buffer_[hdrs.data_[i].value.start],
                                    hdrs.data_[i].value.length));
    }
  }
  inline auto get_method() const { return method_; }
  inline auto get_target() const { return target_; }
  inline auto get_absolute_path() const { return abs_path_; }
  inline auto get_query() const { return qry_part_; }
  inline auto get_header(std::size_t i) const { return headers_.at(i); }
  inline auto get_headers_length() const { return headers_.length(); }
  inline auto has_body() const { return false; }
  inline auto get_body_length() const { return 0; }
  // ---------------------------------------------------------------------------
  // STATIC-METHODs                                                   ( public )
  //
  static request* from(const char* const buffer, std::size_t length) {
    return new request(buffer, length);
  }

 private:
  // ---------------------------------------------------------------------------
  // CONSTRUCTORs/DESTRUCTORs                                        ( private )
  //
  request(const char* const buffer, std::size_t length) {
    if (char* alloc = new char[length]) {
      std::memcpy(alloc, buffer, length);
      buffer_ = alloc;
      length_ = length;
    }
  }
  // ---------------------------------------------------------------------------
  // ATTRIBUTEs                                                      ( private )
  //
  char* buffer_ = nullptr;
  std::size_t length_ = 0;
  target target_ = target::kUnknown;
  std::string_view abs_path_;
  std::string_view qry_part_;
  std::string_view method_;
  headers headers_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
