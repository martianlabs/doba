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

#include <string_view>

#include "protocol/http/v11/reason_phrases.h"
#include "test_helper.h"

#define DOBA_STRINGIZE_RAW(value) #value
#define DOBA_STRINGIZE(value) DOBA_STRINGIZE_RAW(value)

// +===========================================================================+
// | [>] macros expose the documented reason phrases             ( test-case ) |
// +===========================================================================+
DOBA_TEST("macros expose the documented reason phrases") {
  constexpr std::string_view actual[] = {
      DOBA_STRINGIZE(RP_100_CONTINUE),
      DOBA_STRINGIZE(RP_101_SWITCHING_PROTOCOLS),
      DOBA_STRINGIZE(RP_200_OK),
      DOBA_STRINGIZE(RP_201_CREATED),
      DOBA_STRINGIZE(RP_202_ACCEPTED),
      DOBA_STRINGIZE(RP_203_NON_AUTHORITATIVE_INFORMATION),
      DOBA_STRINGIZE(RP_204_NO_CONTENT),
      DOBA_STRINGIZE(RP_205_RESET_CONTENT),
      DOBA_STRINGIZE(RP_206_PARTIAL_CONTENT),
      DOBA_STRINGIZE(RP_300_MULTIPLE_CHOICES),
      DOBA_STRINGIZE(RP_301_MOVED_PERMANENTLY),
      DOBA_STRINGIZE(RP_302_FOUND),
      DOBA_STRINGIZE(RP_303_SEE_OTHER),
      DOBA_STRINGIZE(RP_304_NOT_MODIFIED),
      DOBA_STRINGIZE(RP_305_USE_PROXY),
      DOBA_STRINGIZE(RP_306_UNUSED),
      DOBA_STRINGIZE(RP_307_TEMPORARY_REDIRECT),
      DOBA_STRINGIZE(RP_308_PERMANENT_REDIRECT),
      DOBA_STRINGIZE(RP_400_BAD_REQUEST),
      DOBA_STRINGIZE(RP_401_UNAUTHORIZED),
      DOBA_STRINGIZE(RP_402_PAYMENT_REQUIRED),
      DOBA_STRINGIZE(RP_403_FORBIDDEN),
      DOBA_STRINGIZE(RP_404_NOT_FOUND),
      DOBA_STRINGIZE(RP_405_METHOD_NOT_ALLOWED),
      DOBA_STRINGIZE(RP_406_NOT_ACCEPTABLE),
      DOBA_STRINGIZE(RP_407_PROXY_AUTHENTICATION_REQUIRED),
      DOBA_STRINGIZE(RP_408_REQUEST_TIMEOUT),
      DOBA_STRINGIZE(RP_409_CONFLICT),
      DOBA_STRINGIZE(RP_410_GONE),
      DOBA_STRINGIZE(RP_411_LENGTH_REQUIRED),
      DOBA_STRINGIZE(RP_412_PRECONDITION_FAILED),
      DOBA_STRINGIZE(RP_413_CONTENT_TOO_LARGE),
      DOBA_STRINGIZE(RP_414_URI_TOO_LONG),
      DOBA_STRINGIZE(RP_415_UNSUPPORTED_MEDIA_TYPE),
      DOBA_STRINGIZE(RP_416_RANGE_NOT_SATISFIABLE),
      DOBA_STRINGIZE(RP_417_EXPECTATION_FAILED),
      DOBA_STRINGIZE(RP_418_IM_A_TEAPOT),
      DOBA_STRINGIZE(RP_421_MISDIRECTED_REQUEST),
      DOBA_STRINGIZE(RP_422_UNPROCESSABLE_CONTENT),
      DOBA_STRINGIZE(RP_426_UPGRADE_REQUIRED),
      DOBA_STRINGIZE(RP_431_REQUEST_HEADER_FIELDS_TOO_LARGE),
      DOBA_STRINGIZE(RP_500_INTERNAL_SERVER_ERROR),
      DOBA_STRINGIZE(RP_501_NOT_IMPLEMENTED),
      DOBA_STRINGIZE(RP_502_BAD_GATEWAY),
      DOBA_STRINGIZE(RP_503_SERVICE_UNAVAILABLE),
      DOBA_STRINGIZE(RP_504_GATEWAY_TIMEOUT),
      DOBA_STRINGIZE(RP_505_HTTP_VERSION_NOT_SUPPORTED),
  };
  constexpr std::string_view expected[] = {
      "Continue",
      "Switching Protocols",
      "OK",
      "Created",
      "Accepted",
      "Non-Authoritative Information",
      "No Content",
      "Reset Content",
      "Partial Content",
      "Multiple Choices",
      "Moved Permanently",
      "Found",
      "See Other",
      "Not Modified",
      "Use Proxy",
      "Unused",
      "Temporary Redirect",
      "Permanent Redirect",
      "Bad Request",
      "Unauthorized",
      "Payment Required",
      "Forbidden",
      "Not Found",
      "Method Not Allowed",
      "Not Acceptable",
      "Proxy Authentication Required",
      "Request Timeout",
      "Conflict",
      "Gone",
      "Length Required",
      "Precondition Failed",
      "Content Too Large",
      "URI Too Long",
      "Unsupported Media Type",
      "Range Not Satisfiable",
      "Expectation Failed",
      "Im a teapot",
      "Misdirected Request",
      "Unprocessable Content",
      "Upgrade Required",
      "Request Header Fields Too Large",
      "Internal Server Error",
      "Not Implemented",
      "Bad Gateway",
      "Service Unavailable",
      "Gateway Timeout",
      "HTTP Version Not Supported",
  };
  static_assert(std::size(actual) == std::size(expected));
  for (std::size_t i = 0; i < std::size(actual); i++) {
    DOBA_EXPECT_EQUAL(actual[i], expected[i]);
  }
}

#undef DOBA_STRINGIZE
#undef DOBA_STRINGIZE_RAW
