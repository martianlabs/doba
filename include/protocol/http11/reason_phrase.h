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

#ifndef martianlabs_doba_protocol_http11_reason_phrase_h
#define martianlabs_doba_protocol_http11_reason_phrase_h

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// reason_phrase                                                      ( macros )
// -----------------------------------------------------------------------------
// This section holds for the http 1.1 reson phrases.
// -----------------------------------------------------------------------------
// =============================================================================
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
#define RP_100_CONTINUE Continue
#define RP_101_SWITCHING_PROTOCOLS Switching Protocols
#define RP_200_OK OK
#define RP_201_CREATED Created
#define RP_202_ACCEPTED Accepted
#define RP_203_NON_AUTHORITATIVE_INFORMATION Non - Authoritative Information
#define RP_204_NO_CONTENT No Content
#define RP_205_RESET_CONTENT Reset Content
#define RP_206_PARTIAL_CONTENT Partial Content
#define RP_300_MULTIPLE_CHOICES Multiple Choices
#define RP_301_MOVED_PERMANENTLY Moved Permanently
#define RP_302_FOUND Found
#define RP_303_SEE_OTHER See Other
#define RP_304_NOT_MODIFIED Not Modified
#define RP_305_USE_PROXY Use Proxy
#define RP_306_UNUSED Unused
#define RP_307_TEMPORARY_REDIRECT Temporary Redirect
#define RP_308_PERMANENT_REDIRECT Permanent Redirect
#define RP_400_BAD_REQUEST Bad Request
#define RP_401_UNAUTHORIZED Unauthorized
#define RP_402_PAYMENT_REQUIRED Payment Required
#define RP_403_FORBIDDEN Forbidden
#define RP_404_NOT_FOUND Not Found
#define RP_405_METHOD_NOT_ALLOWED Method Not Allowed
#define RP_406_NOT_ACCEPTABLE Not Acceptable
#define RP_407_PROXY_AUTHENTICATION_REQUIRED Proxy Authentication Required
#define RP_408_REQUEST_TIMEOUT Request Timeout
#define RP_409_CONFLICT Conflict
#define RP_410_GONE Gone
#define RP_411_LENGTH_REQUIRED Length Required
#define RP_412_PRECONDITION_FAILED Precondition Failed
#define RP_413_CONTENT_TOO_LARGE Content Too Large
#define RP_414_URI_TOO_LONG URI Too Long
#define RP_415_UNSUPPORTED_MEDIA_TYPE Unsupported Media Type
#define RP_416_RANGE_NOT_SATISFIABLE Range Not Satisfiable
#define RP_417_EXPECTATION_FAILED Expectation Failed
#define RP_418_IM_A_TEAPOT Im a teapot
#define RP_421_MISDIRECTED_REQUEST Misdirected Request
#define RP_422_UNPROCESSABLE_CONTENT Unprocessable Content
#define RP_426_UPGRADE_REQUIRED Upgrade Required
#define RP_500_INTERNAL_SERVER_ERROR Internal Server Error
#define RP_501_NOT_IMPLEMENTED Not Implemented
#define RP_502_BAD_GATEWAY Bad Gateway
#define RP_503_SERVICE_UNAVAILABLE Service Unavailable
#define RP_504_GATEWAY_TIMEOUT Gateway Timeout
#define RP_505_HTTP_VERSION_NOT_SUPPORTED HTTP Version Not Supported
}  // namespace martianlabs::doba::protocol::http11

#endif
