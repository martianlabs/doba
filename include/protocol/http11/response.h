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
//          material, or evaluation data for AI systems is expresslny
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

#ifndef martianlabs_doba_protocol_http11_response_h
#define martianlabs_doba_protocol_http11_response_h

#include "status_line.h"

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// response                                                            ( class )
// -----------------------------------------------------------------------------
// This class holds for the http 1.1 response implementation.
// -----------------------------------------------------------------------------
// =============================================================================
class response {
 public:
  // ---------------------------------------------------------------------------
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  response() = default;
  response(const response&) = delete;
  response(response&& in) noexcept = delete;
  ~response() = default;
  // ---------------------------------------------------------------------------
  // OPERATORs                                                        ( public )
  //
  response& operator=(const response&) = delete;
  response& operator=(response&& in) noexcept = delete;
  // ---------------------------------------------------------------------------
  // METHODs                                                          ( public )
  //
  std::string_view serialize() {
    buffer_.assign(status_line_).append(headers_).append("\r\n").append(body_);
    return std::string_view(buffer_);
  }
  response& add_header(std::string_view k, std::string_view v) {
    headers_.append(k).append(": ").append(v).append("\r\n");
    return *this;
  }
  template <typename T>
    requires std::is_arithmetic_v<T>
  response& add_header(std::string_view key, const T& val) {
    return add_header(key, std::to_string(val));
  }
  response& set_body(std::string_view sv) {
    body_ = sv;
    return add_header(headers::kContentLength, sv.length());
  }
  template <typename T>
    requires std::is_arithmetic_v<T>
  response& set_body(T&& val) {
    return set_body(std::to_string(val));
  }
  response& continue_100() { return sln(status_line::k100); }
  response& switching_protocols_101() { return sln(status_line::k101); }
  response& ok_200() { return sln(status_line::k200); }
  response& created_201() { return sln(status_line::k201); }
  response& accepted_202() { return sln(status_line::k202); }
  response& non_authoritative_info_203() { return sln(status_line::k203); }
  response& no_content_204() { return sln(status_line::k204); }
  response& reset_content_205() { return sln(status_line::k205); }
  response& partial_content_206() { return sln(status_line::k206); }
  response& multiple_choices_300() { return sln(status_line::k300); }
  response& moved_permanently_301() { return sln(status_line::k301); }
  response& found_302() { return sln(status_line::k302); }
  response& see_other_303() { return sln(status_line::k303); }
  response& not_modified_304() { return sln(status_line::k304); }
  response& use_proxy_305() { return sln(status_line::k305); }
  response& unused_306() { return sln(status_line::k306); }
  response& temporary_redirect_307() { return sln(status_line::k307); }
  response& permanent_redirect_308() { return sln(status_line::k308); }
  response& bad_request_400() { return sln(status_line::k400); }
  response& unauthorized_401() { return sln(status_line::k401); }
  response& payment_required_402() { return sln(status_line::k402); }
  response& forbidden_403() { return sln(status_line::k403); }
  response& not_found_404() { return sln(status_line::k404); }
  response& method_not_allowed_405() { return sln(status_line::k405); }
  response& not_acceptable_406() { return sln(status_line::k406); }
  response& proxy_auth_required_407() { return sln(status_line::k407); }
  response& request_timeout_408() { return sln(status_line::k408); }
  response& conflict_409() { return sln(status_line::k409); }
  response& gone_410() { return sln(status_line::k410); }
  response& length_required_411() { return sln(status_line::k411); }
  response& precondition_failed_412() { return sln(status_line::k412); }
  response& content_too_large_413() { return sln(status_line::k413); }
  response& uri_too_long_414() { return sln(status_line::k414); }
  response& unsupported_media_type_415() { return sln(status_line::k415); }
  response& range_not_satisfiable_416() { return sln(status_line::k416); }
  response& expectation_failed_417() { return sln(status_line::k417); }
  response& unused_418() { return sln(status_line::k418); }
  response& misdirected_request_421() { return sln(status_line::k421); }
  response& unprocessable_content_422() { return sln(status_line::k422); }
  response& upgrade_required_426() { return sln(status_line::k426); }
  response& internal_server_error_500() { return sln(status_line::k500); }
  response& not_implemented_501() { return sln(status_line::k501); }
  response& bad_gateway_502() { return sln(status_line::k502); }
  response& service_unavailable_503() { return sln(status_line::k503); }
  response& gateway_timeout_504() { return sln(status_line::k504); }
  response& http_version_not_supported_505() { return sln(status_line::k505); }

 private:
  // ---------------------------------------------------------------------------
  // METHODs                                                         ( private )
  //
  response& sln(auto&& status_line) {
    status_line_.assign(status_line);
    return *this;
  }
  // ---------------------------------------------------------------------------
  // ATTRIBUTEs                                                      ( private )
  //
  std::string buffer_;
  std::string status_line_;
  std::string headers_;
  std::string body_;
  std::shared_ptr<std::istream> body_stream_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
