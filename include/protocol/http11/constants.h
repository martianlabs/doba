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
    static constexpr char kGet[] = "GET";
    static constexpr char kHead[] = "HEAD";
    static constexpr char kPost[] = "POST";
    static constexpr char kPut[] = "PUT";
    static constexpr char kDelete[] = "DELETE";
    static constexpr char kConnect[] = "CONNECT";
    static constexpr char kOptions[] = "OPTIONS";
    static constexpr char kTrace[] = "TRACE";
  };
  struct characters {
    static constexpr char kSpace = 0x20;
    static constexpr char kCr = 0x0D;
    static constexpr char kLf = 0x0A;
    static constexpr char kExclamation = 0x21;
    static constexpr char kHash = 0x23;
    static constexpr char kDollar = 0x24;
    static constexpr char kPercent = 0x25;
    static constexpr char kApostrophe = 0x27;
    static constexpr char kAsterisk = 0x2A;
    static constexpr char kPlus = 0x2B;
    static constexpr char kAmpersand = 0x26;
    static constexpr char kHyphen = 0x2D;
    static constexpr char kDot = 0x2E;
    static constexpr char kCircumflex = 0x5E;
    static constexpr char kUnderscore = 0x5F;
    static constexpr char kBackTick = 0x60;
    static constexpr char kVerticalBar = 0x7C;
    static constexpr char kTilde = 0x7E;
    static constexpr char kLParenthesis = 0x28;
    static constexpr char kRParenthesis = 0x29;
    static constexpr char kComma = 0x2C;
    static constexpr char kSemiColon = 0x3B;
    static constexpr char kEquals = 0x3D;
    static constexpr char kColon = 0x3A;
    static constexpr char kSlash = 0x2F;
    static constexpr char kQuestion = 0x3F;
    static constexpr char kAt = 0x40;
    static constexpr char k0 = 0x30;
    static constexpr char k9 = 0x39;
    static constexpr char kAUpperCase = 0x41;
    static constexpr char kFUpperCase = 0x46;
    static constexpr char kHUpperCase = 0x48;
    static constexpr char kPUpperCase = 0x50;
    static constexpr char kTUpperCase = 0x54;
    static constexpr char kZUpperCase = 0x5A;
    static constexpr char kALowerCase = 0x61;
    static constexpr char kFLowerCase = 0x66;
    static constexpr char kZLowerCase = 0x7A;
  };
  struct strings {
    static constexpr char kCrLf[] = "\r\n";
    static constexpr char kEndOfHeaders[] = "\r\n\r\n";
  };
};
}  // namespace martianlabs::doba::protocol::http11

#endif
