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

#ifndef martianlabs_doba_protocol_http11_header_names_h
#define martianlabs_doba_protocol_http11_header_names_h

namespace martianlabs::doba::protocol::http11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] header_names                                               ( struct ) |
// +---------------------------------------------------------------------------+
// | This struct holds for the http 1.1 header names specification.            |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct header_names {
  // +=========================================================================+
  // | [>] CONSTANTs                                                ( public ) |
  // +=========================================================================+
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
  static constexpr char kConnection[] = "Connection";
  static constexpr char kDate[] = "Date";
  static constexpr char kVia[] = "Via";
  static constexpr char kCacheControl[] = "Cache-Control";
  static constexpr char kPragma[] = "Pragma";
  static constexpr char kWarning[] = "Warning";
  static constexpr char kHost[] = "Host";
  static constexpr char kUserAgent[] = "User-agent";
  static constexpr char kAccept[] = "Accept";
  static constexpr char kAcceptEncoding[] = "Accept-Encoding";
  static constexpr char kAcceptLanguage[] = "Accept-Language";
  static constexpr char kAuthorization[] = "Authorization";
  static constexpr char kExpect[] = "Expect";
  static constexpr char kFrom[] = "From";
  static constexpr char kIfMatch[] = "If-Match";
  static constexpr char kIfNoneMatch[] = "If-None-Match";
  static constexpr char kIfModifiedSince[] = "If-Modified-Since";
  static constexpr char kIfUnmodifiedSince[] = "If-Unmodified-Since";
  static constexpr char kRange[] = "Range";
  static constexpr char kReferer[] = "Referer";
  static constexpr char kTe[] = "TE";
  static constexpr char kUpgrade[] = "Upgrade";
  static constexpr char kCookie[] = "Cookie";
  static constexpr char kLocation[] = "Location";
  static constexpr char kServer[] = "Server";
  static constexpr char kVary[] = "Vary";
  static constexpr char kWwwAuthenticate[] = "WWW-Authenticate";
  static constexpr char kContentLength[] = "Content-Length";
  static constexpr char kContentType[] = "Content-Type";
  static constexpr char kContentEncoding[] = "Content-Encoding";
  static constexpr char kContentLanguage[] = "Content-Language";
  static constexpr char kContentRange[] = "Content-Range";
  static constexpr char kTrailer[] = "Trailer";
  static constexpr char kTransferEncoding[] = "Transfer-Encoding";
  static constexpr char kAllow[] = "Allow";
  static constexpr char kRetryAfter[] = "Retry-After";
  static constexpr char kAcceptRanges[] = "Accept-Ranges";
  static constexpr char kETag[] = "ETAG";
  static constexpr char kLastModified[] = "Last-Modified";
};
}  // namespace martianlabs::doba::protocol::http11

#endif
