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

#ifndef martianlabs_doba_protocol_http11_response_h
#define martianlabs_doba_protocol_http11_response_h

#include "status_line.h"
#include "common/rob.h"

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
  response() {
    std::size_t required_memory_for_response =
        constants::limits::kDefaultCoreMsgMaxSizeInRam +
        constants::limits::kDefaultBodyMsgMaxSizeInRam;
    auto* buf = new char[required_memory_for_response];
    auto* rco = new common::rob();
    auto* rbo = new common::rob();
    if (buf && rco && rbo) {
      size_ = required_memory_for_response;
      core_size_ = size_ - constants::limits::kDefaultBodyMsgMaxSizeInRam;
      body_size_ = size_ - constants::limits::kDefaultCoreMsgMaxSizeInRam;
      buffer_ = buf;
      rob_core = rco;
      rob_body = rbo;
      rob_next = rob_core;
    } else {
      delete[] buf;
      delete rco;
      delete rbo;
    }
  }
  response(const response&) = delete;
  response(response&& in) noexcept = delete;
  ~response() {
    delete[] buffer_;
    delete rob_core;
    delete rob_body;
  }
  // ---------------------------------------------------------------------------
  // OPERATORs                                                        ( public )
  //
  response& operator=(const response&) = delete;
  response& operator=(response&& in) noexcept = delete;
  // ---------------------------------------------------------------------------
  // METHODs                                                          ( public )
  //
  inline const common::rob* const serialize() {
    static constexpr auto eol = constants::string::kCrLf;
    static constexpr auto eol_len = sizeof(constants::string::kCrLf) - 1;
    if (!serialized_) {
      // [headers] end section!
      if (size_ - core_cursor_ >= eol_len) {
        memcpy(&buffer_[core_cursor_], eol, eol_len);
        core_cursor_ += eol_len;
      }
      // [body] section!
      if (!body_stream_) {
        std::memmove(&buffer_[core_cursor_],
                     &buffer_[constants::limits::kDefaultCoreMsgMaxSizeInRam],
                     body_cursor_);
        core_cursor_ += body_cursor_;
        rob_core->set(buffer_, core_cursor_);
      } else {
        rob_core->set(buffer_, core_cursor_);
        rob_body->set(body_stream_);
      }
      rob_next = rob_core;
      serialized_ = true;
    }
    common::rob* rob_returned = rob_next;
    rob_next = rob_next == rob_core ? rob_body : nullptr;
    return rob_returned;
  }
  inline response& add_header(std::string_view k, std::string_view v) {
    std::size_t k_sz = k.size(), v_sz = v.size();
    std::size_t entry_length = k_sz + v_sz + 3;
    if ((core_size_ - core_cursor_) <= (entry_length + 2)) {
      throw std::runtime_error("headers section too large to fit in memory!");
    }
    if (!k.size()) {
      return *this;
    }
    auto initial_cur = core_cursor_;
    for (const char& c : k) {
      if (!helpers::is_token(c)) {
        core_cursor_ = initial_cur;
        return *this;
      }
      buffer_[core_cursor_++] = helpers::tolower_ascii(c);
    }
    buffer_[core_cursor_++] = constants::character::kColon;
    for (const char& c : v) {
      if (!(helpers::is_vchar(c) || helpers::is_obs_text(c) ||
            c == constants::character::kSpace ||
            c == constants::character::kHTab)) {
        return *this;
      }
    }
    memcpy(&buffer_[core_cursor_], v.data(), v_sz);
    core_cursor_ += v_sz;
    buffer_[core_cursor_++] = constants::character::kCr;
    buffer_[core_cursor_++] = constants::character::kLf;
    return *this;
  }
  template <typename T>
    requires std::is_arithmetic_v<T>
  inline response& add_header(std::string_view key, const T& val) {
    return add_header(key, std::to_string(val));
  }
  inline response& remove_header(std::string_view k) {
    bool matched = true;
    bool searching_for_key = true;
    std::size_t i = core_headers_start_, j = 0, k_start = i, k_sz = k.size();
    while (i < core_cursor_) {
      switch (buffer_[i]) {
        case constants::character::kColon:
          if (searching_for_key) {
            searching_for_key = false;
          }
          break;
        case constants::character::kCr:
          if (searching_for_key) {
            return *this;
          }
          if (matched && j == k_sz) {
            auto const off = i + 2;
            std::memmove(&buffer_[k_start], &buffer_[off], core_cursor_ - off);
            core_cursor_ -= (off - k_start);
            std::memset(&buffer_[core_cursor_], 0, core_size_ - core_cursor_);
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
  inline response& set_body(const char* const buf, std::size_t len) {
    if (body_size_ - body_cursor_ < len) {
      throw std::runtime_error("body too large to fit in memory!");
    }
    std::memcpy(&buffer_[core_size_], buf, len);
    body_cursor_ += len;
    return add_header(headers::kContentLength, len);
  }
  inline response& set_body(std::string_view sv) {
    return set_body(sv.data(), sv.size());
  }
  template <typename T>
    requires std::is_arithmetic_v<T>
  inline response& set_body(T&& val) {
    return set_body(std::to_string(val));
  }
  inline response& set_body(std::shared_ptr<std::istream> stream) {
    if (!(body_stream_ = stream)) {
      throw std::invalid_argument("input stream is empty!");
    }
    std::streampos current = body_stream_->tellg();
    if (current == std::streampos(-1)) {
      throw std::runtime_error("input stream not readable!");
    }
    body_stream_->seekg(0, std::ios::end);
    std::streampos off = body_stream_->tellg();
    if (off == std::streampos(-1)) {
      throw std::runtime_error("input stream not readable!");
    }
    std::size_t body_length = static_cast<std::size_t>(off);
    body_stream_->seekg(current, std::ios::beg);
    return add_header(headers::kContentLength, body_length);
  }
  inline response& continue_100() {
    return setup(EAS(SL(100_CONTINUE)), sizeof(EAS(SL(100_CONTINUE))) - 1);
  }
  inline response& switching_protocols_101() {
    return setup(EAS(SL(101_SWITCHING_PROTOCOLS)),
                 sizeof(EAS(SL(101_SWITCHING_PROTOCOLS))) - 1);
  }
  inline response& ok_200() {
    return setup(EAS(SL(200_OK)), sizeof(EAS(SL(200_OK))) - 1);
  }
  inline response& created_201() {
    return setup(EAS(SL(201_CREATED)), sizeof(EAS(SL(201_CREATED))) - 1);
  }
  inline response& accepted_202() {
    return setup(EAS(SL(202_ACCEPTED)), sizeof(EAS(SL(202_ACCEPTED))) - 1);
  }
  inline response& non_authoritative_information_203() {
    return setup(EAS(SL(203_NON_AUTHORITATIVE_INFORMATION)),
                 sizeof(EAS(SL(203_NON_AUTHORITATIVE_INFORMATION))) - 1);
  }
  inline response& no_content_204() {
    return setup(EAS(SL(204_NO_CONTENT)), sizeof(EAS(SL(204_NO_CONTENT))) - 1);
  }
  inline response& reset_content_205() {
    return setup(EAS(SL(205_RESET_CONTENT)),
                 sizeof(EAS(SL(205_RESET_CONTENT))) - 1);
  }
  inline response& partial_content_206() {
    return setup(EAS(SL(206_PARTIAL_CONTENT)),
                 sizeof(EAS(SL(206_PARTIAL_CONTENT))) - 1);
  }
  inline response& multiple_choices_300() {
    return setup(EAS(SL(300_MULTIPLE_CHOICES)),
                 sizeof(EAS(SL(300_MULTIPLE_CHOICES))) - 1);
  }
  inline response& moved_permanently_301() {
    return setup(EAS(SL(301_MOVED_PERMANENTLY)),
                 sizeof(EAS(SL(301_MOVED_PERMANENTLY))) - 1);
  }
  inline response& found_302() {
    return setup(EAS(SL(302_FOUND)), sizeof(EAS(SL(302_FOUND))) - 1);
  }
  inline response& see_other_303() {
    return setup(EAS(SL(303_SEE_OTHER)), sizeof(EAS(SL(303_SEE_OTHER))) - 1);
  }
  inline response& not_modified_304() {
    return setup(EAS(SL(304_NOT_MODIFIED)),
                 sizeof(EAS(SL(304_NOT_MODIFIED))) - 1);
  }
  inline response& use_proxy_305() {
    return setup(EAS(SL(305_USE_PROXY)), sizeof(EAS(SL(305_USE_PROXY))) - 1);
  }
  inline response& unused_306() {
    return setup(EAS(SL(306_UNUSED)), sizeof(EAS(SL(306_UNUSED))) - 1);
  }
  inline response& temporary_redirect_307() {
    return setup(EAS(SL(307_TEMPORARY_REDIRECT)),
                 sizeof(EAS(SL(307_TEMPORARY_REDIRECT))) - 1);
  }
  inline response& permanent_redirect_308() {
    return setup(EAS(SL(308_PERMANENT_REDIRECT)),
                 sizeof(EAS(SL(308_PERMANENT_REDIRECT))) - 1);
  }
  inline response& bad_request_400() {
    return setup(EAS(SL(400_BAD_REQUEST)),
                 sizeof(EAS(SL(400_BAD_REQUEST))) - 1);
  }
  inline response& unauthorized_401() {
    return setup(EAS(SL(401_UNAUTHORIZED)),
                 sizeof(EAS(SL(401_UNAUTHORIZED))) - 1);
  }
  inline response& payment_required_402() {
    return setup(EAS(SL(402_PAYMENT_REQUIRED)),
                 sizeof(EAS(SL(402_PAYMENT_REQUIRED))) - 1);
  }
  inline response& forbidden_403() {
    return setup(EAS(SL(403_FORBIDDEN)), sizeof(EAS(SL(403_FORBIDDEN))) - 1);
  }
  inline response& not_found_404() {
    return setup(EAS(SL(404_NOT_FOUND)), sizeof(EAS(SL(404_NOT_FOUND))) - 1);
  }
  inline response& method_not_allowed_405() {
    return setup(EAS(SL(405_METHOD_NOT_ALLOWED)),
                 sizeof(EAS(SL(405_METHOD_NOT_ALLOWED))) - 1);
  }
  inline response& not_acceptable_406() {
    return setup(EAS(SL(406_NOT_ACCEPTABLE)),
                 sizeof(EAS(SL(406_NOT_ACCEPTABLE))) - 1);
  }
  inline response& proxy_authentication_required_407() {
    return setup(EAS(SL(407_PROXY_AUTHENTICATION_REQUIRED)),
                 sizeof(EAS(SL(407_PROXY_AUTHENTICATION_REQUIRED))) - 1);
  }
  inline response& request_timeout_408() {
    return setup(EAS(SL(408_REQUEST_TIMEOUT)),
                 sizeof(EAS(SL(408_REQUEST_TIMEOUT))) - 1);
  }
  inline response& conflict_409() {
    return setup(EAS(SL(409_CONFLICT)), sizeof(EAS(SL(409_CONFLICT))) - 1);
  }
  inline response& gone_410() {
    return setup(EAS(SL(410_GONE)), sizeof(EAS(SL(410_GONE))) - 1);
  }
  inline response& length_required_411() {
    return setup(EAS(SL(411_LENGTH_REQUIRED)),
                 sizeof(EAS(SL(411_LENGTH_REQUIRED))) - 1);
  }
  inline response& precondition_failed_412() {
    return setup(EAS(SL(412_PRECONDITION_FAILED)),
                 sizeof(EAS(SL(412_PRECONDITION_FAILED))) - 1);
  }
  inline response& content_too_large_413() {
    return setup(EAS(SL(413_CONTENT_TOO_LARGE)),
                 sizeof(EAS(SL(413_CONTENT_TOO_LARGE))) - 1);
  }
  inline response& uri_too_long_414() {
    return setup(EAS(SL(414_URI_TOO_LONG)),
                 sizeof(EAS(SL(414_UNSUPPORTED_MEDIA_TYPE))) - 1);
  }
  inline response& unsupported_media_type_415() {
    return setup(EAS(SL(415_UNSUPPORTED_MEDIA_TYPE)),
                 sizeof(EAS(SL(415_UNSUPPORTED_MEDIA_TYPE))) - 1);
  }
  inline response& range_not_satisfiable_416() {
    return setup(EAS(SL(416_RANGE_NOT_SATISFIABLE)),
                 sizeof(EAS(SL(416_RANGE_NOT_SATISFIABLE))) - 1);
  }
  inline response& expectation_failed_417() {
    return setup(EAS(SL(417_EXPECTATION_FAILED)),
                 sizeof(EAS(SL(417_EXPECTATION_FAILED))) - 1);
  }
  inline response& unused_418() {
    return setup(EAS(SL(418_UNUSED)), sizeof(EAS(SL(418_UNUSED))) - 1);
  }
  inline response& misdirected_request_421() {
    return setup(EAS(SL(421_MISDIRECTED_REQUEST)),
                 sizeof(EAS(SL(421_MISDIRECTED_REQUEST))) - 1);
  }
  inline response& unprocessable_content_422() {
    return setup(EAS(SL(422_UNPROCESSABLE_CONTENT)),
                 sizeof(EAS(SL(422_UNPROCESSABLE_CONTENT))) - 1);
  }
  inline response& upgrade_required_426() {
    return setup(EAS(SL(426_UPGRADE_REQUIRED)),
                 sizeof(EAS(SL(426_UPGRADE_REQUIRED))) - 1);
  }
  inline response& internal_server_error_500() {
    return setup(EAS(SL(500_INTERNAL_SERVER_ERROR)),
                 sizeof(EAS(SL(500_INTERNAL_SERVER_ERROR))) - 1);
  }
  inline response& not_implemented_501() {
    return setup(EAS(SL(501_NOT_IMPLEMENTED)),
                 sizeof(EAS(SL(501_NOT_IMPLEMENTED))) - 1);
  }
  inline response& bad_gateway_502() {
    return setup(EAS(SL(502_BAD_GATEWAY)),
                 sizeof(EAS(SL(502_BAD_GATEWAY))) - 1);
  }
  inline response& service_unavailable_503() {
    return setup(EAS(SL(503_SERVICE_UNAVAILABLE)),
                 sizeof(EAS(SL(503_SERVICE_UNAVAILABLE))) - 1);
  }
  inline response& gateway_timeout_504() {
    return setup(EAS(SL(504_GATEWAY_TIMEOUT)),
                 sizeof(EAS(SL(504_GATEWAY_TIMEOUT))) - 1);
  }
  inline response& http_version_not_supported_505() {
    return setup(EAS(SL(505_HTTP_VERSION_NOT_SUPPORTED)),
                 sizeof(EAS(SL(505_HTTP_VERSION_NOT_SUPPORTED))) - 1);
  }

 private:
  // ---------------------------------------------------------------------------
  // METHODs                                                         ( private )
  //
  inline response& setup(const char* const sln, std::size_t sln_len) {
    core_cursor_ = sln_len;
    core_headers_start_ = core_cursor_;
    body_cursor_ = 0;
    body_stream_.reset();
    memcpy(buffer_, sln, core_cursor_);
    return *this;
  }
  // ---------------------------------------------------------------------------
  // ATTRIBUTEs                                                      ( private )
  //
  char* buffer_ = nullptr;
  std::size_t size_ = 0;
  std::size_t core_cursor_ = 0;
  std::size_t core_headers_start_ = 0;
  std::size_t core_size_ = 0;
  std::size_t body_cursor_ = 0;
  std::size_t body_size_ = 0;
  std::shared_ptr<std::istream> body_stream_;
  common::rob* rob_core{nullptr};
  common::rob* rob_body{nullptr};
  common::rob* rob_next{nullptr};
  bool serialized_ = false;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
