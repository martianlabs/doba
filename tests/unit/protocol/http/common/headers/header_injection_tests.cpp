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

#include <cstddef>
#include <string>
#include <string_view>

#include "protocol/http/common/headers/accept.h"
#include "protocol/http/common/headers/accept_charset.h"
#include "protocol/http/common/headers/accept_encoding.h"
#include "protocol/http/common/headers/accept_language.h"
#include "protocol/http/common/headers/accept_ranges.h"
#include "protocol/http/common/headers/access_control_allow_headers.h"
#include "protocol/http/common/headers/access_control_allow_methods.h"
#include "protocol/http/common/headers/access_control_allow_origin.h"
#include "protocol/http/common/headers/access_control_expose_headers.h"
#include "protocol/http/common/headers/access_control_request_headers.h"
#include "protocol/http/common/headers/access_control_request_method.h"
#include "protocol/http/common/headers/age.h"
#include "protocol/http/common/headers/allow.h"
#include "protocol/http/common/headers/authorization.h"
#include "protocol/http/common/headers/cache_control.h"
#include "protocol/http/common/headers/content_encoding.h"
#include "protocol/http/common/headers/content_language.h"
#include "protocol/http/common/headers/content_type.h"
#include "protocol/http/common/headers/cookie.h"
#include "protocol/http/common/headers/date.h"
#include "protocol/http/common/headers/etag.h"
#include "protocol/http/common/headers/if_match.h"
#include "protocol/http/common/headers/if_none_match.h"
#include "protocol/http/common/headers/location.h"
#include "protocol/http/common/headers/origin.h"
#include "protocol/http/common/headers/range.h"
#include "protocol/http/common/headers/referer.h"
#include "protocol/http/common/headers/server.h"
#include "protocol/http/common/headers/set_cookie.h"
#include "protocol/http/common/headers/user_agent.h"
#include "protocol/http/common/headers/vary.h"
#include "test_helper.h"

namespace {
namespace h = martianlabs::doba::protocol::http::headers;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] header_case                                                ( struct ) |
// +---------------------------------------------------------------------------+
// | Header checker paired with a value it must accept.                        |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct header_case {
  std::string_view name;
  bool (*check)(std::string_view);
  std::string_view sample;
};
template <typename Hty>
bool check(std::string_view value) {
  return Hty::check(value);
}
constexpr header_case kCases[] = {
    {"Accept", &check<h::accept>, "text/html, application/json;q=0.9"},
    {"Accept-Charset", &check<h::accept_charset>, "utf-8, iso-8859-1;q=0.5"},
    {"Accept-Encoding", &check<h::accept_encoding>, "gzip, br;q=0.8"},
    {"Accept-Language", &check<h::accept_language>, "en-US, es;q=0.7"},
    {"Accept-Ranges", &check<h::accept_ranges>, "bytes"},
    {"Access-Control-Allow-Headers",
     &check<h::access_control_allow_headers>, "X-A, X-B"},
    {"Access-Control-Allow-Methods",
     &check<h::access_control_allow_methods>, "GET, POST"},
    {"Access-Control-Allow-Origin", &check<h::access_control_allow_origin>,
     "https://example.com"},
    {"Access-Control-Expose-Headers",
     &check<h::access_control_expose_headers>, "X-A, X-B"},
    {"Access-Control-Request-Headers",
     &check<h::access_control_request_headers>, "x-a, x-b"},
    {"Access-Control-Request-Method",
     &check<h::access_control_request_method>, "PUT"},
    {"Age", &check<h::age>, "3600"},
    {"Allow", &check<h::allow>, "GET, HEAD"},
    {"Authorization", &check<h::authorization>, "Bearer abc.def"},
    {"Cache-Control", &check<h::cache_control>, "no-cache, max-age=60"},
    {"Content-Encoding", &check<h::content_encoding>, "gzip"},
    {"Content-Language", &check<h::content_language>, "en, es"},
    {"Content-Type", &check<h::content_type>, "text/plain; charset=utf-8"},
    {"Cookie", &check<h::cookie>, "a=b; c=d"},
    {"Date", &check<h::date>, "Sun, 06 Nov 1994 08:49:37 GMT"},
    {"ETag", &check<h::etag>, "\"abc\""},
    {"If-Match", &check<h::if_match>, "\"a\", \"b\""},
    {"If-None-Match", &check<h::if_none_match>, "W/\"a\", \"b\""},
    {"Location", &check<h::location>, "https://example.com/a?b=c"},
    {"Origin", &check<h::origin>, "https://example.com"},
    {"Range", &check<h::range>, "bytes=0-99"},
    {"Referer", &check<h::referer>, "https://example.com/a"},
    {"Server", &check<h::server>, "doba/1.0 (test)"},
    {"Set-Cookie", &check<h::set_cookie>, "a=b; Path=/; Secure"},
    {"User-Agent", &check<h::user_agent>, "doba/1.0 (test)"},
    {"Vary", &check<h::vary>, "Accept, Origin"},
};
}  // namespace

// +===========================================================================+
// | [>] header checkers accept their reference samples          ( test-case ) |
// +===========================================================================+
DOBA_TEST("header checkers accept their reference samples") {
  for (const auto& value : kCases) {
    martianlabs::doba::tests::unit::test_helper::set_context(value.name);
    DOBA_EXPECT(value.check(value.sample));
  }
}
// +===========================================================================+
// | [>] header checkers reject injected control bytes           ( test-case ) |
// +===========================================================================+
DOBA_TEST("header checkers reject injected control bytes") {
  for (const auto& value : kCases) {
    const std::size_t positions[] = {0, value.sample.size() / 2,
                                     value.sample.size()};
    for (const std::size_t position : positions) {
      for (const char control : {'\0', '\r', '\n', '\x7f'}) {
        std::string source(value.sample);
        source.insert(source.begin() + static_cast<std::ptrdiff_t>(position),
                      control);
        martianlabs::doba::tests::unit::test_helper::set_context(
            std::string(value.name) + ", position " +
            std::to_string(position) + ", byte " +
            std::to_string(static_cast<unsigned char>(control)));
        DOBA_EXPECT(!value.check(source));
      }
    }
    std::string split(value.sample);
    split += "\r\nX-Injected: 1";
    martianlabs::doba::tests::unit::test_helper::set_context(value.name);
    DOBA_EXPECT(!value.check(split));
  }
}
