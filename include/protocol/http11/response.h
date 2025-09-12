﻿//      _       _
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
#include "response_handler.h"
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
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  response() {
    buf_ = NULL;
    cursor_ = 0;
    size_ = constants::limits::kDefaultCoreMsgMaxSizeInRam;
    buf_ = (char*)malloc(size_);
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
  std::shared_ptr<common::virtual_buffer> serialize() {
    static const auto eol = (const char*)constants::string::kCrLf;
    static const auto eol_len = sizeof(constants::string::kCrLf) - 1;
    if (cursor_ - size_ < eol_len) return nullptr;
    memcpy(&buf_[cursor_], eol, eol_len);
    cursor_ += eol_len;
    if (body_) {
      if (body_->get_ios_type() == body::ios_type::kReader) {
        if (body_->get_buf_type() == body::buf_type::kMemory) {
          auto ptr = body_->memory_data();
          auto len = body_->memory_size();
          if (size_ - cursor_ >= len) {
            memcpy(&buf_[cursor_], ptr, len);
            cursor_ += len;
          } else {
            // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
            // [to-do] -> add support for this!
            // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
          }
        } else {
          // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
          // [to-do] -> add support for this!
          // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
        }
      } else {
        return nullptr;
      }
    }
    return std::make_shared<common::virtual_buffer>(buf_, cursor_);
  }
  response_handler& continue_100() {
    return setup(EAS(SL(100_CONTINUE)), sizeof(EAS(SL(100_CONTINUE))) - 1);
  }
  response_handler& switching_protocols_101() {
    return setup(EAS(SL(101_SWITCHING_PROTOCOLS)),
                 sizeof(EAS(SL(101_SWITCHING_PROTOCOLS))) - 1);
  }
  response_handler& ok_200() {
    return setup(EAS(SL(200_OK)), sizeof(EAS(SL(200_OK))) - 1);
  }
  response_handler& created_201() {
    return setup(EAS(SL(201_CREATED)), sizeof(EAS(SL(201_CREATED))) - 1);
  }
  response_handler& accepted_202() {
    return setup(EAS(SL(202_ACCEPTED)), sizeof(EAS(SL(202_ACCEPTED))) - 1);
  }
  response_handler& non_authoritative_information_203() {
    return setup(EAS(SL(203_NON_AUTHORITATIVE_INFORMATION)),
                 sizeof(EAS(SL(203_NON_AUTHORITATIVE_INFORMATION))) - 1);
  }
  response_handler& no_content_204() {
    return setup(EAS(SL(204_NO_CONTENT)), sizeof(EAS(SL(204_NO_CONTENT))) - 1);
  }
  response_handler& reset_content_205() {
    return setup(EAS(SL(205_RESET_CONTENT)),
                 sizeof(EAS(SL(205_RESET_CONTENT))) - 1);
  }
  response_handler& partial_content_206() {
    return setup(EAS(SL(206_PARTIAL_CONTENT)),
                 sizeof(EAS(SL(206_PARTIAL_CONTENT))) - 1);
  }
  response_handler& multiple_choices_300() {
    return setup(EAS(SL(300_MULTIPLE_CHOICES)),
                 sizeof(EAS(SL(300_MULTIPLE_CHOICES))) - 1);
  }
  response_handler& moved_permanently_301() {
    return setup(EAS(SL(301_MOVED_PERMANENTLY)),
                 sizeof(EAS(SL(301_MOVED_PERMANENTLY))) - 1);
  }
  response_handler& found_302() {
    return setup(EAS(SL(302_FOUND)), sizeof(EAS(SL(302_FOUND))) - 1);
  }
  response_handler& see_other_303() {
    return setup(EAS(SL(303_SEE_OTHER)), sizeof(EAS(SL(303_SEE_OTHER))) - 1);
  }
  response_handler& not_modified_304() {
    return setup(EAS(SL(304_NOT_MODIFIED)),
                 sizeof(EAS(SL(304_NOT_MODIFIED))) - 1);
  }
  response_handler& use_proxy_305() {
    return setup(EAS(SL(305_USE_PROXY)), sizeof(EAS(SL(305_USE_PROXY))) - 1);
  }
  response_handler& unused_306() {
    return setup(EAS(SL(306_UNUSED)), sizeof(EAS(SL(306_UNUSED))) - 1);
  }
  response_handler& temporary_redirect_307() {
    return setup(EAS(SL(307_TEMPORARY_REDIRECT)),
                 sizeof(EAS(SL(307_TEMPORARY_REDIRECT))) - 1);
  }
  response_handler& permanent_redirect_308() {
    return setup(EAS(SL(308_PERMANENT_REDIRECT)),
                 sizeof(EAS(SL(308_PERMANENT_REDIRECT))) - 1);
  }
  response_handler& bad_request_400() {
    return setup(EAS(SL(400_BAD_REQUEST)),
                 sizeof(EAS(SL(400_BAD_REQUEST))) - 1);
  }
  response_handler& unauthorized_401() {
    return setup(EAS(SL(401_UNAUTHORIZED)),
                 sizeof(EAS(SL(401_UNAUTHORIZED))) - 1);
  }
  response_handler& payment_required_402() {
    return setup(EAS(SL(402_PAYMENT_REQUIRED)),
                 sizeof(EAS(SL(402_PAYMENT_REQUIRED))) - 1);
  }
  response_handler& forbidden_403() {
    return setup(EAS(SL(403_FORBIDDEN)), sizeof(EAS(SL(403_FORBIDDEN))) - 1);
  }
  response_handler& not_found_404() {
    return setup(EAS(SL(404_NOT_FOUND)), sizeof(EAS(SL(404_NOT_FOUND))) - 1);
  }
  response_handler& method_not_allowed_405() {
    return setup(EAS(SL(405_METHOD_NOT_ALLOWED)),
                 sizeof(EAS(SL(405_METHOD_NOT_ALLOWED))) - 1);
  }
  response_handler& not_acceptable_406() {
    return setup(EAS(SL(406_NOT_ACCEPTABLE)),
                 sizeof(EAS(SL(406_NOT_ACCEPTABLE))) - 1);
  }
  response_handler& proxy_authentication_required_407() {
    return setup(EAS(SL(407_PROXY_AUTHENTICATION_REQUIRED)),
                 sizeof(EAS(SL(407_PROXY_AUTHENTICATION_REQUIRED))) - 1);
  }
  response_handler& request_timeout_408() {
    return setup(EAS(SL(408_REQUEST_TIMEOUT)),
                 sizeof(EAS(SL(408_REQUEST_TIMEOUT))) - 1);
  }
  response_handler& conflict_409() {
    return setup(EAS(SL(409_CONFLICT)), sizeof(EAS(SL(409_CONFLICT))) - 1);
  }
  response_handler& gone_410() {
    return setup(EAS(SL(410_GONE)), sizeof(EAS(SL(410_GONE))) - 1);
  }
  response_handler& length_required_411() {
    return setup(EAS(SL(411_LENGTH_REQUIRED)),
                 sizeof(EAS(SL(411_LENGTH_REQUIRED))) - 1);
  }
  response_handler& precondition_failed_412() {
    return setup(EAS(SL(412_PRECONDITION_FAILED)),
                 sizeof(EAS(SL(412_PRECONDITION_FAILED))) - 1);
  }
  response_handler& content_too_large_413() {
    return setup(EAS(SL(413_CONTENT_TOO_LARGE)),
                 sizeof(EAS(SL(413_CONTENT_TOO_LARGE))) - 1);
  }
  response_handler& uri_too_long_414() {
    return setup(EAS(SL(414_URI_TOO_LONG)),
                 sizeof(EAS(SL(414_UNSUPPORTED_MEDIA_TYPE))) - 1);
  }
  response_handler& unsupported_media_type_415() {
    return setup(EAS(SL(415_UNSUPPORTED_MEDIA_TYPE)),
                 sizeof(EAS(SL(415_UNSUPPORTED_MEDIA_TYPE))) - 1);
  }
  response_handler& range_not_satisfiable_416() {
    return setup(EAS(SL(416_RANGE_NOT_SATISFIABLE)),
                 sizeof(EAS(SL(416_RANGE_NOT_SATISFIABLE))) - 1);
  }
  response_handler& expectation_failed_417() {
    return setup(EAS(SL(417_EXPECTATION_FAILED)),
                 sizeof(EAS(SL(417_EXPECTATION_FAILED))) - 1);
  }
  response_handler& unused_418() {
    return setup(EAS(SL(418_UNUSED)), sizeof(EAS(SL(418_UNUSED))) - 1);
  }
  response_handler& misdirected_request_421() {
    return setup(EAS(SL(421_MISDIRECTED_REQUEST)),
                 sizeof(EAS(SL(421_MISDIRECTED_REQUEST))) - 1);
  }
  response_handler& unprocessable_content_422() {
    return setup(EAS(SL(422_UNPROCESSABLE_CONTENT)),
                 sizeof(EAS(SL(422_UNPROCESSABLE_CONTENT))) - 1);
  }
  response_handler& upgrade_required_426() {
    return setup(EAS(SL(426_UPGRADE_REQUIRED)),
                 sizeof(EAS(SL(426_UPGRADE_REQUIRED))) - 1);
  }
  response_handler& internal_server_error_500() {
    return setup(EAS(SL(500_INTERNAL_SERVER_ERROR)),
                 sizeof(EAS(SL(500_INTERNAL_SERVER_ERROR))) - 1);
  }
  response_handler& not_implemented_501() {
    return setup(EAS(SL(501_NOT_IMPLEMENTED)),
                 sizeof(EAS(SL(501_NOT_IMPLEMENTED))) - 1);
  }
  response_handler& bad_gateway_502() {
    return setup(EAS(SL(502_BAD_GATEWAY)),
                 sizeof(EAS(SL(502_BAD_GATEWAY))) - 1);
  }
  response_handler& service_unavailable_503() {
    return setup(EAS(SL(503_SERVICE_UNAVAILABLE)),
                 sizeof(EAS(SL(503_SERVICE_UNAVAILABLE))) - 1);
  }
  response_handler& gateway_timeout_504() {
    return setup(EAS(SL(504_GATEWAY_TIMEOUT)),
                 sizeof(EAS(SL(504_GATEWAY_TIMEOUT))) - 1);
  }
  response_handler& http_version_not_supported_505() {
    return setup(EAS(SL(505_HTTP_VERSION_NOT_SUPPORTED)),
                 sizeof(EAS(SL(505_HTTP_VERSION_NOT_SUPPORTED))) - 1);
  }
  response_handler& setup(const char* const sln, std::size_t sln_len) {
    cursor_ = sln_len;
    memcpy(buf_, sln, cursor_);
    handler_ = std::make_shared<response_handler>(buf_, size_, cursor_, body_);
    return *handler_;
  }

 private:
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  char* buf_;
  std::size_t size_;
  std::size_t cursor_;
  std::shared_ptr<body> body_;
  std::shared_ptr<response_handler> handler_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
