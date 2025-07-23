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

#ifndef martianlabs_doba_protocol_http11_status_code_h
#define martianlabs_doba_protocol_http11_status_code_h

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// status_code                                                        ( macros )
// -----------------------------------------------------------------------------
// This section holds for the http 1.1 status codes.
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
#define SC_100_CONTINUE 100
#define SC_101_SWITCHING_PROTOCOLS 101
#define SC_200_OK 200
#define SC_201_CREATED 201
#define SC_202_ACCEPTED 202
#define SC_203_NON_AUTHORITATIVE_INFORMATION 203
#define SC_204_NO_CONTENT 204
#define SC_205_RESET_CONTENT 205
#define SC_206_PARTIAL_CONTENT 206
#define SC_300_MULTIPLE_CHOICES 300
#define SC_301_MOVED_PERMANENTLY 301
#define SC_302_FOUND 302
#define SC_303_SEE_OTHER 303
#define SC_304_NOT_MODIFIED 304
#define SC_305_USE_PROXY 305
#define SC_306_UNUSED 306
#define SC_307_TEMPORARY_REDIRECT 307
#define SC_308_PERMANENT_REDIRECT 308
#define SC_400_BAD_REQUEST 400
#define SC_401_UNAUTHORIZED 401
#define SC_402_PAYMENT_REQUIRED 402
#define SC_403_FORBIDDEN 403
#define SC_404_NOT_FOUND 404
#define SC_405_METHOD_NOT_ALLOWED 405
#define SC_406_NOT_ACCEPTABLE 406
#define SC_407_PROXY_AUTHENTICATION_REQUIRED 407
#define SC_408_REQUEST_TIMEOUT 408
#define SC_409_CONFLICT 409
#define SC_410_GONE 410
#define SC_411_LENGTH_REQUIRED 411
#define SC_412_PRECONDITION_FAILED 412
#define SC_413_CONTENT_TOO_LARGE 413
#define SC_414_URI_TOO_LONG 414
#define SC_415_UNSUPPORTED_MEDIA_TYPE 415
#define SC_416_RANGE_NOT_SATISFIABLE 416
#define SC_417_EXPECTATION_FAILED 417
#define SC_418_IM_A_TEAPOT 418
#define SC_421_MISDIRECTED_REQUEST 421
#define SC_422_UNPROCESSABLE_CONTENT 422
#define SC_426_UPGRADE_REQUIRED 426
#define SC_500_INTERNAL_SERVER_ERROR 500
#define SC_501_NOT_IMPLEMENTED 501
#define SC_502_BAD_GATEWAY 502
#define SC_503_SERVICE_UNAVAILABLE 503
#define SC_504_GATEWAY_TIMEOUT 504
#define SC_505_HTTP_VERSION_NOT_SUPPORTED 505
}  // namespace martianlabs::doba::protocol::http11

#endif
