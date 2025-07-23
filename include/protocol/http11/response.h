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

#include "status_line.h"
#include "response_handler.h"

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// response                                                            ( class )
// -----------------------------------------------------------------------------
// This class holds for the http 1.1 response implementation.
// -----------------------------------------------------------------------------
// =============================================================================
template <template <typename, typename> typename TRty>
class response {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  response() { buffer_ = (char*)malloc(constants::limits::kDefaultResponseSz); }
  response(const response&) = delete;
  response(response&&) noexcept = delete;
  ~response() { free(buffer_); }
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  response& operator=(const response&) = delete;
  response& operator=(response&&) noexcept = delete;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  response_handler<response>& continue_100() {
    start_line_ = EAS(SL(100_CONTINUE));
    start_line_cur_ = sizeof(EAS(SL(100_CONTINUE))) - 1;
    return handler_;
  }
  response_handler<response>& switching_protocols_101() {
    start_line_ = EAS(SL(101_SWITCHING_PROTOCOLS));
    start_line_cur_ = sizeof(EAS(SL(101_SWITCHING_PROTOCOLS))) - 1;
    return handler_;
  }
  response_handler<response>& ok_200() {
    start_line_ = EAS(SL(200_OK));
    start_line_cur_ = sizeof(EAS(SL(200_OK))) - 1;
    return handler_;
  }
  response_handler<response>& created_201() {
    start_line_ = EAS(SL(201_CREATED));
    start_line_cur_ = sizeof(EAS(SL(201_CREATED))) - 1;
    return handler_;
  }
  response_handler<response>& accepted_202() {
    start_line_ = EAS(SL(202_ACCEPTED));
    start_line_cur_ = sizeof(EAS(SL(202_ACCEPTED))) - 1;
    return handler_;
  }
  response_handler<response>& non_authoritative_information_203() {
    start_line_ = EAS(SL(203_NON_AUTHORITATIVE_INFORMATION));
    start_line_cur_ = sizeof(EAS(SL(203_NON_AUTHORITATIVE_INFORMATION))) - 1;
    return handler_;
  }
  response_handler<response>& no_content_204() {
    start_line_ = EAS(SL(204_NO_CONTENT));
    start_line_cur_ = sizeof(EAS(SL(204_NO_CONTENT))) - 1;
    return handler_;
  }
  response_handler<response>& reset_content_205() {
    start_line_ = EAS(SL(205_RESET_CONTENT));
    start_line_cur_ = sizeof(EAS(SL(205_RESET_CONTENT))) - 1;
    return handler_;
  }
  response_handler<response>& partial_content_206() {
    start_line_ = EAS(SL(206_PARTIAL_CONTENT));
    start_line_cur_ = sizeof(EAS(SL(206_PARTIAL_CONTENT))) - 1;
    return handler_;
  }
  response_handler<response>& multiple_choices_300() {
    start_line_ = EAS(SL(300_MULTIPLE_CHOICES));
    start_line_cur_ = sizeof(EAS(SL(300_MULTIPLE_CHOICES))) - 1;
    return handler_;
  }
  response_handler<response>& moved_permanently_301() {
    start_line_ = EAS(SL(301_MOVED_PERMANENTLY));
    start_line_cur_ = sizeof(EAS(SL(301_MOVED_PERMANENTLY))) - 1;
    return handler_;
  }
  response_handler<response>& found_302() {
    start_line_ = EAS(SL(302_FOUND));
    start_line_cur_ = sizeof(EAS(SL(302_FOUND))) - 1;
    return handler_;
  }
  response_handler<response>& see_other_303() {
    start_line_ = EAS(SL(303_SEE_OTHER));
    start_line_cur_ = sizeof(EAS(SL(303_SEE_OTHER))) - 1;
    return handler_;
  }
  response_handler<response>& not_modified_304() {
    start_line_ = EAS(SL(304_NOT_MODIFIED));
    start_line_cur_ = sizeof(EAS(SL(304_NOT_MODIFIED))) - 1;
    return handler_;
  }
  response_handler<response>& use_proxy_305() {
    start_line_ = EAS(SL(305_USE_PROXY));
    start_line_cur_ = sizeof(EAS(SL(305_USE_PROXY))) - 1;
    return handler_;
  }
  response_handler<response>& unused_306() {
    start_line_ = EAS(SL(306_UNUSED));
    start_line_cur_ = sizeof(EAS(SL(306_UNUSED))) - 1;
    return handler_;
  }
  response_handler<response>& temporary_redirect_307() {
    start_line_ = EAS(SL(307_TEMPORARY_REDIRECT));
    start_line_cur_ = sizeof(EAS(SL(307_TEMPORARY_REDIRECT))) - 1;
    return handler_;
  }
  response_handler<response>& permanent_redirect_308() {
    start_line_ = EAS(SL(308_PERMANENT_REDIRECT));
    start_line_cur_ = sizeof(EAS(SL(308_PERMANENT_REDIRECT))) - 1;
    return handler_;
  }
  response_handler<response>& bad_request_400() {
    start_line_ = EAS(SL(400_BAD_REQUEST));
    start_line_cur_ = sizeof(EAS(SL(400_BAD_REQUEST))) - 1;
    return handler_;
  }
  response_handler<response>& unauthorized_401() {
    start_line_ = EAS(SL(401_UNAUTHORIZED));
    start_line_cur_ = sizeof(EAS(SL(401_UNAUTHORIZED))) - 1;
    return handler_;
  }
  response_handler<response>& payment_required_402() {
    start_line_ = EAS(SL(402_PAYMENT_REQUIRED));
    start_line_cur_ = sizeof(EAS(SL(402_PAYMENT_REQUIRED))) - 1;
    return handler_;
  }
  response_handler<response>& forbidden_403() {
    start_line_ = EAS(SL(403_FORBIDDEN));
    start_line_cur_ = sizeof(EAS(SL(403_FORBIDDEN))) - 1;
    return handler_;
  }
  response_handler<response>& not_found_404() {
    start_line_ = EAS(SL(404_NOT_FOUND));
    start_line_cur_ = sizeof(EAS(SL(404_NOT_FOUND))) - 1;
    return handler_;
  }
  response_handler<response>& method_not_allowed_405() {
    start_line_ = EAS(SL(405_METHOD_NOT_ALLOWED));
    start_line_cur_ = sizeof(EAS(SL(405_METHOD_NOT_ALLOWED))) - 1;
    return handler_;
  }
  response_handler<response>& not_acceptable_406() {
    start_line_ = EAS(SL(406_NOT_ACCEPTABLE));
    start_line_cur_ = sizeof(EAS(SL(406_NOT_ACCEPTABLE))) - 1;
    return handler_;
  }
  response_handler<response>& proxy_authentication_required_407() {
    start_line_ = EAS(SL(407_PROXY_AUTHENTICATION_REQUIRED));
    start_line_cur_ = sizeof(EAS(SL(407_PROXY_AUTHENTICATION_REQUIRED))) - 1;
    return handler_;
  }
  response_handler<response>& request_timeout_408() {
    start_line_ = EAS(SL(408_REQUEST_TIMEOUT));
    start_line_cur_ = sizeof(EAS(SL(408_REQUEST_TIMEOUT))) - 1;
    return handler_;
  }
  response_handler<response>& conflict_409() {
    start_line_ = EAS(SL(409_CONFLICT));
    start_line_cur_ = sizeof(EAS(SL(409_CONFLICT))) - 1;
    return handler_;
  }
  response_handler<response>& gone_410() {
    start_line_ = EAS(SL(410_GONE));
    start_line_cur_ = sizeof(EAS(SL(410_GONE))) - 1;
    return handler_;
  }
  response_handler<response>& length_required_411() {
    start_line_ = EAS(SL(411_LENGTH_REQUIRED));
    start_line_cur_ = sizeof(EAS(SL(411_LENGTH_REQUIRED))) - 1;
    return handler_;
  }
  response_handler<response>& precondition_failed_412() {
    start_line_ = EAS(SL(412_PRECONDITION_FAILED));
    start_line_cur_ = sizeof(EAS(SL(412_PRECONDITION_FAILED))) - 1;
    return handler_;
  }
  response_handler<response>& content_too_large_413() {
    start_line_ = EAS(SL(413_CONTENT_TOO_LARGE));
    start_line_cur_ = sizeof(EAS(SL(413_CONTENT_TOO_LARGE))) - 1;
    return handler_;
  }
  response_handler<response>& uri_too_long_414() {
    start_line_ = EAS(SL(414_URI_TOO_LONG));
    start_line_cur_ = sizeof(EAS(SL(414_URI_TOO_LONG))) - 1;
    return handler_;
  }
  response_handler<response>& unsupported_media_type_415() {
    start_line_ = EAS(SL(415_UNSUPPORTED_MEDIA_TYPE));
    start_line_cur_ = sizeof(EAS(SL(415_UNSUPPORTED_MEDIA_TYPE))) - 1;
    return handler_;
  }
  response_handler<response>& range_not_satisfiable_416() {
    start_line_ = EAS(SL(416_RANGE_NOT_SATISFIABLE));
    start_line_cur_ = sizeof(EAS(SL(416_RANGE_NOT_SATISFIABLE))) - 1;
    return handler_;
  }
  response_handler<response>& expectation_failed_417() {
    start_line_ = EAS(SL(417_EXPECTATION_FAILED));
    start_line_cur_ = sizeof(EAS(SL(417_EXPECTATION_FAILED))) - 1;
    return handler_;
  }
  response_handler<response>& unused_418() {
    start_line_ = EAS(SL(418_UNUSED));
    start_line_cur_ = sizeof(EAS(SL(418_UNUSED))) - 1;
    return handler_;
  }
  response_handler<response>& misdirected_request_421() {
    start_line_ = EAS(SL(421_MISDIRECTED_REQUEST));
    start_line_cur_ = sizeof(EAS(SL(421_MISDIRECTED_REQUEST))) - 1;
    return handler_;
  }
  response_handler<response>& unprocessable_content_422() {
    start_line_ = EAS(SL(422_UNPROCESSABLE_CONTENT));
    start_line_cur_ = sizeof(EAS(SL(422_UNPROCESSABLE_CONTENT))) - 1;
    return handler_;
  }
  response_handler<response>& upgrade_required_426() {
    start_line_ = EAS(SL(426_UPGRADE_REQUIRED));
    start_line_cur_ = sizeof(EAS(SL(426_UPGRADE_REQUIRED))) - 1;
    return handler_;
  }
  response_handler<response>& internal_server_error_500() {
    start_line_ = EAS(SL(500_INTERNAL_SERVER_ERROR));
    start_line_cur_ = sizeof(EAS(SL(500_INTERNAL_SERVER_ERROR))) - 1;
    return handler_;
  }
  response_handler<response>& not_implemented_501() {
    start_line_ = EAS(SL(501_NOT_IMPLEMENTED));
    start_line_cur_ = sizeof(EAS(SL(501_NOT_IMPLEMENTED))) - 1;
    return handler_;
  }
  response_handler<response>& bad_gateway_502() {
    start_line_ = EAS(SL(502_BAD_GATEWAY));
    start_line_cur_ = sizeof(EAS(SL(502_BAD_GATEWAY))) - 1;
    return handler_;
  }
  response_handler<response>& service_unavailable_503() {
    start_line_ = EAS(SL(503_SERVICE_UNAVAILABLE));
    start_line_cur_ = sizeof(EAS(SL(503_SERVICE_UNAVAILABLE))) - 1;
    return handler_;
  }
  response_handler<response>& gateway_timeout_504() {
    start_line_ = EAS(SL(504_GATEWAY_TIMEOUT));
    start_line_cur_ = sizeof(EAS(SL(504_GATEWAY_TIMEOUT))) - 1;
    return handler_;
  }
  response_handler<response>& http_version_not_supported_505() {
    start_line_ = EAS(SL(505_HTTP_VERSION_NOT_SUPPORTED));
    start_line_cur_ = sizeof(EAS(SL(505_HTTP_VERSION_NOT_SUPPORTED))) - 1;
    return handler_;
  }

 private:
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  inline std::shared_ptr<std::istream> serialize() {
    std::size_t cur = 0;
    memcpy(&buffer_[cur], start_line_, start_line_cur_);
    cur += start_line_cur_;
    memcpy(&buffer_[cur], handler_.headers(), handler_.headers_length());
    cur += handler_.headers_length();
    buffer_[cur++] = constants::character::kCr;
    buffer_[cur++] = constants::character::kLf;
    if (handler_.body_length()) {
      memcpy(&buffer_[cur], handler_.body(), handler_.body_length());
      cur += handler_.body_length();
    }
    buffer_[cur++] = 0;
    return std::make_shared<std::stringstream>(buffer_);
  }
  inline void reset() {
    start_line_ = nullptr;
    start_line_cur_ = 0;
    handler_.reset();
  }
  // ___________________________________________________________________________
  // FRIEND-CLASSEs                                                  ( private )
  //
  friend typename TRty<request, response>;
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  char* buffer_ = nullptr;
  const char* start_line_ = nullptr;
  std::size_t start_line_cur_ = 0;
  response_handler<response> handler_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
