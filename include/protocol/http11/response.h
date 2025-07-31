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

#ifndef martianlabs_doba_protocol_http11_response_h
#define martianlabs_doba_protocol_http11_response_h

#include "reference_buffer.h"
#include "status_line.h"
#include "message.h"

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// response                                                            ( class )
// -----------------------------------------------------------------------------
// This class holds for the http 1.1 response implementation.
// -----------------------------------------------------------------------------
// =============================================================================
class response {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  response() {
    hsz_ = kDefaultHeadersSectionMemory;
    bsz_ = kDefaultBodySectionMemory;
    buf_ = (char*)malloc(hsz_ + bsz_);
    reference_ = std::make_shared<reference_buffer>();
  }
  response(const response&) = delete;
  response(response&&) noexcept = delete;
  ~response() { free(buf_); }
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  response& operator=(const response&) = delete;
  response& operator=(response&&) noexcept = delete;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  inline message& continue_100() {
    sln_ = EAS(SL(100_CONTINUE));
    sln_cur_ = sizeof(EAS(SL(100_CONTINUE))) - 1;
    return prepare();
  }
  inline message& switching_protocols_101() {
    sln_ = EAS(SL(101_SWITCHING_PROTOCOLS));
    sln_cur_ = sizeof(EAS(SL(101_SWITCHING_PROTOCOLS))) - 1;
    return prepare();
  }
  inline message& ok_200() {
    sln_ = EAS(SL(200_OK));
    sln_cur_ = sizeof(EAS(SL(200_OK))) - 1;
    return prepare();
  }
  inline message& created_201() {
    sln_ = EAS(SL(201_CREATED));
    sln_cur_ = sizeof(EAS(SL(201_CREATED))) - 1;
    return prepare();
  }
  inline message& accepted_202() {
    sln_ = EAS(SL(202_ACCEPTED));
    sln_cur_ = sizeof(EAS(SL(202_ACCEPTED))) - 1;
    return prepare();
  }
  inline message& non_authoritative_information_203() {
    sln_ = EAS(SL(203_NON_AUTHORITATIVE_INFORMATION));
    sln_cur_ = sizeof(EAS(SL(203_NON_AUTHORITATIVE_INFORMATION))) - 1;
    return prepare();
  }
  inline message& no_content_204() {
    sln_ = EAS(SL(204_NO_CONTENT));
    sln_cur_ = sizeof(EAS(SL(204_NO_CONTENT))) - 1;
    return prepare();
  }
  inline message& reset_content_205() {
    sln_ = EAS(SL(205_RESET_CONTENT));
    sln_cur_ = sizeof(EAS(SL(205_RESET_CONTENT))) - 1;
    return prepare();
  }
  inline message& partial_content_206() {
    sln_ = EAS(SL(206_PARTIAL_CONTENT));
    sln_cur_ = sizeof(EAS(SL(206_PARTIAL_CONTENT))) - 1;
    return prepare();
  }
  inline message& multiple_choices_300() {
    sln_ = EAS(SL(300_MULTIPLE_CHOICES));
    sln_cur_ = sizeof(EAS(SL(300_MULTIPLE_CHOICES))) - 1;
    return prepare();
  }
  inline message& moved_permanently_301() {
    sln_ = EAS(SL(301_MOVED_PERMANENTLY));
    sln_cur_ = sizeof(EAS(SL(301_MOVED_PERMANENTLY))) - 1;
    return prepare();
  }
  inline message& found_302() {
    sln_ = EAS(SL(302_FOUND));
    sln_cur_ = sizeof(EAS(SL(302_FOUND))) - 1;
    return prepare();
  }
  inline message& see_other_303() {
    sln_ = EAS(SL(303_SEE_OTHER));
    sln_cur_ = sizeof(EAS(SL(303_SEE_OTHER))) - 1;
    return prepare();
  }
  inline message& not_modified_304() {
    sln_ = EAS(SL(304_NOT_MODIFIED));
    sln_cur_ = sizeof(EAS(SL(304_NOT_MODIFIED))) - 1;
    return prepare();
  }
  inline message& use_proxy_305() {
    sln_ = EAS(SL(305_USE_PROXY));
    sln_cur_ = sizeof(EAS(SL(305_USE_PROXY))) - 1;
    return prepare();
  }
  inline message& unused_306() {
    sln_ = EAS(SL(306_UNUSED));
    sln_cur_ = sizeof(EAS(SL(306_UNUSED))) - 1;
    return prepare();
  }
  inline message& temporary_redirect_307() {
    sln_ = EAS(SL(307_TEMPORARY_REDIRECT));
    sln_cur_ = sizeof(EAS(SL(307_TEMPORARY_REDIRECT))) - 1;
    return prepare();
  }
  inline message& permanent_redirect_308() {
    sln_ = EAS(SL(308_PERMANENT_REDIRECT));
    sln_cur_ = sizeof(EAS(SL(308_PERMANENT_REDIRECT))) - 1;
    return prepare();
  }
  inline message& bad_request_400() {
    sln_ = EAS(SL(400_BAD_REQUEST));
    sln_cur_ = sizeof(EAS(SL(400_BAD_REQUEST))) - 1;
    return prepare();
  }
  inline message& unauthorized_401() {
    sln_ = EAS(SL(401_UNAUTHORIZED));
    sln_cur_ = sizeof(EAS(SL(401_UNAUTHORIZED))) - 1;
    return prepare();
  }
  inline message& payment_required_402() {
    sln_ = EAS(SL(402_PAYMENT_REQUIRED));
    sln_cur_ = sizeof(EAS(SL(402_PAYMENT_REQUIRED))) - 1;
    return prepare();
  }
  inline message& forbidden_403() {
    sln_ = EAS(SL(403_FORBIDDEN));
    sln_cur_ = sizeof(EAS(SL(403_FORBIDDEN))) - 1;
    return prepare();
  }
  inline message& not_found_404() {
    sln_ = EAS(SL(404_NOT_FOUND));
    sln_cur_ = sizeof(EAS(SL(404_NOT_FOUND))) - 1;
    return prepare();
  }
  inline message& method_not_allowed_405() {
    sln_ = EAS(SL(405_METHOD_NOT_ALLOWED));
    sln_cur_ = sizeof(EAS(SL(405_METHOD_NOT_ALLOWED))) - 1;
    return prepare();
  }
  inline message& not_acceptable_406() {
    sln_ = EAS(SL(406_NOT_ACCEPTABLE));
    sln_cur_ = sizeof(EAS(SL(406_NOT_ACCEPTABLE))) - 1;
    return prepare();
  }
  inline message& proxy_authentication_required_407() {
    sln_ = EAS(SL(407_PROXY_AUTHENTICATION_REQUIRED));
    sln_cur_ = sizeof(EAS(SL(407_PROXY_AUTHENTICATION_REQUIRED))) - 1;
    return prepare();
  }
  inline message& request_timeout_408() {
    sln_ = EAS(SL(408_REQUEST_TIMEOUT));
    sln_cur_ = sizeof(EAS(SL(408_REQUEST_TIMEOUT))) - 1;
    return prepare();
  }
  inline message& conflict_409() {
    sln_ = EAS(SL(409_CONFLICT));
    sln_cur_ = sizeof(EAS(SL(409_CONFLICT))) - 1;
    return prepare();
  }
  inline message& gone_410() {
    sln_ = EAS(SL(410_GONE));
    sln_cur_ = sizeof(EAS(SL(410_GONE))) - 1;
    return prepare();
  }
  inline message& length_required_411() {
    sln_ = EAS(SL(411_LENGTH_REQUIRED));
    sln_cur_ = sizeof(EAS(SL(411_LENGTH_REQUIRED))) - 1;
    return prepare();
  }
  inline message& precondition_failed_412() {
    sln_ = EAS(SL(412_PRECONDITION_FAILED));
    sln_cur_ = sizeof(EAS(SL(412_PRECONDITION_FAILED))) - 1;
    return prepare();
  }
  inline message& content_too_large_413() {
    sln_ = EAS(SL(413_CONTENT_TOO_LARGE));
    sln_cur_ = sizeof(EAS(SL(413_CONTENT_TOO_LARGE))) - 1;
    return prepare();
  }
  inline message& uri_too_long_414() {
    sln_ = EAS(SL(414_URI_TOO_LONG));
    sln_cur_ = sizeof(EAS(SL(414_URI_TOO_LONG))) - 1;
    return prepare();
  }
  inline message& unsupported_media_type_415() {
    sln_ = EAS(SL(415_UNSUPPORTED_MEDIA_TYPE));
    sln_cur_ = sizeof(EAS(SL(415_UNSUPPORTED_MEDIA_TYPE))) - 1;
    return prepare();
  }
  inline message& range_not_satisfiable_416() {
    sln_ = EAS(SL(416_RANGE_NOT_SATISFIABLE));
    sln_cur_ = sizeof(EAS(SL(416_RANGE_NOT_SATISFIABLE))) - 1;
    return prepare();
  }
  inline message& expectation_failed_417() {
    sln_ = EAS(SL(417_EXPECTATION_FAILED));
    sln_cur_ = sizeof(EAS(SL(417_EXPECTATION_FAILED))) - 1;
    return prepare();
  }
  inline message& unused_418() {
    sln_ = EAS(SL(418_UNUSED));
    sln_cur_ = sizeof(EAS(SL(418_UNUSED))) - 1;
    return prepare();
  }
  inline message& misdirected_request_421() {
    sln_ = EAS(SL(421_MISDIRECTED_REQUEST));
    sln_cur_ = sizeof(EAS(SL(421_MISDIRECTED_REQUEST))) - 1;
    return prepare();
  }
  inline message& unprocessable_content_422() {
    sln_ = EAS(SL(422_UNPROCESSABLE_CONTENT));
    sln_cur_ = sizeof(EAS(SL(422_UNPROCESSABLE_CONTENT))) - 1;
    return prepare();
  }
  inline message& upgrade_required_426() {
    sln_ = EAS(SL(426_UPGRADE_REQUIRED));
    sln_cur_ = sizeof(EAS(SL(426_UPGRADE_REQUIRED))) - 1;
    return prepare();
  }
  inline message& internal_server_error_500() {
    sln_ = EAS(SL(500_INTERNAL_SERVER_ERROR));
    sln_cur_ = sizeof(EAS(SL(500_INTERNAL_SERVER_ERROR))) - 1;
    return prepare();
  }
  inline message& not_implemented_501() {
    sln_ = EAS(SL(501_NOT_IMPLEMENTED));
    sln_cur_ = sizeof(EAS(SL(501_NOT_IMPLEMENTED))) - 1;
    return prepare();
  }
  inline message& bad_gateway_502() {
    sln_ = EAS(SL(502_BAD_GATEWAY));
    sln_cur_ = sizeof(EAS(SL(502_BAD_GATEWAY))) - 1;
    return prepare();
  }
  inline message& service_unavailable_503() {
    sln_ = EAS(SL(503_SERVICE_UNAVAILABLE));
    sln_cur_ = sizeof(EAS(SL(503_SERVICE_UNAVAILABLE))) - 1;
    return prepare();
  }
  inline message& gateway_timeout_504() {
    sln_ = EAS(SL(504_GATEWAY_TIMEOUT));
    sln_cur_ = sizeof(EAS(SL(504_GATEWAY_TIMEOUT))) - 1;
    return prepare();
  }
  inline message& http_version_not_supported_505() {
    sln_ = EAS(SL(505_HTTP_VERSION_NOT_SUPPORTED));
    sln_cur_ = sizeof(EAS(SL(505_HTTP_VERSION_NOT_SUPPORTED))) - 1;
    return prepare();
  }
  inline std::shared_ptr<reference_buffer> serialize() {
    std::size_t cur = 0;
    std::size_t hdr_off = message_.headers_length();
    std::size_t bod_off = message_.body_length();
    memcpy(buf_, sln_, sln_cur_);
    buf_[sln_cur_ + hdr_off] = constants::character::kCr;
    buf_[sln_cur_ + hdr_off + 1] = constants::character::kLf;
    memcpy(&buf_[sln_cur_ + hdr_off + 2], &buf_[bsz_], bod_off);
    reference_->set(buf_, sln_cur_ + hdr_off + bod_off + 2);
    return reference_;
  }
  inline void reset() {
    sln_ = nullptr;
    sln_cur_ = 0;
    message_.reset();
  }

 private:
  // ___________________________________________________________________________
  // CONSTANTs                                                       ( private )
  //
  static constexpr uint32_t kDefaultHeadersSectionMemory = 4096;  // 4kb.
  static constexpr uint32_t kDefaultBodySectionMemory = 4096;     // 4kb.
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  inline message& prepare() {
    return message_.prepare(&buf_[sln_cur_], (hsz_ + bsz_) - sln_cur_, bsz_);
  }
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  char* buf_ = nullptr;
  std::size_t hsz_ = 0;
  std::size_t bsz_ = 0;
  const char* sln_ = nullptr;
  std::size_t sln_cur_ = 0;
  message message_;
  std::shared_ptr<reference_buffer> reference_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
