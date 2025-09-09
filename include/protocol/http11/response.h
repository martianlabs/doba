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

#include "common/virtual_buffer.h"
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
    buf_sz_ = kDefaultResponseFullSizeInMemory;
    bod_sz_ = kDefaultResponseBodySizeInMemory;
    buf_ = (char*)malloc(buf_sz_);
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
  message& continue_100() {
    sln_ = EAS(SL(100_CONTINUE));
    sln_sz_ = sizeof(EAS(SL(100_CONTINUE))) - 1;
    return set();
  }
  message& switching_protocols_101() {
    sln_ = EAS(SL(101_SWITCHING_PROTOCOLS));
    sln_sz_ = sizeof(EAS(SL(101_SWITCHING_PROTOCOLS))) - 1;
    return set();
  }
  message& ok_200() {
    sln_ = EAS(SL(200_OK));
    sln_sz_ = sizeof(EAS(SL(200_OK))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& created_201() {
    sln_ = EAS(SL(201_CREATED));
    sln_sz_ = sizeof(EAS(SL(201_CREATED))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& accepted_202() {
    sln_ = EAS(SL(202_ACCEPTED));
    sln_sz_ = sizeof(EAS(SL(202_ACCEPTED))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& non_authoritative_information_203() {
    sln_ = EAS(SL(203_NON_AUTHORITATIVE_INFORMATION));
    sln_sz_ = sizeof(EAS(SL(203_NON_AUTHORITATIVE_INFORMATION))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& no_content_204() {
    sln_ = EAS(SL(204_NO_CONTENT));
    sln_sz_ = sizeof(EAS(SL(204_NO_CONTENT))) - 1;
    return set();
  }
  message& reset_content_205() {
    sln_ = EAS(SL(205_RESET_CONTENT));
    sln_sz_ = sizeof(EAS(SL(205_RESET_CONTENT))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& partial_content_206() {
    sln_ = EAS(SL(206_PARTIAL_CONTENT));
    sln_sz_ = sizeof(EAS(SL(206_PARTIAL_CONTENT))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& multiple_choices_300() {
    sln_ = EAS(SL(300_MULTIPLE_CHOICES));
    sln_sz_ = sizeof(EAS(SL(300_MULTIPLE_CHOICES))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& moved_permanently_301() {
    sln_ = EAS(SL(301_MOVED_PERMANENTLY));
    sln_sz_ = sizeof(EAS(SL(301_MOVED_PERMANENTLY))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& found_302() {
    sln_ = EAS(SL(302_FOUND));
    sln_sz_ = sizeof(EAS(SL(302_FOUND))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& see_other_303() {
    sln_ = EAS(SL(303_SEE_OTHER));
    sln_sz_ = sizeof(EAS(SL(303_SEE_OTHER))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& not_modified_304() {
    sln_ = EAS(SL(304_NOT_MODIFIED));
    sln_sz_ = sizeof(EAS(SL(304_NOT_MODIFIED))) - 1;
    return set();
  }
  message& use_proxy_305() {
    sln_ = EAS(SL(305_USE_PROXY));
    sln_sz_ = sizeof(EAS(SL(305_USE_PROXY))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& unused_306() {
    sln_ = EAS(SL(306_UNUSED));
    sln_sz_ = sizeof(EAS(SL(306_UNUSED))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& temporary_redirect_307() {
    sln_ = EAS(SL(307_TEMPORARY_REDIRECT));
    sln_sz_ = sizeof(EAS(SL(307_TEMPORARY_REDIRECT))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& permanent_redirect_308() {
    sln_ = EAS(SL(308_PERMANENT_REDIRECT));
    sln_sz_ = sizeof(EAS(SL(308_PERMANENT_REDIRECT))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& bad_request_400() {
    sln_ = EAS(SL(400_BAD_REQUEST));
    sln_sz_ = sizeof(EAS(SL(400_BAD_REQUEST))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& unauthorized_401() {
    sln_ = EAS(SL(401_UNAUTHORIZED));
    sln_sz_ = sizeof(EAS(SL(401_UNAUTHORIZED))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& payment_required_402() {
    sln_ = EAS(SL(402_PAYMENT_REQUIRED));
    sln_sz_ = sizeof(EAS(SL(402_PAYMENT_REQUIRED))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& forbidden_403() {
    sln_ = EAS(SL(403_FORBIDDEN));
    sln_sz_ = sizeof(EAS(SL(403_FORBIDDEN))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& not_found_404() {
    sln_ = EAS(SL(404_NOT_FOUND));
    sln_sz_ = sizeof(EAS(SL(404_NOT_FOUND))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& method_not_allowed_405() {
    sln_ = EAS(SL(405_METHOD_NOT_ALLOWED));
    sln_sz_ = sizeof(EAS(SL(405_METHOD_NOT_ALLOWED))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& not_acceptable_406() {
    sln_ = EAS(SL(406_NOT_ACCEPTABLE));
    sln_sz_ = sizeof(EAS(SL(406_NOT_ACCEPTABLE))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& proxy_authentication_required_407() {
    sln_ = EAS(SL(407_PROXY_AUTHENTICATION_REQUIRED));
    sln_sz_ = sizeof(EAS(SL(407_PROXY_AUTHENTICATION_REQUIRED))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& request_timeout_408() {
    sln_ = EAS(SL(408_REQUEST_TIMEOUT));
    sln_sz_ = sizeof(EAS(SL(408_REQUEST_TIMEOUT))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& conflict_409() {
    sln_ = EAS(SL(409_CONFLICT));
    sln_sz_ = sizeof(EAS(SL(409_CONFLICT))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& gone_410() {
    sln_ = EAS(SL(410_GONE));
    sln_sz_ = sizeof(EAS(SL(410_GONE))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& length_required_411() {
    sln_ = EAS(SL(411_LENGTH_REQUIRED));
    sln_sz_ = sizeof(EAS(SL(411_LENGTH_REQUIRED))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& precondition_failed_412() {
    sln_ = EAS(SL(412_PRECONDITION_FAILED));
    sln_sz_ = sizeof(EAS(SL(412_PRECONDITION_FAILED))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& content_too_large_413() {
    sln_ = EAS(SL(413_CONTENT_TOO_LARGE));
    sln_sz_ = sizeof(EAS(SL(413_CONTENT_TOO_LARGE))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& uri_too_long_414() {
    sln_ = EAS(SL(414_URI_TOO_LONG));
    sln_sz_ = sizeof(EAS(SL(414_URI_TOO_LONG))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& unsupported_media_type_415() {
    sln_ = EAS(SL(415_UNSUPPORTED_MEDIA_TYPE));
    sln_sz_ = sizeof(EAS(SL(415_UNSUPPORTED_MEDIA_TYPE))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& range_not_satisfiable_416() {
    sln_ = EAS(SL(416_RANGE_NOT_SATISFIABLE));
    sln_sz_ = sizeof(EAS(SL(416_RANGE_NOT_SATISFIABLE))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& expectation_failed_417() {
    sln_ = EAS(SL(417_EXPECTATION_FAILED));
    sln_sz_ = sizeof(EAS(SL(417_EXPECTATION_FAILED))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& unused_418() {
    sln_ = EAS(SL(418_UNUSED));
    sln_sz_ = sizeof(EAS(SL(418_UNUSED))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& misdirected_request_421() {
    sln_ = EAS(SL(421_MISDIRECTED_REQUEST));
    sln_sz_ = sizeof(EAS(SL(421_MISDIRECTED_REQUEST))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& unprocessable_content_422() {
    sln_ = EAS(SL(422_UNPROCESSABLE_CONTENT));
    sln_sz_ = sizeof(EAS(SL(422_UNPROCESSABLE_CONTENT))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& upgrade_required_426() {
    sln_ = EAS(SL(426_UPGRADE_REQUIRED));
    sln_sz_ = sizeof(EAS(SL(426_UPGRADE_REQUIRED))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& internal_server_error_500() {
    sln_ = EAS(SL(500_INTERNAL_SERVER_ERROR));
    sln_sz_ = sizeof(EAS(SL(500_INTERNAL_SERVER_ERROR))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& not_implemented_501() {
    sln_ = EAS(SL(501_NOT_IMPLEMENTED));
    sln_sz_ = sizeof(EAS(SL(501_NOT_IMPLEMENTED))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& bad_gateway_502() {
    sln_ = EAS(SL(502_BAD_GATEWAY));
    sln_sz_ = sizeof(EAS(SL(502_BAD_GATEWAY))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& service_unavailable_503() {
    sln_ = EAS(SL(503_SERVICE_UNAVAILABLE));
    sln_sz_ = sizeof(EAS(SL(503_SERVICE_UNAVAILABLE))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& gateway_timeout_504() {
    sln_ = EAS(SL(504_GATEWAY_TIMEOUT));
    sln_sz_ = sizeof(EAS(SL(504_GATEWAY_TIMEOUT))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  message& http_version_not_supported_505() {
    sln_ = EAS(SL(505_HTTP_VERSION_NOT_SUPPORTED));
    sln_sz_ = sizeof(EAS(SL(505_HTTP_VERSION_NOT_SUPPORTED))) - 1;
    return set().add_header(headers::kDate, helpers::get_current_date());
  }
  response& remove_header(std::string_view key) {
    message_.remove_header(key);
    return *this;
  }

 private:
  // ___________________________________________________________________________
  // CONSTANTs                                                       ( private )
  //
  static constexpr std::size_t kDefaultResponseFullSizeInMemory = 4096;  // 4kb.
  static constexpr std::size_t kDefaultResponseBodySizeInMemory = 2048;  // 2kb.
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  message& set() {
    auto hdr_buf_size = buf_sz_ - sln_sz_ - bod_sz_;
    auto hdr_buf = &buf_[sln_sz_];
    auto bod_buf = &buf_[sln_sz_ + hdr_buf_size];
    return message_.set(hdr_buf, hdr_buf_size, bod_buf, bod_sz_, 0);
  }
  void reset() {
    sln_ = nullptr;
    sln_sz_ = 0;
    message_.reset();
  }
  std::shared_ptr<common::virtual_buffer> serialize() {
    std::size_t hd_len = message_.get_headers_length();
    std::size_t bd_len = message_.get_body_length();
    std::size_t sl_off = sln_sz_ + hd_len;
    memcpy(buf_, sln_, sln_sz_);
    buf_[sl_off] = constants::character::kCr;
    buf_[sl_off + 1] = constants::character::kLf;
    memcpy(&buf_[sl_off + 2], &buf_[buf_sz_ - bod_sz_], bd_len);
    return std::make_shared<common::virtual_buffer>(buf_, sl_off + bd_len + 2);
  }
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  char* buf_ = nullptr;
  std::size_t buf_sz_ = 0;
  std::size_t bod_sz_ = 0;
  const char* sln_ = nullptr;
  std::size_t sln_sz_ = 0;
  message message_;
  // ___________________________________________________________________________
  // FRIENDs                                                         ( private )
  //
  friend class decoder;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
