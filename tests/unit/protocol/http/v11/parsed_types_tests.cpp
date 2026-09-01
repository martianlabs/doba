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

#include "protocol/http/v11/parsed_types.h"
#include "test_helper.h"

namespace {
using namespace martianlabs::doba::protocol::http::v11;
using martianlabs::doba::protocol::http::helpers;
}  // namespace

// +===========================================================================+
// | [>] parsed products default to empty zero copy values       ( test-case ) |
// +===========================================================================+
DOBA_TEST("parsed products default to empty zero copy values") {
  const parsed_host_port host;
  DOBA_EXPECT(host.host.empty());
  DOBA_EXPECT(host.port.empty());
  DOBA_EXPECT(host.scheme.empty());
  DOBA_EXPECT_EQUAL(host.type, helpers::host_type::kUnknown);
  DOBA_EXPECT(parsed_token_list{}.elements.empty());
  DOBA_EXPECT(parsed_parameter_list{}.elements.empty());
  DOBA_EXPECT(parsed_scalar{}.value.empty());
  DOBA_EXPECT(parsed_via_list{}.elements.empty());
  DOBA_EXPECT(parsed_forwarded_list{}.elements.empty());
  DOBA_EXPECT(parsed_host_port_list{}.elements.empty());
}
// +===========================================================================+
// | [>] parsed products retain all nested fields                ( test-case ) |
// +===========================================================================+
DOBA_TEST("parsed products retain all nested fields") {
  const parsed_host_port host{"example.com", "443",
                              helpers::host_type::kRegName, "https"};
  const parsed_token_list tokens{{"close", "upgrade"}};
  const parsed_parameter_list parameters{{"gzip", "chunked"}};
  const parsed_scalar scalar{"100-continue"};
  const parsed_via_list via{{{"HTTP/1.1", "proxy", "(comment)"}}};
  const parsed_forwarded_list forwarded{
      {{{{"for", "192.0.2.1"}, {"proto", "https"}}}}};
  const parsed_host_port_list hosts{{host}};
  DOBA_EXPECT_EQUAL(host.host, "example.com");
  DOBA_EXPECT_EQUAL(host.port, "443");
  DOBA_EXPECT_EQUAL(host.scheme, "https");
  DOBA_EXPECT_EQUAL(tokens.elements.size(), 2);
  DOBA_EXPECT_EQUAL(parameters.elements[1], "chunked");
  DOBA_EXPECT_EQUAL(scalar.value, "100-continue");
  DOBA_EXPECT_EQUAL(via.elements[0].comment, "(comment)");
  DOBA_EXPECT_EQUAL(forwarded.elements[0].pairs[1].name, "proto");
  DOBA_EXPECT_EQUAL(hosts.elements[0].host, "example.com");
}
