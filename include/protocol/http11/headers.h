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

#include <algorithm>
#include <string_view>

#include "constants.h"
#include "hash_map.h"

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
  inline void prepare(char* buffer, std::size_t size, std::size_t used) {
    buf_ = buffer;
    buf_sz_ = size;
    cur_ = used;
  }
  inline void reset() {
    buf_ = nullptr;
    buf_sz_ = 0;
    cur_ = 0;
  }
  inline void add(std::string_view k, std::string_view v) {
    std::size_t k_sz = k.size(), v_sz = v.size();
    std::size_t entry_length = k_sz + v_sz + 3;
    if ((buf_sz_ - cur_) > (entry_length + 2)) {
      if (!k.size()) return;
      auto initial_cur = cur_;
      for (const char& c : k) {
        if (!helpers::is_token(c)) {
          cur_ = initial_cur;
          return;
        }
        buf_[cur_++] = helpers::tolower_ascii(c);
      }
      buf_[cur_++] = constants::character::kColon;
      for (const char& c : v) {
        if (!(helpers::is_vchar(c) || helpers::is_obs_text(c) ||
              c == constants::character::kSpace ||
              c == constants::character::kHTab)) {
          return;
        }
      }
      memcpy(&buf_[cur_], v.data(), v_sz);
      cur_ += v_sz;
      buf_[cur_++] = constants::character::kCr;
      buf_[cur_++] = constants::character::kLf;
    }
  }
  template <typename T>
    requires std::is_arithmetic_v<T>
  inline void add(std::string_view k, const T& v) {
    add(k, std::to_string(v));
  }
  inline hash_map<std::string_view, std::string_view> get_all() const {
    bool searching_for_key = true;
    std::size_t i = 0, j = 0, k_start = i, v_start = i;
    hash_map<std::string_view, std::string_view> out;
    while (i < cur_) {
      switch (buf_[i]) {
        case constants::character::kColon:
          if (searching_for_key) {
            searching_for_key = false;
            v_start = i + 1;
          }
          break;
        case constants::character::kCr:
          out.insert(std::make_pair(
              std::string_view(&buf_[k_start], (v_start - 1) - k_start),
              helpers::ows_ltrim(helpers::ows_rtrim(
                  std::string_view(&buf_[v_start], i - v_start)))));
          searching_for_key = true;
          k_start = i + 2;
          j = 0;
          i++;
          break;
        default:
          break;
      }
      i++;
    }
    return out;
  }
  inline std::optional<std::string_view> get(std::string_view k) const {
    bool matched = true;
    bool searching_for_key = true;
    std::size_t i = 0, j = 0, v_start = i, k_sz = k.size();
    while (i < cur_) {
      switch (buf_[i]) {
        case constants::character::kColon:
          if (searching_for_key) {
            searching_for_key = false;
            v_start = i + 1;
          }
          break;
        case constants::character::kCr:
          if (matched && j == k_sz) {
            return helpers::ows_ltrim(helpers::ows_rtrim(
                std::string_view(&buf_[v_start], i - v_start)));
          }
          searching_for_key = true;
          matched = true;
          j = 0;
          i++;
          break;
        default:
          if (searching_for_key) {
            if (matched) {
              if (j < k_sz) {
                matched = buf_[i] == helpers::tolower_ascii(k[j++]);
              } else {
                matched = false;
              }
            }
          }
          break;
      }
      i++;
    }
    return std::nullopt;
  }
  inline void remove(std::string_view k) {
    bool matched = true;
    bool searching_for_key = true;
    std::size_t i = 0, j = 0, k_start = i, k_sz = k.size();
    while (i < cur_) {
      switch (buf_[i]) {
        case constants::character::kColon:
          if (searching_for_key) {
            searching_for_key = false;
          }
          break;
        case constants::character::kCr:
          if (matched && j == k_sz) {
            auto const off = i + 2;
            std::memmove(&buf_[k_start], &buf_[off], cur_ - off);
            cur_ -= (off - k_start);
            std::memset(&buf_[cur_], 0, buf_sz_ - cur_);
            return;
          }
          searching_for_key = true;
          matched = true;
          k_start = i + 2;
          j = 0;
          i++;
          break;
        default:
          if (searching_for_key) {
            if (matched) {
              if (j < k_sz) {
                matched = buf_[i] == helpers::tolower_ascii(k[j++]);
              } else {
                matched = false;
              }
            }
          }
          break;
      }
      i++;
    }
  }
  inline std::size_t length() const { return cur_; }
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
  char* buf_ = nullptr;
  std::size_t buf_sz_ = 0;
  std::size_t cur_ = 0;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
