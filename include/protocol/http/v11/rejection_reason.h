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

#ifndef martianlabs_doba_protocol_http_v11_rejection_reason_h
#define martianlabs_doba_protocol_http_v11_rejection_reason_h

namespace martianlabs::doba::protocol::http::v11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] rejection_reason                                       ( enum-class ) |
// +---------------------------------------------------------------------------+
// | Keep the rejection reason specific enough to choose an HTTP status,       |
// | whether rejection came from syntax or semantics.                          |
// +---------------------------------------------------------------------------+
// | kNone                 | no rejection (or reason not tracked yet).         |
// +---------------------------------------------------------------------------+
// | kSyntax               | malformed message; maps to 400 Bad Request.       |
// +---------------------------------------------------------------------------+
// | kPayloadTooLarge      | a policy limit on message size was exceeded; maps |
// | | to 413 Content Too Large.                                               |
// +---------------------------------------------------------------------------+
// | kUnsupportedFeature   | a syntactically valid feature is disabled by      |
// | | policy; maps to 501 Not Implemented.                                    |
// +---------------------------------------------------------------------------+
// | kVersionNotSupported  | the HTTP version is above what this server        |
// | | speaks; maps to 505 HTTP Version Not Supported.                         |
// +---------------------------------------------------------------------------+
// | kUriTooLong           | the request-target exceeds the configured         |
// | | length limit; maps to 414 URI Too Long.                                 |
// +---------------------------------------------------------------------------+
// | kHeaderFieldsTooLarge | the header section exceeds the configured         |
// | | size limit; maps to 431 Request Header Fields                           |
// | | Too Large.                                                              |
// +---------------------------------------------------------------------------+
// | kHandlerError         | the user's request handler threw an exception;    |
// | | maps to 500 Internal Server Error.                                      |
// +---------------------------------------------------------------------------+
// | kExpectationFailed    | the Expect field states an expectation this       |
// | | server cannot meet; maps to 417 Expectation                             |
// | | Failed.                                                                 |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
enum class rejection_reason {
  kNone,                  // No rejection, or no reason recorded yet.
  kSyntax,                // Malformed message: 400 Bad Request.
  kPayloadTooLarge,       // Message too large for policy: 413.
  kUnsupportedFeature,    // Valid but disabled feature: 501.
  kVersionNotSupported,   // We don't speak this higher HTTP version: 505.
  kUriTooLong,            // Request target too long: 414.
  kHeaderFieldsTooLarge,  // Too many header bytes: 431.
  kHandlerError,          // The handler threw: 500.
  kExpectationFailed,     // Can't meet this Expect value: 417.
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
