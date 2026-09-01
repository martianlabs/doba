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

#include "protocol/http/v11/headers/x_forwarded_host.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::helpers;
using martianlabs::doba::protocol::http::v11::connection;
using martianlabs::doba::protocol::http::v11::parsed_host_port;
using martianlabs::doba::protocol::http::v11::policies;
using martianlabs::doba::protocol::http::v11::verdict;
using martianlabs::doba::protocol::http::v11::headers::x_forwarded_host;
}  // namespace

// +===========================================================================+
// | [>] check parses host and optional port                     ( test-case ) |
// +===========================================================================+
DOBA_TEST("check parses host and optional port") {
  struct test_case {
    std::string_view source;
    std::string_view host;
    std::string_view port;
    helpers::host_type type;
  };
  constexpr test_case cases[] = {
      {"example.com", "example.com", "", helpers::host_type::kRegName},
      {"example.com:80", "example.com", "80", helpers::host_type::kRegName},
      {"192.0.2.1:8080", "192.0.2.1", "8080", helpers::host_type::kIpV4Address},
      {"[2001:db8::1]:443", "[2001:db8::1]", "443",
       helpers::host_type::kIpLiteral},
  };
  for (const auto& test : cases) {
    parsed_host_port parsed;
    DOBA_EXPECT(x_forwarded_host::check(test.source, parsed));
    DOBA_EXPECT_EQUAL(parsed.host, test.host);
    DOBA_EXPECT_EQUAL(parsed.port, test.port);
    DOBA_EXPECT_EQUAL(parsed.type, test.type);
    const std::string padded = "x" + std::string(test.source) + "y";
    parsed_host_port bounded;
    DOBA_EXPECT(x_forwarded_host::check(
        std::string_view(padded).substr(1, test.source.size()), bounded));
    DOBA_EXPECT_EQUAL(bounded.host, test.host);
    DOBA_EXPECT_EQUAL(bounded.port, test.port);
  }
  parsed_host_port empty;
  DOBA_EXPECT(x_forwarded_host::check("", empty));
  parsed_host_port empty_host;
  DOBA_EXPECT(x_forwarded_host::check(":80", empty_host));
  parsed_host_port empty_port;
  DOBA_EXPECT(x_forwarded_host::check("example.com:", empty_port));
}
// +===========================================================================+
// | [>] check rejects invalid host values                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("check rejects invalid host values") {
  constexpr std::string_view cases[] = {
      " ",           "example.com:http", "example.com:65536x", "[2001:db8::1",
      "2001:db8::1", "example.com/path", "example.com ",
  };
  for (const auto source : cases) {
    parsed_host_port parsed;
    DOBA_EXPECT(!x_forwarded_host::check(source, parsed));
  }
  parsed_host_port parsed;
  DOBA_EXPECT(!x_forwarded_host::check(std::string_view{"host\0", 5}, parsed));
}
// +===========================================================================+
// | [>] interpret accepts parsed host                           ( test-case ) |
// +===========================================================================+
DOBA_TEST("interpret accepts parsed host") {
  parsed_host_port parsed;
  DOBA_EXPECT(x_forwarded_host::check("example.com:80", parsed));
  martianlabs::doba::protocol::http::v11::connection state;
  policies policy;
  DOBA_EXPECT_EQUAL(x_forwarded_host::interpret(parsed, state, policy),
                    verdict::kAccept);
}
