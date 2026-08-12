//                              _       _
//                           __| | ___ | |__   __ _
//                          / _` |/ _ \| '_ \ / _` |
//                         | (_| | (_) | |_) | (_| |
//                          \__,_|\___/|_.__/ \__,_|
//
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/LICENSE-2.0
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

#include "protocol/http/common/status_codes.h"
#include "test_helper.h"

// +===========================================================================+
// | [>] macros expose the registered status codes               ( test-case ) |
// +===========================================================================+
DOBA_TEST("macros expose the registered status codes") {
  constexpr int actual[] = {
      SC_100_CONTINUE,
      SC_101_SWITCHING_PROTOCOLS,
      SC_200_OK,
      SC_201_CREATED,
      SC_202_ACCEPTED,
      SC_203_NON_AUTHORITATIVE_INFORMATION,
      SC_204_NO_CONTENT,
      SC_205_RESET_CONTENT,
      SC_206_PARTIAL_CONTENT,
      SC_300_MULTIPLE_CHOICES,
      SC_301_MOVED_PERMANENTLY,
      SC_302_FOUND,
      SC_303_SEE_OTHER,
      SC_304_NOT_MODIFIED,
      SC_305_USE_PROXY,
      SC_306_UNUSED,
      SC_307_TEMPORARY_REDIRECT,
      SC_308_PERMANENT_REDIRECT,
      SC_400_BAD_REQUEST,
      SC_401_UNAUTHORIZED,
      SC_402_PAYMENT_REQUIRED,
      SC_403_FORBIDDEN,
      SC_404_NOT_FOUND,
      SC_405_METHOD_NOT_ALLOWED,
      SC_406_NOT_ACCEPTABLE,
      SC_407_PROXY_AUTHENTICATION_REQUIRED,
      SC_408_REQUEST_TIMEOUT,
      SC_409_CONFLICT,
      SC_410_GONE,
      SC_411_LENGTH_REQUIRED,
      SC_412_PRECONDITION_FAILED,
      SC_413_CONTENT_TOO_LARGE,
      SC_414_URI_TOO_LONG,
      SC_415_UNSUPPORTED_MEDIA_TYPE,
      SC_416_RANGE_NOT_SATISFIABLE,
      SC_417_EXPECTATION_FAILED,
      SC_418_IM_A_TEAPOT,
      SC_421_MISDIRECTED_REQUEST,
      SC_422_UNPROCESSABLE_CONTENT,
      SC_426_UPGRADE_REQUIRED,
      SC_431_REQUEST_HEADER_FIELDS_TOO_LARGE,
      SC_500_INTERNAL_SERVER_ERROR,
      SC_501_NOT_IMPLEMENTED,
      SC_502_BAD_GATEWAY,
      SC_503_SERVICE_UNAVAILABLE,
      SC_504_GATEWAY_TIMEOUT,
      SC_505_HTTP_VERSION_NOT_SUPPORTED,
  };
  constexpr int expected[] = {
      100, 101, 200, 201, 202, 203, 204, 205, 206, 300, 301, 302,
      303, 304, 305, 306, 307, 308, 400, 401, 402, 403, 404, 405,
      406, 407, 408, 409, 410, 411, 412, 413, 414, 415, 416, 417,
      418, 421, 422, 426, 431, 500, 501, 502, 503, 504, 505,
  };
  static_assert(std::size(actual) == std::size(expected));
  for (std::size_t i = 0; i < std::size(actual); i++) {
    DOBA_EXPECT_EQUAL(actual[i], expected[i]);
  }
}
