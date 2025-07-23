//      _       _           
//   __| | ___ | |__   __ _ 
//  / _` |/ _ \| '_ \ / _` |
// | (_| | (_) | |_) | (_| |
//  \__,_|\___/|_.__/ \__,_|
// 
// Copyright 2025 martianLabs
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef martianlabs_doba_protocol_http11_status_line_h
#define martianlabs_doba_protocol_http11_status_line_h

#include "status_code.h"
#include "reason_phrase.h"

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// status_line                                                        ( macros )
// -----------------------------------------------------------------------------
// This section holds for the http 1.1 status lines.
// -----------------------------------------------------------------------------
// =============================================================================
#define SP
#define CRLF \r\n
#define STR_VALUE(x) #x
#define CC_RAW(a, b) a##b
#define CC_EXP(a, b) CC_RAW(a, b)
#define EAS(x) STR_VALUE(x)
#define HTTP_HEADER HTTP
#define SLASH /
#define ONE_DOT_ONE 1.1
#define VERSION CC_EXP(HTTP_HEADER, CC_EXP(SLASH, ONE_DOT_ONE))
#define SL(x) VERSION CC_RAW(SC_, x) CC_EXP(CC_RAW(RP_, x), CRLF)
}  // namespace martianlabs::doba::protocol::http11

#endif
