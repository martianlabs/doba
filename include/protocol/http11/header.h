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


#ifndef martianlabs_doba_protocol_http11_header_h
#define martianlabs_doba_protocol_http11_header_h

#include <cstddef>
#include <cstdint>

namespace martianlabs::doba::protocol::http11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] header                                                      ( using ) |
// +---------------------------------------------------------------------------+
// | A header is a key-value pair of strings. The key is the header name       |
// (case-insensitive) and the value is the header field-value (case-sensitive).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
using header = std::pair<std::string, std::string>;
using header_view = std::pair<std::string_view, std::string_view>;
}  // namespace martianlabs::doba::protocol::http11

#endif
