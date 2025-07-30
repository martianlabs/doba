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
  struct method {
    static constexpr char kGet[] = "GET";
    static constexpr char kHead[] = "HEAD";
    static constexpr char kPost[] = "POST";
    static constexpr char kPut[] = "PUT";
    static constexpr char kDelete[] = "DELETE";
    static constexpr char kConnect[] = "CONNECT";
    static constexpr char kOptions[] = "OPTIONS";
    static constexpr char kTrace[] = "TRACE";
  };
  // +------+-------------------------------+
  // | Code | Reason Phrase                 |
  // +------+-------------------------------+
  // | 100  | Continue                      |
  // | 101  | Switching Protocols           |
  // | 200  | OK                            |
  // | 201  | Created                       |
  // | 202  | Accepted                      |
  // | 203  | Non-Authoritative Information |
  // | 204  | No Content                    |
  // | 205  | Reset Content                 |
  // | 206  | Partial Content               |
  // | 300  | Multiple Choices              |
  // | 301  | Moved Permanently             |
  // | 302  | Found                         |
  // | 303  | See Other                     |
  // | 304  | Not Modified                  |
  // | 305  | Use Proxy                     |
  // | 306  | (Unused)                      |
  // | 307  | Temporary Redirect            |
  // | 308  | Permanent Redirect            |
  // | 400  | Bad Request                   |
  // | 401  | Unauthorized                  |
  // | 402  | Payment Required              |
  // | 403  | Forbidden                     |
  // | 404  | Not Found                     |
  // | 405  | Method Not Allowed            |
  // | 406  | Not Acceptable                |
  // | 407  | Proxy Authentication Required |
  // | 408  | Request Timeout               |
  // | 409  | Conflict                      |
  // | 410  | Gone                          |
  // | 411  | Length Required               |
  // | 412  | Precondition Failed           |
  // | 413  | Content Too Large             |
  // | 414  | URI Too Long                  |
  // | 415  | Unsupported Media Type        |
  // | 416  | Range Not Satisfiable         |
  // | 417  | Expectation Failed            |
  // | 418  | (Unused)                      |
  // | 421  | Misdirected Request           |
  // | 422  | Unprocessable Content         |
  // | 426  | Upgrade Required              |
  // | 500  | Internal Server Error         |
  // | 501  | Not Implemented               |
  // | 502  | Bad Gateway                   |
  // | 503  | Service Unavailable           |
  // | 504  | Gateway Timeout               |
  // | 505  | HTTP Version Not Supported    |
  // +------+-------------------------------+
  struct status_code {
    // informational (1xx)
    static constexpr const char kContinue[] = "100";
    static constexpr const char kSwitchingProtocols[] = "101";
    // successful (2xx)
    static constexpr const char kOK[] = "200";
    static constexpr const char kCreated[] = "201";
    static constexpr const char kAccepted[] = "202";
    static constexpr const char kNonAuthoritativeInformation[] = "203";
    static constexpr const char kNoContent[] = "204";
    static constexpr const char kResetContent[] = "205";
    static constexpr const char kPartialContent[] = "206";
    // redirection (3xx)
    static constexpr const char kMultipleChoices[] = "300";
    static constexpr const char kMovedPermanently[] = "301";
    static constexpr const char kFound[] = "302";
    static constexpr const char kSeeOther[] = "303";
    static constexpr const char kNotModified[] = "304";
    static constexpr const char kUseProxy[] = "305";
    static constexpr const char kTemporaryRedirect[] = "307";
    static constexpr const char kPermanentRedirect[] = "308";
    // client errors (4xx)
    static constexpr const char kBadRequest[] = "400";
    static constexpr const char kUnauthorized[] = "401";
    static constexpr const char kPaymentRequired[] = "402";
    static constexpr const char kForbidden[] = "403";
    static constexpr const char kNotFound[] = "404";
    static constexpr const char kMethodNotAllowed[] = "405";
    static constexpr const char kNotAcceptable[] = "406";
    static constexpr const char kProxyAuthenticationRequired[] = "407";
    static constexpr const char kRequestTimeout[] = "408";
    static constexpr const char kConflict[] = "409";
    static constexpr const char kGone[] = "410";
    static constexpr const char kLengthRequired[] = "411";
    static constexpr const char kPreconditionFailed[] = "412";
    static constexpr const char kContentTooLarge[] = "413";
    static constexpr const char kURITooLong[] = "414";
    static constexpr const char kUnsupportedMediaType[] = "415";
    static constexpr const char kRangeNotSatisfiable[] = "416";
    static constexpr const char kExpectationFailed[] = "417";
    static constexpr const char kMisdirectedRequest[] = "421";
    static constexpr const char kUnprocessableContent[] = "422";
    static constexpr const char kUpgradeRequired[] = "426";
    // server errors (5xx)
    static constexpr const char kInternalServerError[] = "500";
    static constexpr const char kNotImplemented[] = "501";
    static constexpr const char kBadGateway[] = "502";
    static constexpr const char kServiceUnavailable[] = "503";
    static constexpr const char kGatewayTimeout[] = "504";
    static constexpr const char kHTTPVersionNotSupported[] = "505";
  };
  struct reason_phrase {
    // informational (1xx)
    static constexpr const char kContinue[] = "Continue";
    static constexpr const char kSwitchingProtocols[] = "Switching Protocols";
    // successful (2xx)
    static constexpr const char kOK[] = "OK";
    static constexpr const char kCreated[] = "Created";
    static constexpr const char kAccepted[] = "Accepted";
    static constexpr const char kNonAuthoritativeInformation[] =
        "Non-Authoritative Information";
    static constexpr const char kNoContent[] = "No Content";
    static constexpr const char kResetContent[] = "Reset Content";
    static constexpr const char kPartialContent[] = "Partial Content";
    // redirection (3xx)
    static constexpr const char kMultipleChoices[] = "Multiple Choices";
    static constexpr const char kMovedPermanently[] = "Moved Permanently";
    static constexpr const char kFound[] = "Found";
    static constexpr const char kSeeOther[] = "See Other";
    static constexpr const char kNotModified[] = "Not Modified";
    static constexpr const char kUseProxy[] = "Use Proxy";
    static constexpr const char kTemporaryRedirect[] = "Temporary Redirect";
    static constexpr const char kPermanentRedirect[] = "Permanent Redirect";
    // client errors (4xx)
    static constexpr const char kBadRequest[] = "Bad Request";
    static constexpr const char kUnauthorized[] = "Unauthorized";
    static constexpr const char kPaymentRequired[] = "Payment Required";
    static constexpr const char kForbidden[] = "Forbidden";
    static constexpr const char kNotFound[] = "Not Found";
    static constexpr const char kMethodNotAllowed[] = "Method Not Allowed";
    static constexpr const char kNotAcceptable[] = "Not Acceptable";
    static constexpr const char kProxyAuthenticationRequired[] =
        "Proxy Authentication Required";
    static constexpr const char kRequestTimeout[] = "Request Timeout";
    static constexpr const char kConflict[] = "Conflict";
    static constexpr const char kGone[] = "Gone";
    static constexpr const char kLengthRequired[] = "Length Required";
    static constexpr const char kPreconditionFailed[] = "Precondition Failed";
    static constexpr const char kContentTooLarge[] = "Content Too Large";
    static constexpr const char kURITooLong[] = "URI Too Long";
    static constexpr const char kUnsupportedMediaType[] =
        "Unsupported Media Type";
    static constexpr const char kRangeNotSatisfiable[] =
        "Range Not Satisfiable";
    static constexpr const char kExpectationFailed[] = "Expectation Failed";
    static constexpr const char kMisdirectedRequest[] = "Misdirected Request";
    static constexpr const char kUnprocessableContent[] =
        "Unprocessable Content";
    static constexpr const char kUpgradeRequired[] = "Upgrade Required";
    // server errors (5xx)
    static constexpr const char kInternalServerError[] =
        "Internal Server Error";
    static constexpr const char kNotImplemented[] = "Not Implemented";
    static constexpr const char kBadGateway[] = "Bad Gateway";
    static constexpr const char kServiceUnavailable[] = "Service Unavailable";
    static constexpr const char kGatewayTimeout[] = "Gateway Timeout";
    static constexpr const char kHTTPVersionNotSupported[] =
        "HTTP Version Not Supported";
  };
  struct character {
    static constexpr uint8_t kHTab = 0x09;
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
  struct string {
    static constexpr uint8_t kCrLf[] = "\r\n";
    static constexpr uint8_t kEndOfHeaders[] = "\r\n\r\n";
    static constexpr uint8_t kHttpVersion[] = "HTTP/1.1";
  };
};
}  // namespace martianlabs::doba::protocol::http11

#endif
