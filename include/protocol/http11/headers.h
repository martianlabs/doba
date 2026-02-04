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
//        --- martianLabs Anti-AI Usage and Model-Training Addendum ---
//
// TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION
//
// Copyright 2025 martianLabs
//
// Except as otherwise stated in this Addendum, this software is licensed
// under the Apache License, Version 2.0 (the "License"); you may not use
// this file except in compliance with the License.
//
// The following additional terms are hereby added to the Apache License for
// the purpose of restricting the use of this software by Artificial
// Intelligence systems, machine learning models, data-scraping bots, and
// automated systems.
//
// 1.  MACHINE LEARNING AND AI RESTRICTIONS
//     1.1. No entity, organization, or individual may use this software,
//          its source code, object code, or any derivative work for the
//          purpose of training, fine-tuning, evaluating, or improving any
//          machine learning model, artificial intelligence system, large
//          language model, or similar automated system.
//     1.2. No automated system may copy, parse, analyze, index, or
//          otherwise process this software for any AI-related purpose.
//     1.3. Use of this software as input, prompt material, reference
//          material, or evaluation data for AI systems is expressly
//          prohibited.
//
// 2.  SCRAPING AND AUTOMATED ACCESS RESTRICTIONS
//     2.1. No automated crawler, training pipeline, or data-extraction
//          system may collect, store, or incorporate any portion of this
//          software in any dataset used for machine learning or AI
//          training.
//     2.2. Any automated access must comply with this License and with
//          applicable copyright law.
//
// 3.  PROHIBITION ON DERIVATIVE DATASETS
//     3.1. You may not create datasets, corpora, embeddings, vector
//          stores, or similar derivative data intended for use by
//          automated systems, AI models, or machine learning algorithms.
//
// 4.  NO WAIVER OF RIGHTS
//     4.1. These restrictions apply in addition to, and do not limit,
//          the rights and protections provided to the copyright holder
//          under the Apache License Version 2.0 and applicable law.
//
// 5.  ACCEPTANCE
//     5.1. Any use of this software constitutes acceptance of both the
//          Apache License Version 2.0 and this Anti-AI Addendum.
//
// You may obtain a copy of the Apache License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.  See the License for the specific language governing
// permissions and limitations under the Apache License Version 2.0.

#ifndef martianlabs_doba_protocol_http11_headers_h
#define martianlabs_doba_protocol_http11_headers_h

#include "constants.h"
#include "header.h"

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// headers                                                             ( class )
// -----------------------------------------------------------------------------
// This class holds for the http 1.1 headers specification.
// -----------------------------------------------------------------------------
// =============================================================================
class headers {
 public:
  // ---------------------------------------------------------------------------
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  headers() = default;
  headers(const headers&) = delete;
  headers(headers&&) noexcept = delete;
  // ---------------------------------------------------------------------------
  // OPERATORs                                                        ( public )
  //
  headers& operator=(const headers&) = delete;
  headers& operator=(headers&&) noexcept = delete;
  // ---------------------------------------------------------------------------
  // METHODs                                                          ( public )
  //
  inline void add(std::string_view name, std::string_view value) {
    if (length_ >= constants::limits::kDefaultRequestMaxHeaders) {
      throw std::out_of_range("maximum number of headers reached!");
    }
    data_[length_].name = name;
    data_[length_].value = value;
    length_++;
  }
  inline header at(std::size_t i) const {
    if (i >= length_) {
      throw std::out_of_range("out of bounds!");
    }
    return data_[i];
  }
  inline std::size_t length() const { return length_; }
  // ---------------------------------------------------------------------------
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
  // ---------------------------------------------------------------------------
  // ATTRIBUTEs                                                      ( private )
  //
  std::array<header, constants::limits::kDefaultRequestMaxHeaders> data_;
  std::size_t length_ = 0;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
