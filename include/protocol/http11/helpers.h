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

#ifndef martianlabs_doba_protocol_http11_helpers_h
#define martianlabs_doba_protocol_http11_helpers_h

#include "constants.h"

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
//                                                                    ( macros )
// -----------------------------------------------------------------------------
// Set of helpful macros.
// -----------------------------------------------------------------------------
// =============================================================================
#define IS_SP(c) (c == 0x20)
#define IS_CR(c) (c == 0x0D)
#define IS_LF(c) (c == 0x0A)
#define IS_DIGIT(c) (c >= 0x30 && c <= 0x39)
#define IS_HEX_DIGIT(c) \
  (IS_DIGIT(c) || (c >= 0x41 && c <= 0x46) || (c >= 0x61 && c <= 0x66))
#define IS_ALPHA(c) ((c >= 0x41 && c <= 0x5A) || (c >= 0x61 && c <= 0x7A))
#define IS_TOKEN(c)                                                 \
  (c == 0x21 || c == 0x23 || c == 0x24 || c == 0x25 || c == 0x26 || \
   c == 0x27 || c == 0x2A || c == 0x2B || c == 0x2D || c == 0x2E || \
   c == 0x5E || c == 0x5F || c == 0x60 || c == 0x7C || c == 0x7E || \
   IS_DIGIT(c) || IS_ALPHA(c))
#define IS_UNRESERVED(c)                                              \
  (c == 0x2D || c == 0x2E || c == 0x5F || c == 0x7E || IS_DIGIT(c) || \
   IS_ALPHA(c))
#define IS_SUB_DELIM(c)                                             \
  (c == 0x21 || c == 0x24 || c == 0x26 || c == 0x27 || c == 0x28 || \
   c == 0x29 || c == 0x2A || c == 0x2B || c == 0x2C || c == 0x3B || c == 0x3D)
#define IS_PCHAR(c) \
  (c == 0x3A || c == 0x40 || IS_UNRESERVED(c) || IS_SUB_DELIM(c))
}  // namespace martianlabs::doba::protocol::http11

#endif
