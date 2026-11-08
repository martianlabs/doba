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
// | RFC 3986 S3.2.2 / S3.2.3 - host [ ":" port ]                              |
// +---------------------------------------------------------------------------+
// | Borrow the parsed host and raw port; keep IPv6 brackets intact. Either    |
// | may be empty. type identifies the host form. scheme is set only for       |
// | absolute-form targets, so routing can fill a default port.                |
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
// | RFC 9110 S5.6.1 - #token                                                  |
// +---------------------------------------------------------------------------+
// | Borrow nonempty, OWS-trimmed list elements in their original order. An    |
// | empty list gives an empty vector.                                         |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct parsed_token_list {
  std::vector<std::string_view> elements;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parsed_parameter_list                                      ( struct ) |
// +---------------------------------------------------------------------------+
// | RFC 9110 S5.6.1 / S5.6.6 - #( token *( ";" parameter ) )                  |
// +---------------------------------------------------------------------------+
// | Borrow each list element with its raw semicolon parameters intact. Let    |
// | the semantic layer choose which parameters matter.                        |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct parsed_parameter_list {
  std::vector<std::string_view> elements;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parsed_scalar                                              ( struct ) |
// +---------------------------------------------------------------------------+
// | Borrow a checked scalar value with its surrounding OWS removed.           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct parsed_scalar {
  std::string_view value;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parsed_via_element                                         ( struct ) |
// +---------------------------------------------------------------------------+
// | RFC 9110 S7.6.3 - Via                                                     |
// +---------------------------------------------------------------------------+
// | received-protocol = [ protocol-name "/" ] protocol-version                |
// | received-by       = pseudonym / ( uri-host [ ":" port ] )                 |
// +---------------------------------------------------------------------------+
// | Borrow the required received_protocol and received_by parts, plus an      |
// | optional trailing comment.                                                |
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
// | RFC 9110 S7.6.3 - 1#( received-protocol RWS received-by [ RWS comment ] ) |
// +---------------------------------------------------------------------------+
// | Borrow every Via element, in order.                                       |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct parsed_via_list {
  std::vector<parsed_via_element> elements;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parsed_forwarded_pair                                      ( struct ) |
// +---------------------------------------------------------------------------+
// | RFC 7239 S4 - forwarded-pair = token "=" value                            |
// +---------------------------------------------------------------------------+
// | Borrow a Forwarded name=value pair. Keep value quotes and escapes as      |
// | received.                                                                 |
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
// | RFC 7239 S4 - forwarded-element = [ pair ] *( ";" [ pair ] )              |
// +---------------------------------------------------------------------------+
// | Keep a Forwarded element's semicolon-separated pairs in order.            |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct parsed_forwarded_element {
  std::vector<parsed_forwarded_pair> pairs;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parsed_forwarded_list                                      ( struct ) |
// +---------------------------------------------------------------------------+
// | RFC 7239 S4 - Forwarded = 1#forwarded-element                             |
// +---------------------------------------------------------------------------+
// | Borrow every Forwarded element, in order.                                 |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct parsed_forwarded_list {
  std::vector<parsed_forwarded_element> elements;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parsed_host_port_list                                      ( struct ) |
// +---------------------------------------------------------------------------+
// | de-facto X-Forwarded-Host - #( uri-host [ ":" port ] )                    |
// +---------------------------------------------------------------------------+
// | Keep each X-Forwarded-Host item as parsed host and port views.            |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct parsed_host_port_list {
  std::vector<parsed_host_port> elements;
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
