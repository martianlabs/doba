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
    static constexpr uint8_t kGet[] = "GET";
    static constexpr uint8_t kHead[] = "HEAD";
    static constexpr uint8_t kPost[] = "POST";
    static constexpr uint8_t kPut[] = "PUT";
    static constexpr uint8_t kDelete[] = "DELETE";
    static constexpr uint8_t kConnect[] = "CONNECT";
    static constexpr uint8_t kOptions[] = "OPTIONS";
    static constexpr uint8_t kTrace[] = "TRACE";
  };
  // +------------------------+--------------------+---------------------------+
  // |        Header          |       Sección      |          Tipo             |
  // +------------------------+--------------------+---------------------------+
  // | Connection             | RFC 9110 §7.6.1    | General                   |
  // | Date                   | RFC 9110 §8.6.1    | General                   |
  // | Via                    | RFC 9110 §7.6.3    | General                   |
  // | Cache-Control          | RFC 9111 §5.2      | General (caching)         |
  // | Pragma                 | RFC 9111 §5.4      | General (legacy caching)  |
  // | Warning                | RFC 9111 §5.5      | General (caching)         |
  // | Host                   | RFC 9110 §7.2      | Request                   |
  // | User-Agent             | RFC 9110 §10.1.5   | Request                   |
  // | Accept                 | RFC 9110 §12.5.1   | Request                   |
  // | Accept-Encoding        | RFC 9110 §12.5.3   | Request                   |
  // | Accept-Language        | RFC 9110 §12.5.4   | Request                   |
  // | Authorization          | RFC 9110 §11.6.2   | Request (auth)            |
  // | Expect                 | RFC 9110 §10.1.3   | Request                   |
  // | From                   | RFC 9110 §10.1.4   | Request                   |
  // | If-Match               | RFC 9110 §13.1.1   | Request (conditional)     |
  // | If-None-Match          | RFC 9110 §13.1.2   | Request (conditional)     |
  // | If-Modified-Since      | RFC 9110 §13.1.3   | Request (conditional)     |
  // | If-Unmodified-Since    | RFC 9110 §13.1.4   | Request (conditional)     |
  // | Range                  | RFC 9110 §14.1     | Request (range requests)  |
  // | Referer                | RFC 9110 §10.1.6   | Request                   |
  // | TE                     | RFC 9110 §10.1.7   | Request                   |
  // | Upgrade                | RFC 9110 §7.8      | Request                   |
  // | Cookie                 | RFC 6265           | Request (cookies)         |
  // | Location               | RFC 9110 §10.2.2   | Response                  |
  // | Server                 | RFC 9110 §10.2.5   | Response                  |
  // | Vary                   | RFC 9110 §12.5.5   | Response                  |
  // | WWW-Authenticate       | RFC 9110 §11.6.1   | Response (auth challenge) |
  // | Content-Length         | RFC 9110 §8.6.1    | Entity                    |
  // | Content-Type           | RFC 9110 §8.3.1    | Entity                    |
  // | Content-Encoding       | RFC 9110 §8.4      | Entity                    |
  // | Content-Language       | RFC 9110 §8.5      | Entity                    |
  // | Content-Range          | RFC 9110 §14.4     | Entity                    |
  // | Trailer                | RFC 9110 §6.5      | Trailer header            |
  // | Transfer-Encoding      | RFC 9112 §6.1      | Special                   |
  // | Allow                  | RFC 9110 §10.2.1   | Response                  |
  // | Retry-After            | RFC 9110 §10.2.3   | Response                  |
  // | Accept-Ranges          | RFC 9110 §14.2     | Response                  |
  // | ETag                   | RFC 9110 §8.8.3    | Response                  |
  // | Last-Modified          | RFC 9110 §8.8.2    | Response                  |
  // +------------------------+--------------------+---------------------------+
  struct header {
    static constexpr uint8_t kConnection[] = "connection";
    static constexpr uint8_t kDate[] = "date";
    static constexpr uint8_t kVia[] = "via";
    static constexpr uint8_t kCacheControl[] = "cache-control";
    static constexpr uint8_t kPragma[] = "pragma";
    static constexpr uint8_t kWarning[] = "warning";
    static constexpr uint8_t kHost[] = "host";
    static constexpr uint8_t kUserAgent[] = "user-agent";
    static constexpr uint8_t kAccept[] = "accept";
    static constexpr uint8_t kAcceptEncoding[] = "accept-encoding";
    static constexpr uint8_t kAcceptLanguage[] = "accept-language";
    static constexpr uint8_t kAuthorization[] = "authorization";
    static constexpr uint8_t kExpect[] = "expect";
    static constexpr uint8_t kFrom[] = "from";
    static constexpr uint8_t kIfMatch[] = "if-match";
    static constexpr uint8_t kIfNoneMatch[] = "if-none-match";
    static constexpr uint8_t kIfModifiedSince[] = "if-modified-since";
    static constexpr uint8_t kIfUnmodifiedSince[] = "if-unmodified-since";
    static constexpr uint8_t kRange[] = "range";
    static constexpr uint8_t kReferer[] = "referer";
    static constexpr uint8_t kTe[] = "te";
    static constexpr uint8_t kUpgrade[] = "upgrade";
    static constexpr uint8_t kCookie[] = "cookie";
    static constexpr uint8_t kLocation[] = "location";
    static constexpr uint8_t kServer[] = "server";
    static constexpr uint8_t kVary[] = "vary";
    static constexpr uint8_t kWwwAuthenticate[] = "www-authenticate";
    static constexpr uint8_t kContentLength[] = "content-length";
    static constexpr uint8_t kContentType[] = "content-type";
    static constexpr uint8_t kContentEncoding[] = "content-encoding";
    static constexpr uint8_t kContentLanguage[] = "content-language";
    static constexpr uint8_t kContentRange[] = "content-range";
    static constexpr uint8_t kTrailer[] = "trailer";
    static constexpr uint8_t kTransferEncoding[] = "transfer-encoding";
    static constexpr uint8_t kAllow[] = "allow";
    static constexpr uint8_t kRetryAfter[] = "retry-after";
    static constexpr uint8_t kAcceptRanges[] = "accept-ranges";
    static constexpr uint8_t kETag[] = "etag";
    static constexpr uint8_t kLastModified[] = "last-modified";
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
    static constexpr uint16_t kContinue = 100;
    static constexpr uint16_t kSwitchingProtocols = 101;
    // successful (2xx)
    static constexpr uint16_t kOK = 200;
    static constexpr uint16_t kCreated = 201;
    static constexpr uint16_t kAccepted = 202;
    static constexpr uint16_t kNonAuthoritativeInformation = 203;
    static constexpr uint16_t kNoContent = 204;
    static constexpr uint16_t kResetContent = 205;
    static constexpr uint16_t kPartialContent = 206;
    // redirection (3xx)
    static constexpr uint16_t kMultipleChoices = 300;
    static constexpr uint16_t kMovedPermanently = 301;
    static constexpr uint16_t kFound = 302;
    static constexpr uint16_t kSeeOther = 303;
    static constexpr uint16_t kNotModified = 304;
    static constexpr uint16_t kUseProxy = 305;
    static constexpr uint16_t kUnused = 306;
    static constexpr uint16_t kTemporaryRedirect = 307;
    static constexpr uint16_t kPermanentRedirect = 308;
    // client errors (4xx)
    static constexpr uint16_t kBadRequest = 400;
    static constexpr uint16_t kUnauthorized = 401;
    static constexpr uint16_t kPaymentRequired = 402;
    static constexpr uint16_t kForbidden = 403;
    static constexpr uint16_t kNotFound = 404;
    static constexpr uint16_t kMethodNotAllowed = 405;
    static constexpr uint16_t kNotAcceptable = 406;
    static constexpr uint16_t kProxyAuthRequired = 407;
    static constexpr uint16_t kRequestTimeout = 408;
    static constexpr uint16_t kConflict = 409;
    static constexpr uint16_t kGone = 410;
    static constexpr uint16_t kLengthRequired = 411;
    static constexpr uint16_t kPreconditionFailed = 412;
    static constexpr uint16_t kContentTooLarge = 413;
    static constexpr uint16_t kURITooLong = 414;
    static constexpr uint16_t kUnsupportedMediaType = 415;
    static constexpr uint16_t kRangeNotSatisfiable = 416;
    static constexpr uint16_t kExpectationFailed = 417;
    static constexpr uint16_t kUnused418 = 418;
    static constexpr uint16_t kMisdirectedRequest = 421;
    static constexpr uint16_t kUnprocessableContent = 422;
    static constexpr uint16_t kUpgradeRequired = 426;
    // server errors (5xx)
    static constexpr uint16_t kInternalServerError = 500;
    static constexpr uint16_t kNotImplemented = 501;
    static constexpr uint16_t kBadGateway = 502;
    static constexpr uint16_t kServiceUnavailable = 503;
    static constexpr uint16_t kGatewayTimeout = 504;
    static constexpr uint16_t kHTTPVersionNotSupported = 505;
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
  };
};
}  // namespace martianlabs::doba::protocol::http11

#endif
