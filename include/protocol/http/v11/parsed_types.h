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

#ifndef martianlabs_doba_protocol_http_v11_parsed_types_h
#define martianlabs_doba_protocol_http_v11_parsed_types_h

#include <string_view>
#include <vector>

#include "protocol/http/common/helpers.h"

namespace martianlabs::doba::protocol::http::v11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parsed_host_port                                           ( struct ) |
// +---------------------------------------------------------------------------+
// | Host and raw port digits. Brackets around IP literals stay intact.        |
// | The host and port may both be empty.                                      |
// | scheme comes from absolute-form targets, for default-port comparisons.    |
// | Host headers and authority-form targets leave scheme empty.               |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct parsed_host_port {
  std::string_view host;
  std::string_view port;
  helpers::host_type type = helpers::host_type::kUnknown;
  std::string_view scheme;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parsed_token_list                                          ( struct ) |
// +---------------------------------------------------------------------------+
// | Comma-separated tokens in input order, with spaces/tabs trimmed.          |
// | Empty elements are skipped.                                               |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct parsed_token_list {
  std::vector<std::string_view> elements;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parsed_parameter_list                                      ( struct ) |
// +---------------------------------------------------------------------------+
// | Comma-separated elements, keeping their raw text and ;parameters.         |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct parsed_parameter_list {
  std::vector<std::string_view> elements;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parsed_scalar                                              ( struct ) |
// +---------------------------------------------------------------------------+
// | One checked field value, with surrounding spaces/tabs removed.            |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct parsed_scalar {
  std::string_view value;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parsed_via_element                                         ( struct ) |
// +---------------------------------------------------------------------------+
// | One Via entry, keeping the protocol and sender text as received.          |
// | comment is empty when absent.                                             |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct parsed_via_element {
  std::string_view received_protocol;
  std::string_view received_by;
  std::string_view comment;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parsed_via_list                                            ( struct ) |
// +---------------------------------------------------------------------------+
// | Via entries in input order.                                               |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct parsed_via_list {
  std::vector<parsed_via_element> elements;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parsed_forwarded_pair                                      ( struct ) |
// +---------------------------------------------------------------------------+
// | One Forwarded name=value pair. Quotes and escapes stay in value.          |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct parsed_forwarded_pair {
  std::string_view name;
  std::string_view value;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parsed_forwarded_element                                   ( struct ) |
// +---------------------------------------------------------------------------+
// | The semicolon-separated pairs of one Forwarded entry, in input order.     |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct parsed_forwarded_element {
  std::vector<parsed_forwarded_pair> pairs;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parsed_forwarded_list                                      ( struct ) |
// +---------------------------------------------------------------------------+
// | Forwarded entries in input order.                                         |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct parsed_forwarded_list {
  std::vector<parsed_forwarded_element> elements;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parsed_host_port_list                                      ( struct ) |
// +---------------------------------------------------------------------------+
// | X-Forwarded-Host entries split into host and port, in input order.        |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct parsed_host_port_list {
  std::vector<parsed_host_port> elements;
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
