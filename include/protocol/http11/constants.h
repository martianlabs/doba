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

#ifndef martianlabs_doba_protocol_http11_constants_h
#define martianlabs_doba_protocol_http11_constants_h

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// constants                                                          ( struct )
// -----------------------------------------------------------------------------
// This struct holds for the http 1.1 constants.
// -----------------------------------------------------------------------------
// =============================================================================
struct constants {
  // https://www.rfc-editor.org/rfc/rfc9110#section-9
  // +---------+---------------------------------------------------------------+
  // | Method  | Description                                                   |
  // +---------+---------------------------------------------------------------+
  // | GET     | Transfer a current representation of the target resource.     |
  // | HEAD    | Same as GET, but do not transfer the response content.        |
  // | POST    | Perform resource-specific processing on the request content.  |
  // | PUT     | Replace all current representations of the target resource    |
  // |         | with the request content.                                     |
  // | DELETE  | Remove all current representations of the target resource.    |
  // | CONNECT | Establish a tunnel to the server identified by the            |
  // |         | target resource.                                              |
  // | OPTIONS | Describe the communication options for the target resource.   |
  // | TRACE   | Perform a message loop-back test along the path to the        |
  // |         | target resource.                                              |
  // +---------+---------------------------------------------------------------+
  struct methods {
    static constexpr uint8_t kGet[] = "GET";
    static constexpr uint8_t kHead[] = "HEAD";
    static constexpr uint8_t kPost[] = "POST";
    static constexpr uint8_t kPut[] = "PUT";
    static constexpr uint8_t kDelete[] = "DELETE";
    static constexpr uint8_t kConnect[] = "CONNECT";
    static constexpr uint8_t kOptions[] = "OPTIONS";
    static constexpr uint8_t kTrace[] = "TRACE";
  };
  struct characters {
    static constexpr uint8_t kTab = 0x09;
    static constexpr uint8_t kSpace = 0x20;
    static constexpr uint8_t kCr = 0x0D;
    static constexpr uint8_t kLf = 0x0A;
    static constexpr uint8_t kExclamation = 0x21;
    static constexpr uint8_t kHash = 0x23;
    static constexpr uint8_t kDollar = 0x24;
    static constexpr uint8_t kPercent = 0x25;
    static constexpr uint8_t kApostrophe = 0x27;
    static constexpr uint8_t kAsterisk = 0x2A;
    static constexpr uint8_t kPlus = 0x2B;
    static constexpr uint8_t kAmpersand = 0x26;
    static constexpr uint8_t kHyphen = 0x2D;
    static constexpr uint8_t kDot = 0x2E;
    static constexpr uint8_t kCircumflex = 0x5E;
    static constexpr uint8_t kUnderscore = 0x5F;
    static constexpr uint8_t kBackTick = 0x60;
    static constexpr uint8_t kVerticalBar = 0x7C;
    static constexpr uint8_t kTilde = 0x7E;
    static constexpr uint8_t kLParenthesis = 0x28;
    static constexpr uint8_t kRParenthesis = 0x29;
    static constexpr uint8_t kComma = 0x2C;
    static constexpr uint8_t kSemiColon = 0x3B;
    static constexpr uint8_t kEquals = 0x3D;
    static constexpr uint8_t kColon = 0x3A;
    static constexpr uint8_t kSlash = 0x2F;
    static constexpr uint8_t kQuestion = 0x3F;
    static constexpr uint8_t kAt = 0x40;
    static constexpr uint8_t kObsTextStart = 0x80;
    static constexpr uint8_t kObsTextEnd = 0xFF;
    static constexpr uint8_t k0 = 0x30;
    static constexpr uint8_t k9 = 0x39;
    static constexpr uint8_t kAUpperCase = 0x41;
    static constexpr uint8_t kFUpperCase = 0x46;
    static constexpr uint8_t kHUpperCase = 0x48;
    static constexpr uint8_t kPUpperCase = 0x50;
    static constexpr uint8_t kTUpperCase = 0x54;
    static constexpr uint8_t kZUpperCase = 0x5A;
    static constexpr uint8_t kALowerCase = 0x61;
    static constexpr uint8_t kFLowerCase = 0x66;
    static constexpr uint8_t kZLowerCase = 0x7A;
  };
  struct strings {
    static constexpr uint8_t kCrLf[] = "\r\n";
    static constexpr uint8_t kEndOfHeaders[] = "\r\n\r\n";
  };
};
}  // namespace martianlabs::doba::protocol::http11

#endif
