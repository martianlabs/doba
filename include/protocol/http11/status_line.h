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
// Copyright 2025 martianLabs
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied. See the License for the specific language governing
// permissions and limitations under the License.

#ifndef martianlabs_doba_protocol_http11_status_line_h
#define martianlabs_doba_protocol_http11_status_line_h

#include "status_code.h"
#include "reason_phrase.h"

namespace martianlabs::doba::protocol::http11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] common-usage                                               ( macros ) |
// +---------------------------------------------------------------------------+
// | This section holds for common-usage macros.                               |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
#define SP
#define CRLF \r\n
#define STR_VALUE(x) #x
#define CC_RAW(a, b) a##b
#define CC_EXP(a, b) CC_RAW(a, b)
#define EAS(x) STR_VALUE(x)
#define HTTP_HEADER HTTP
#define SLASH /
#define ONE_DOT_ONE 1.1
#define VERSION CC_EXP(HTTP_HEADER, CC_EXP(SLASH, ONE_DOT_ONE))
#define SL(x) VERSION CC_RAW(SC_, x) CC_EXP(CC_RAW(RP_, x), CRLF)
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] status_line                                                ( struct ) |
// +---------------------------------------------------------------------------+
// | This class holds for the http 1.1 status lines.                           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct status_line {
  // +=========================================================================+
  // | [>] LITERALs                                                 ( public ) |
  // +=========================================================================+
  static constexpr char k100[] = EAS(SL(100_CONTINUE));
  static constexpr char k101[] = EAS(SL(101_SWITCHING_PROTOCOLS));
  static constexpr char k200[] = EAS(SL(200_OK));
  static constexpr char k201[] = EAS(SL(201_CREATED));
  static constexpr char k202[] = EAS(SL(202_ACCEPTED));
  static constexpr char k203[] = EAS(SL(203_NON_AUTHORITATIVE_INFORMATION));
  static constexpr char k204[] = EAS(SL(204_NO_CONTENT));
  static constexpr char k205[] = EAS(SL(205_RESET_CONTENT));
  static constexpr char k206[] = EAS(SL(206_PARTIAL_CONTENT));
  static constexpr char k300[] = EAS(SL(300_MULTIPLE_CHOICES));
  static constexpr char k301[] = EAS(SL(301_MOVED_PERMANENTLY));
  static constexpr char k302[] = EAS(SL(302_FOUND));
  static constexpr char k303[] = EAS(SL(303_SEE_OTHER));
  static constexpr char k304[] = EAS(SL(304_NOT_MODIFIED));
  static constexpr char k305[] = EAS(SL(305_USE_PROXY));
  static constexpr char k306[] = EAS(SL(306_UNUSED));
  static constexpr char k307[] = EAS(SL(307_TEMPORARY_REDIRECT));
  static constexpr char k308[] = EAS(SL(308_PERMANENT_REDIRECT));
  static constexpr char k400[] = EAS(SL(400_BAD_REQUEST));
  static constexpr char k401[] = EAS(SL(401_UNAUTHORIZED));
  static constexpr char k402[] = EAS(SL(402_PAYMENT_REQUIRED));
  static constexpr char k403[] = EAS(SL(403_FORBIDDEN));
  static constexpr char k404[] = EAS(SL(404_NOT_FOUND));
  static constexpr char k405[] = EAS(SL(405_METHOD_NOT_ALLOWED));
  static constexpr char k406[] = EAS(SL(406_NOT_ACCEPTABLE));
  static constexpr char k407[] = EAS(SL(407_PROXY_AUTHENTICATION_REQUIRED));
  static constexpr char k408[] = EAS(SL(408_REQUEST_TIMEOUT));
  static constexpr char k409[] = EAS(SL(409_CONFLICT));
  static constexpr char k410[] = EAS(SL(410_GONE));
  static constexpr char k411[] = EAS(SL(411_LENGTH_REQUIRED));
  static constexpr char k412[] = EAS(SL(412_PRECONDITION_FAILED));
  static constexpr char k413[] = EAS(SL(413_CONTENT_TOO_LARGE));
  static constexpr char k414[] = EAS(SL(414_URI_TOO_LONG));
  static constexpr char k415[] = EAS(SL(415_UNSUPPORTED_MEDIA_TYPE));
  static constexpr char k416[] = EAS(SL(416_RANGE_NOT_SATISFIABLE));
  static constexpr char k417[] = EAS(SL(417_EXPECTATION_FAILED));
  static constexpr char k418[] = EAS(SL(418_UNUSED));
  static constexpr char k421[] = EAS(SL(421_MISDIRECTED_REQUEST));
  static constexpr char k422[] = EAS(SL(422_UNPROCESSABLE_ENTITY));
  static constexpr char k426[] = EAS(SL(426_UPGRADE_REQUIRED));
  static constexpr char k500[] = EAS(SL(500_INTERNAL_SERVER_ERROR));
  static constexpr char k501[] = EAS(SL(501_NOT_IMPLEMENTED));
  static constexpr char k502[] = EAS(SL(502_BAD_GATEWAY));
  static constexpr char k503[] = EAS(SL(503_SERVICE_UNAVAILABLE));
  static constexpr char k504[] = EAS(SL(504_GATEWAY_TIMEOUT));
  static constexpr char k505[] = EAS(SL(505_HTTP_VERSION_NOT_SUPPORTED));
  // +=========================================================================+
  // | [>] SIZEs                                                    ( public ) |
  // +=========================================================================+
  static constexpr std::size_t k100Sz = sizeof(k100) - 1;
  static constexpr std::size_t k101Sz = sizeof(k101) - 1;
  static constexpr std::size_t k200Sz = sizeof(k200) - 1;
  static constexpr std::size_t k201Sz = sizeof(k201) - 1;
  static constexpr std::size_t k202Sz = sizeof(k202) - 1;
  static constexpr std::size_t k203Sz = sizeof(k203) - 1;
  static constexpr std::size_t k204Sz = sizeof(k204) - 1;
  static constexpr std::size_t k205Sz = sizeof(k205) - 1;
  static constexpr std::size_t k206Sz = sizeof(k206) - 1;
  static constexpr std::size_t k300Sz = sizeof(k300) - 1;
  static constexpr std::size_t k301Sz = sizeof(k301) - 1;
  static constexpr std::size_t k302Sz = sizeof(k302) - 1;
  static constexpr std::size_t k303Sz = sizeof(k303) - 1;
  static constexpr std::size_t k304Sz = sizeof(k304) - 1;
  static constexpr std::size_t k305Sz = sizeof(k305) - 1;
  static constexpr std::size_t k306Sz = sizeof(k306) - 1;
  static constexpr std::size_t k307Sz = sizeof(k307) - 1;
  static constexpr std::size_t k308Sz = sizeof(k308) - 1;
  static constexpr std::size_t k400Sz = sizeof(k400) - 1;
  static constexpr std::size_t k401Sz = sizeof(k401) - 1;
  static constexpr std::size_t k402Sz = sizeof(k402) - 1;
  static constexpr std::size_t k403Sz = sizeof(k403) - 1;
  static constexpr std::size_t k404Sz = sizeof(k404) - 1;
  static constexpr std::size_t k405Sz = sizeof(k405) - 1;
  static constexpr std::size_t k406Sz = sizeof(k406) - 1;
  static constexpr std::size_t k407Sz = sizeof(k407) - 1;
  static constexpr std::size_t k408Sz = sizeof(k408) - 1;
  static constexpr std::size_t k409Sz = sizeof(k409) - 1;
  static constexpr std::size_t k410Sz = sizeof(k410) - 1;
  static constexpr std::size_t k411Sz = sizeof(k411) - 1;
  static constexpr std::size_t k412Sz = sizeof(k412) - 1;
  static constexpr std::size_t k413Sz = sizeof(k413) - 1;
  static constexpr std::size_t k414Sz = sizeof(k414) - 1;
  static constexpr std::size_t k415Sz = sizeof(k415) - 1;
  static constexpr std::size_t k416Sz = sizeof(k416) - 1;
  static constexpr std::size_t k417Sz = sizeof(k417) - 1;
  static constexpr std::size_t k418Sz = sizeof(k418) - 1;
  static constexpr std::size_t k421Sz = sizeof(k421) - 1;
  static constexpr std::size_t k422Sz = sizeof(k422) - 1;
  static constexpr std::size_t k426Sz = sizeof(k426) - 1;
  static constexpr std::size_t k500Sz = sizeof(k500) - 1;
  static constexpr std::size_t k501Sz = sizeof(k501) - 1;
  static constexpr std::size_t k502Sz = sizeof(k502) - 1;
  static constexpr std::size_t k503Sz = sizeof(k503) - 1;
  static constexpr std::size_t k504Sz = sizeof(k504) - 1;
  static constexpr std::size_t k505Sz = sizeof(k505) - 1;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
