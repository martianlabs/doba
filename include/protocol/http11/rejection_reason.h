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


#ifndef martianlabs_doba_protocol_http11_rejection_reason_h
#define martianlabs_doba_protocol_http11_rejection_reason_h

namespace martianlabs::doba::protocol::http11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] rejection_reason                                       ( enum-class ) |
// +---------------------------------------------------------------------------+
// | Why a verdict::kReject (or a syntactic kInvalidSource) happened, at the   |
// | granularity the HTTP/1.1 layer needs to pick a status code. This is       |
// | strictly an http11 concept: it is translated to a protocol-agnostic       |
// | integer before crossing into protocol::deserialization_result, so the     |
// | generic protocol/transport contract never learns HTTP vocabulary.         |
// +---------------------------------------------------------------------------+
// | kNone                 | no rejection (or reason not tracked yet).         |
// +-----------------------+---------------------------------------------------+
// | kSyntax               | malformed message; maps to 400 Bad Request.       |
// +-----------------------+---------------------------------------------------+
// | kPayloadTooLarge      | a policy limit on message size was exceeded; maps |
// |                       | to 413 Content Too Large.                         |
// +-----------------------+---------------------------------------------------+
// | kUnsupportedFeature   | a syntactically valid feature is disabled by      |
// |                       | policy; maps to 501 Not Implemented.              |
// +-----------------------+---------------------------------------------------+
// | kVersionNotSupported  | the HTTP version is above what this server        |
// |                       | speaks; maps to 505 HTTP Version Not Supported.   |
// +-----------------------+---------------------------------------------------+
// | kUriTooLong           | the request-target exceeds the configured         |
// |                       | length limit; maps to 414 URI Too Long.           |
// +-----------------------+---------------------------------------------------+
// | kHeaderFieldsTooLarge | the header section exceeds the configured         |
// |                       | size limit; maps to 431 Request Header Fields     |
// |                       | Too Large.                                        |
// +-----------------------+---------------------------------------------------+
// | kHandlerError         | the user's request handler threw an exception;    |
// |                       | maps to 500 Internal Server Error.                 |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
enum class rejection_reason {
  kNone,                  // no rejection (or reason not tracked yet).
  kSyntax,                // malformed message; maps to 400 Bad Request.
  kPayloadTooLarge,       // policy size limit exceeded; maps to 413.
  kUnsupportedFeature,    // valid feature disabled by policy; maps to 501.
  kVersionNotSupported,   // HTTP version above what is spoken; maps to 505.
  kUriTooLong,            // request-target too long; maps to 414.
  kHeaderFieldsTooLarge,  // header section too large; maps to 431.
  kHandlerError,          // user handler threw an exception; maps to 500.
};
}  // namespace martianlabs::doba::protocol::http11

#endif
