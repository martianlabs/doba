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

#ifndef martianlabs_doba_protocol_http11_headers_h
#define martianlabs_doba_protocol_http11_headers_h

#include <string_view>

#include "constants.h"

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// headers                                                             ( class )
// -----------------------------------------------------------------------------
// This class holds for the http 1.1 headers implementation.
// -----------------------------------------------------------------------------
// =============================================================================
class headers {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  headers() = default;
  headers(const headers&) = delete;
  headers(headers&&) noexcept = delete;
  ~headers() = default;
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  headers& operator=(const headers&) = delete;
  headers& operator=(headers&&) noexcept = delete;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  inline void prepare(char* buffer, const std::size_t& size) {
    buffer_ = buffer;
    size_ = size;
  }
  inline void reset() {
    buffer_ = nullptr;
    size_ = 0;
    length_ = 0;
  }
  inline void add(const std::string_view& k, const std::string_view& v) {
    std::size_t entry_length = k.length() + v.length() + 3;
    if ((size_ - length_) > (entry_length + 2)) {
      memcpy(&buffer_[length_], k.data(), k.length());
      length_ += k.length();
      buffer_[length_++] = constants::character::kColon;
      memcpy(&buffer_[length_], v.data(), v.length());
      length_ += v.length();
      buffer_[length_++] = constants::character::kCr;
      buffer_[length_++] = constants::character::kLf;
    }
  }
  template <typename T>
    requires std::is_arithmetic_v<T>
  inline void add(const std::string_view& k, const T& v) {
    add(k, std::to_string(v));
  }
  inline std::size_t length() const { return length_; }
  // ___________________________________________________________________________
  // CONSTANTs                                                        ( public )
  //
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
  static constexpr char kConnection[] = "connection";
  static constexpr char kDate[] = "date";
  static constexpr char kVia[] = "via";
  static constexpr char kCacheControl[] = "cache-control";
  static constexpr char kPragma[] = "pragma";
  static constexpr char kWarning[] = "warning";
  static constexpr char kHost[] = "host";
  static constexpr char kUserAgent[] = "user-agent";
  static constexpr char kAccept[] = "accept";
  static constexpr char kAcceptEncoding[] = "accept-encoding";
  static constexpr char kAcceptLanguage[] = "accept-language";
  static constexpr char kAuthorization[] = "authorization";
  static constexpr char kExpect[] = "expect";
  static constexpr char kFrom[] = "from";
  static constexpr char kIfMatch[] = "if-match";
  static constexpr char kIfNoneMatch[] = "if-none-match";
  static constexpr char kIfModifiedSince[] = "if-modified-since";
  static constexpr char kIfUnmodifiedSince[] = "if-unmodified-since";
  static constexpr char kRange[] = "range";
  static constexpr char kReferer[] = "referer";
  static constexpr char kTe[] = "te";
  static constexpr char kUpgrade[] = "upgrade";
  static constexpr char kCookie[] = "cookie";
  static constexpr char kLocation[] = "location";
  static constexpr char kServer[] = "server";
  static constexpr char kVary[] = "vary";
  static constexpr char kWwwAuthenticate[] = "www-authenticate";
  static constexpr char kContentLength[] = "content-length";
  static constexpr char kContentType[] = "content-type";
  static constexpr char kContentEncoding[] = "content-encoding";
  static constexpr char kContentLanguage[] = "content-language";
  static constexpr char kContentRange[] = "content-range";
  static constexpr char kTrailer[] = "trailer";
  static constexpr char kTransferEncoding[] = "transfer-encoding";
  static constexpr char kAllow[] = "allow";
  static constexpr char kRetryAfter[] = "retry-after";
  static constexpr char kAcceptRanges[] = "accept-ranges";
  static constexpr char kETag[] = "etag";
  static constexpr char kLastModified[] = "last-modified";

 private:
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  char* buffer_ = nullptr;
  std::size_t size_ = 0;
  std::size_t length_ = 0;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
