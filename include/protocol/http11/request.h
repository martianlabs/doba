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

#include <charconv>
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
  inline auto get_method() const { return method_; }
  inline auto get_target() const { return target_; }
  inline auto get_absolute_path() const { return abs_path_; }
  inline auto get_query() const { return qry_part_; }
  inline auto get_header(std::size_t i) const { return headers_.at(i); }
  inline auto get_headers_length() const { return headers_used_; }
  inline auto has_body() const { return false; }
  inline auto get_body_length() const { return 0; }
  // ---------------------------------------------------------------------------
  // STATIC-METHODs                                                   ( public )
  //
  static request* from(const char* const buffer, std::size_t length,
                       target target, internal::marker method,
                       internal::marker abs_path, internal::marker qry_part,
                       internal::headers_markers headers,
                       std::size_t headers_length) {
    return new request(buffer, length, target, method, abs_path, qry_part,
                       headers, headers_length);
  }

 private:
  // ---------------------------------------------------------------------------
  // CONSTRUCTORs/DESTRUCTORs                                        ( private )
  //
  request(const char* const buffer, std::size_t length, target target,
          internal::marker method, internal::marker abs_path,
          internal::marker qry_part, internal::headers_markers headers,
          std::size_t headers_length) {
    if (char* alloc = new char[length]) {
      std::memcpy(alloc, buffer, length);
      buffer_ = alloc;
      length_ = length;
      target_ = target;
      abs_path_ = std::string_view(&buffer_[abs_path.start], abs_path.length);
      qry_part_ = std::string_view(&buffer_[qry_part.start], qry_part.length);
      method_ = std::string_view(&buffer_[method.start], method.length);
      for (auto i = 0; i < headers_length; i++) {
        headers_.add(std::string_view(&buffer_[headers.data_[i].name.start],
                                      headers.data_[i].name.length),
                     std::string_view(&buffer_[headers.data_[i].value.start],
                                      headers.data_[i].value.length));
      }
    }
  }
  // ---------------------------------------------------------------------------
  // STATIC-METHODs                                                  ( private )
  //
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
  std::size_t headers_used_ = 0;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
