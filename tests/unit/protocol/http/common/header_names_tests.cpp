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

#include "protocol/http/common/header_names.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::header_names;
}  // namespace

// +===========================================================================+
// | [>] constants contain canonical field names                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("constants contain canonical field names") {
  constexpr std::string_view actual[] = {
      header_names::kConnection,
      header_names::kDate,
      header_names::kVia,
      header_names::kCacheControl,
      header_names::kPragma,
      header_names::kWarning,
      header_names::kHost,
      header_names::kUserAgent,
      header_names::kAccept,
      header_names::kAcceptEncoding,
      header_names::kAcceptLanguage,
      header_names::kAuthorization,
      header_names::kExpect,
      header_names::kFrom,
      header_names::kIfMatch,
      header_names::kIfNoneMatch,
      header_names::kIfModifiedSince,
      header_names::kIfUnmodifiedSince,
      header_names::kRange,
      header_names::kReferer,
      header_names::kTe,
      header_names::kUpgrade,
      header_names::kCookie,
      header_names::kLocation,
      header_names::kServer,
      header_names::kVary,
      header_names::kWwwAuthenticate,
      header_names::kContentLength,
      header_names::kContentType,
      header_names::kContentEncoding,
      header_names::kContentLanguage,
      header_names::kContentRange,
      header_names::kTrailer,
      header_names::kTransferEncoding,
      header_names::kAllow,
      header_names::kRetryAfter,
      header_names::kAcceptRanges,
      header_names::kETag,
      header_names::kLastModified,
  };
  constexpr std::string_view expected[] = {
      "Connection",
      "Date",
      "Via",
      "Cache-Control",
      "Pragma",
      "Warning",
      "Host",
      "User-agent",
      "Accept",
      "Accept-Encoding",
      "Accept-Language",
      "Authorization",
      "Expect",
      "From",
      "If-Match",
      "If-None-Match",
      "If-Modified-Since",
      "If-Unmodified-Since",
      "Range",
      "Referer",
      "TE",
      "Upgrade",
      "Cookie",
      "Location",
      "Server",
      "Vary",
      "WWW-Authenticate",
      "Content-Length",
      "Content-Type",
      "Content-Encoding",
      "Content-Language",
      "Content-Range",
      "Trailer",
      "Transfer-Encoding",
      "Allow",
      "Retry-After",
      "Accept-Ranges",
      "ETAG",
      "Last-Modified",
  };
  static_assert(std::size(actual) == std::size(expected));
  for (std::size_t i = 0; i < std::size(actual); i++) {
    DOBA_EXPECT_EQUAL(actual[i], expected[i]);
  }
}
