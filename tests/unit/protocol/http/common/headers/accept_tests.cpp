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

#include <string>
#include <string_view>

#include "protocol/http/common/headers/accept.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::headers::accept;
}  // namespace

// +===========================================================================+
// | [>] check accepts media ranges                              ( test-case ) |
// +===========================================================================+
DOBA_TEST("check accepts media ranges") {
  constexpr std::string_view cases[] = {
      "*/*",
      "text/*",
      "text/plain",
      "TEXT/PLAIN",
      "application/vnd.example+json",
      "x!#$%&'*+-.^_`|~/y!#$%&'*+-.^_`|~",
  };
  for (const auto source : cases) {
    DOBA_EXPECT(accept::check(source));
    const std::string padded = "x" + std::string(source) + "y";
    DOBA_EXPECT(
        accept::check(std::string_view(padded).substr(1, source.size())));
  }
}
// +===========================================================================+
// | [>] check accepts parameters                                ( test-case ) |
// +===========================================================================+
DOBA_TEST("check accepts parameters") {
  constexpr std::string_view cases[] = {
      "text/html;charset=utf-8",
      "text/html ; charset=utf-8",
      "text/html\t;\tcharset=\"utf-8\"",
      "text/plain;format=\"flowed text\"",
      "text/plain;note=\"a,b;c\"",
      "text/plain;note=\"a\\\"b\\\\c\"",
      "text/plain;empty=\"\"",
      "text/plain;",
      "text/plain;;",
      "text/plain; ; charset=utf-8",
      "text/plain;note=\"a,b\",application/json",
  };
  for (const auto source : cases) {
    DOBA_EXPECT(accept::check(source));
  }
  std::string obs_text = "text/plain;note=\"";
  obs_text += static_cast<char>(0x80);
  obs_text += '"';
  DOBA_EXPECT(accept::check(obs_text));
}
// +===========================================================================+
// | [>] check accepts quality values                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("check accepts quality values") {
  constexpr std::string_view cases[] = {
      "text/plain;q=0",
      "text/plain;q=0.",
      "text/plain;q=0.000",
      "text/plain;q=0.001",
      "text/plain;q=0.999",
      "text/plain;q=1",
      "text/plain;q=1.",
      "text/plain;q=1.0",
      "text/plain;q=1.000",
      "text/plain;Q=0.5",
      "text/plain;charset=utf-8;q=0.5",
      "text/plain;q=0.5;charset=utf-8",
  };
  for (const auto source : cases) {
    DOBA_EXPECT(accept::check(source));
  }
}
// +===========================================================================+
// | [>] check accepts lists                                     ( test-case ) |
// +===========================================================================+
DOBA_TEST("check accepts lists") {
  constexpr std::string_view cases[] = {
      "",
      ",",
      ", ,",
      "text/plain,",
      ",text/plain",
      "text/plain,,application/json",
      "text/plain , \t, application/json",
      "text/plain;q=0.5, text/html, */*;q=0.1",
      "text/plain;note=\"a,b\", application/json",
  };
  for (const auto source : cases) {
    DOBA_EXPECT(accept::check(source));
  }
  DOBA_EXPECT(accept::check(std::string_view{}));
}
// +===========================================================================+
// | [>] check rejects invalid media ranges                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("check rejects invalid media ranges") {
  constexpr std::string_view cases[] = {
      " ",
      "*",
      "*/plain",
      "/plain",
      "text/",
      "text//plain",
      "text/plain/extra",
      "text/(plain)",
      "te xt/plain",
      "text/pla in",
      " text/plain",
      "text/plain ",
      "text/plain\\",
  };
  for (const auto source : cases) {
    DOBA_EXPECT(!accept::check(source));
  }
}
// +===========================================================================+
// | [>] check rejects invalid parameters                        ( test-case ) |
// +===========================================================================+
DOBA_TEST("check rejects invalid parameters") {
  constexpr std::string_view cases[] = {
      "text/plain charset=utf-8",
      "text/plain;=utf-8",
      "text/plain;charset",
      "text/plain;charset=",
      "text/plain;charset =utf-8",
      "text/plain;charset= utf-8",
      "text/plain;charset=utf 8",
      "text/plain;charset=utf-8/extra",
      "text/plain;charset=\"unterminated",
      "text/plain;charset=\"dangling\\",
      "text/plain,\"application/json\"",
      "text/plain,application/json\"",
      "text/plain,application/json\\",
  };
  for (const auto source : cases) {
    DOBA_EXPECT(!accept::check(source));
  }
  std::string control = "text/plain;note=\"a";
  control += static_cast<char>(0x01);
  control += "b\"";
  DOBA_EXPECT(!accept::check(control));
  std::string escaped_nul = "text/plain;note=\"a\\";
  escaped_nul += '\0';
  escaped_nul += "b\"";
  DOBA_EXPECT(!accept::check(escaped_nul));
}
// +===========================================================================+
// | [>] check rejects invalid quality values                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("check rejects invalid quality values") {
  constexpr std::string_view cases[] = {
      "text/plain;q=",          "text/plain;q=.5",
      "text/plain;q=00",        "text/plain;q=01",
      "text/plain;q=2",         "text/plain;q=-0.1",
      "text/plain;q=+0.5",      "text/plain;q=0.0000",
      "text/plain;q=1.0000",    "text/plain;q=1.001",
      "text/plain;q=1.1",       "text/plain;q=\"0.5\"",
      "text/plain;q =0.5",      "text/plain;q= 0.5",
      "text/plain;q=0.5x",      "text/plain;q=0.5;q=0.4",
      "text/plain;q=0.5;Q=0.4",
  };
  for (const auto source : cases) {
    DOBA_EXPECT(!accept::check(source));
  }
}
// +===========================================================================+
// | [>] check handles string view boundaries                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("check handles string view boundaries") {
  std::string long_type(4096, 'a');
  long_type += "/plain";
  DOBA_EXPECT(accept::check(long_type));
  DOBA_EXPECT(!accept::check(std::string_view{"\0", 1}));
  DOBA_EXPECT(!accept::check(std::string_view{"text/plain\0", 11}));
  DOBA_EXPECT(!accept::check(std::string_view{"text/plain\r", 11}));
  DOBA_EXPECT(!accept::check(std::string_view{"text/plain\n", 11}));
  DOBA_EXPECT(!accept::check(std::string_view{"\x80", 1}));
}
