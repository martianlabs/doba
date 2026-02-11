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

#ifndef martianlabs_doba_protocol_http11_decoder_h
#define martianlabs_doba_protocol_http11_decoder_h

#include "common/deserialize_result.h"
#include "internal/headers_markers.h"
#include "protocol/http11/checkers/host.h"
#include "protocol/http11/checkers/date.h"
#include "protocol/http11/checkers/connection.h"
#include "protocol/http11/checkers/content_length.h"
#include "protocol/http11/checkers/transfer_encoding.h"

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// decoder                                                             ( class )
// -----------------------------------------------------------------------------
// This class holds for the http 1.1 requests decoder.
// -----------------------------------------------------------------------------
// Template parameters:
//    RQty - request being used.
//    RSty - response being used.
//    BFsz - buffer size.
// =============================================================================
template <typename RQty, typename RSty, std::size_t BFsz>
class decoder {
 public:
  // ---------------------------------------------------------------------------
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  decoder() { setup_header_checkers_functions(); }
  decoder(const decoder&) = delete;
  decoder(decoder&&) noexcept = delete;
  // ---------------------------------------------------------------------------
  // OPERATORs                                                        ( public )
  //
  decoder& operator=(const decoder&) = delete;
  decoder& operator=(decoder&&) noexcept = delete;
  // ---------------------------------------------------------------------------
  // METHODs                                                          ( public )
  //
  inline bool add(const char* buffer, std::size_t length) noexcept {
    if (length > (BFsz - length_)) return false;
    std::memcpy(&buffer_[length_], buffer, length);
    length_ += length;
    return true;
  }
  inline common::deserialize_result deserialize(RQty*& out) noexcept {
    common::deserialize_result res = check_request_line();
    if (res == common::deserialize_result::kSucceeded) {
      if ((res = check_headers()) == common::deserialize_result::kSucceeded) {
        out = RQty::from(buffer_, off_);
        out->set_method(method_);
        out->set_target(target_);
        out->set_absolute_path(absolute_path_);
        out->set_query_part(query_part_);
        out->set_headers(headers_, headers_len_);
      }
    }
    if (off_ < length_) {
      std::memmove(buffer_, &buffer_[off_], length_ - off_);
    }
    length_ -= off_;
    off_ = 0;
    target_ = target::kUnknown;
    query_part_ = {0, 0};
    absolute_path_ = {0, 0};
    method_ = {0, 0};
    headers_len_ = 0;
    return res;
  }

 private:
  // ---------------------------------------------------------------------------
  // USINGs                                                          ( private )
  //
  using on_header_check_delegate = std::function<bool(std::string_view)>;
  // ---------------------------------------------------------------------------
  // METHODs                                                        ( private  )
  //
  // +=========================================================================+
  // |                                                            request-line |
  // +=========================================================================+
  // | request-line = method SP request-target SP HTTP-version                 |
  // +-------------------------------------------------------------------------+
  // | source: https://datatracker.ietf.org/doc/html/rfc9110                   |
  // +-------------------------------------------------------------------------+
  inline common::deserialize_result check_request_line() {
    std::size_t off = off_, tmp = 0;
    static constexpr std::size_t kHttpVersionLen = 8;
    // +-----------------------------------------------------------------------+
    // |                                                                method |
    // +---------+-------------------------------------------------------------+
    // | Field   | Definition                                                  |
    // +---------+-------------------------------------------------------------+
    // | method  | token                                                       |
    // | token   | 1*tchar                                                     |
    // | tchar   | "!" / "#" / "$" / "%" / "&" / "'" / "*" /                   |
    // |         | "+" / "-" / "." / "^" / "_" / "`" / "|" / "~"               |
    // |         | / DIGIT / ALPHA                                             |
    // +---------+-------------------------------------------------------------+
    // | source: https://datatracker.ietf.org/doc/html/rfc9110                 |
    // +----------------+------------------------------------------------------+
    char* sp = nullptr;
    while (off < length_) {
      if (buffer_[off] == constants::character::kSpace) {
        sp = &buffer_[off++];
        break;
      }
      if (!helpers::is_token(buffer_[off])) {
        return common::deserialize_result::kInvalidSource;
      }
      off++;
    }
    if (!sp) return common::deserialize_result::kMoreBytesNeeded;
    method_.start = 0;
    method_.length = sp - buffer_;
    std::string_view method(buffer_, sp - buffer_);
    // +-----------------------------------------------------------------------+
    // | HTTP/1.1: request-target (RFC 9112 §2.2) – Valid Forms                |
    // +-----------------------------------------------------------------------+
    // |                                                                       |
    // | request-target = origin-form                                          |
    // |                / absolute-form                                        |
    // |                / authority-form                                       |
    // |                / asterisk-form                                        |
    // |                                                                       |
    // |-----------------------------------------------------------------------|
    // | FORM 1: origin-form (most common)                                     |
    // |    origin-form = absolute-path [ "?" query ]                          |
    // |-----------------------------------------------------------------------|
    // | FORM 2: absolute-form (used via HTTP proxy)                           |
    // |    URI = scheme ":" hier-part [ "?" query ] [ "#" fragment ]          |
    // |-----------------------------------------------------------------------|
    // | FORM 3: authority-form (used with CONNECT)                            |
    // |    authority-form = host ":" port                                     |
    // |-----------------------------------------------------------------------|
    // | FORM 4: asterisk-form (used with OPTIONS)                             |
    // |    asterisk-form = "*"                                                |
    // +=======================================================================+
    if (method == constants::method::kConnect) {
      // +---------------------------------------------------------------------+
      // |                                                      authority-form |
      // +----------------+----------------------------------------------------+
      // | Rule           | Definition                                         |
      // +----------------+----------------------------------------------------+
      // | authority-form | authority                                          |
      // |                | (RFC 9112 §2.2)                                    |
      // | authority      | [ userinfo "@" ] host [ ":" port ]                 |
      // |                | (RFC 3986 §3.2)                                    |
      // |                |  userinfo is forbidden in HTTP `authority-form`    |
      // | host           | IP-literal / IPv4address / reg-name                |
      // |                | (RFC 3986 §3.2.2)                                  |
      // | port           | *DIGIT                                             |
      // |                | (RFC 3986 §3.2.3)                                  |
      // | userinfo       | *( unreserved / pct-encoded / sub-delims / ":" )   |
      // |                | (RFC 3986 §3.2.1) : forbidden in HTTP              |
      // | unreserved     | ALPHA / DIGIT / "-" / "." / "_" / "~"              |
      // |                | (RFC 3986 §2.3)                                    |
      // | pct-encoded    | "%" HEXDIG HEXDIG                                  |
      // |                | (RFC 3986 §2.1)                                    |
      // | sub-delims     | "!" / "$" / "&" / "'" / "(" / ")"                  |
      // |                | "*" / "+" / "," / ";" / "="                        |
      // |                | (RFC 3986 §2.2)                                    |
      // | IP-literal     | "[" ( IPv6address / IPvFuture ) "]"                |
      // |                | (RFC 3986 §3.2.2)                                  |
      // | IPv4address    | dec-octet "." dec-octet "." dec-octet "." dec-octet|
      // |                | (RFC 3986 §3.2.2)                                  |
      // | dec-octet      | DIGIT                 ; 0-9                        |
      // |                | / %x31-39 DIGIT       ; 10-99                      |
      // |                | / "1" 2DIGIT          ; 100-199                    |
      // |                | / "2" %x30-34 DIGIT   ; 200-249                    |
      // |                | / "25" %x30-35        ; 250-255                    |
      // |                | (RFC 3986 §3.2.2)                                  |
      // | reg-name       | *( unreserved / pct-encoded / sub-delims )         |
      // |                | (RFC 3986 §3.2.2)                                  |
      // +----------------+----------------------------------------------------+
      // [to-do] -> add support for this!
    } else if (method == constants::method::kOptions) {
      // +---------------------------------------------------------------------+
      // |                                                       asterisk-form |
      // +----------------+----------------------------------------------------+
      // | Field          | Definition                                         |
      // +----------------+----------------------------------------------------+
      // | asterisk-form  | "*"                                                |
      // +----------------+----------------------------------------------------+
      // [to-do] -> add support for this!
    } else if (decode_absolute_path(off, tmp)) {
      // +---------------------------------------------------------------------+
      // |                                                         origin-form |
      // +----------------+----------------------------------------------------+
      // | Field          | Definition                                         |
      // +----------------+----------------------------------------------------+
      // | origin-form    | absolute-path [ "?" query ]                        |
      // +----------------+----------------------------------------------------+
      target_ = target::kOriginForm;
      if ((off += tmp) >= length_) {
        return common::deserialize_result::kMoreBytesNeeded;
      }
      if (buffer_[off] == constants::character::kQuestion) {
        common::deserialize_result res = decode_query_part(++off, tmp);
        if (res != common::deserialize_result::kSucceeded) {
          return res;
        }
        off += tmp;
      }
    } else {
      // +---------------------------------------------------------------------+
      // |                                                       absolute-form |
      // +----------------+----------------------------------------------------+
      // | Field          | Definition                                         |
      // +----------------+----------------------------------------------------+
      // | absolute-form  | URI                                                |
      // +----------------+----------------------------------------------------+
      // [to-do] -> add support for this!
    }
    // +-----------------------------------------------------------------------+
    // |                                                          http-version |
    // +----------------+------------------------------------------------------+
    // | HTTP-version   | HTTP-name "/" DIGIT "." DIGIT                        |
    // | HTTP-name      | %s"HTTP"                                             |
    // +----------------+------------------------------------------------------+
    // | source: https://datatracker.ietf.org/doc/html/rfc9110 |               |
    // +-----------------------------------------------------------------------+
    if (off >= length_) return common::deserialize_result::kMoreBytesNeeded;
    if (buffer_[off++] != constants::character::kSpace) {
      return common::deserialize_result::kInvalidSource;
    }
    if (length_ - off < kHttpVersionLen) {
      return common::deserialize_result::kMoreBytesNeeded;
    }
    if (buffer_[off + 0] != constants::character::kHUpperCase ||
        buffer_[off + 1] != constants::character::kTUpperCase ||
        buffer_[off + 2] != constants::character::kTUpperCase ||
        buffer_[off + 3] != constants::character::kPUpperCase ||
        buffer_[off + 4] != constants::character::kSlash ||
        !helpers::is_digit(buffer_[off + 5]) ||
        buffer_[off + 6] != constants::character::kDot ||
        !helpers::is_digit(buffer_[off + 7])) {
      return common::deserialize_result::kInvalidSource;
    }
    off += kHttpVersionLen;
    if (length_ - off < 2) return common::deserialize_result::kMoreBytesNeeded;
    if (buffer_[off + 0] != constants::character::kCr ||
        buffer_[off + 1] != constants::character::kLf) {
      return common::deserialize_result::kInvalidSource;
    }
    off_ = off += 2;
    return common::deserialize_result::kSucceeded;
  }
  // +=========================================================================+
  // |                                                                 headers |
  // +=========================================================================+
  // +-----------------+-------------------------------------------------------+
  // | Rule            | Definition                                            |
  // +-----------------+-------------------------------------------------------+
  // | header-field    | field-name ":" OWS field-value OWS                    |
  // | field-name      | token                                                 |
  // | token           | 1*tchar                                               |
  // | tchar           | ALPHA / DIGIT / "!" / "#" / "$" / "%" / "&" / "'" /   |
  // |                 | "*" / "+" / "-" / "." / "^" / "_" / "`" / "|" / "~"   |
  // | field-value     | *( field-content )                                    |
  // | field-content   | field-vchar [ *( SP / HTAB ) field-vchar ]            |
  // | field-vchar     | VCHAR / obs-text                                      |
  // | OWS             | *( SP / HTAB )                                        |
  // | obs-fold        | CRLF 1*( SP / HTAB ) ; obsolete, not supported        |
  // +-----------------+-------------------------------------------------------+
  // | source: https://datatracker.ietf.org/doc/html/rfc9110                   |
  // +-------------------------------------------------------------------------+
  inline common::deserialize_result check_headers() {
    std::size_t off = off_;
    bool end_of_headers_found = false;
    while (off < length_) {
      // [end-of-headers] check..
      if (buffer_[off] == constants::character::kCr) {
        if (off + 1 >= length_) {
          return common::deserialize_result::kMoreBytesNeeded;
        }
        if (buffer_[off + 1] != constants::character::kLf) {
          return common::deserialize_result::kInvalidSource;
        }
        end_of_headers_found = true;
        off += 2;
        break;
      }
      // [field-name] decoding..
      char* col = nullptr;
      char* fns = &buffer_[off];
      while (off < length_) {
        if (buffer_[off] == constants::character::kColon) {
          col = &buffer_[off++];
          break;
        }
        if (!helpers::is_token(buffer_[off])) {
          return common::deserialize_result::kInvalidSource;
        }
        buffer_[off] = helpers::tolower_ascii(buffer_[off]);
        off++;
      }
      if (!col) {
        return common::deserialize_result::kMoreBytesNeeded;
      }
      if (col == fns) {
        // empty field-name not allowed!
        return common::deserialize_result::kInvalidSource;
      }
      std::size_t fnl = col - fns;
      // [field-value] decoding..
      char* crlf = nullptr;
      char* fvs = &buffer_[off];
      while (off < length_) {
        if (buffer_[off] == constants::character::kCr) {
          if (off + 1 >= length_) {
            return common::deserialize_result::kMoreBytesNeeded;
          }
          if (buffer_[off + 1] != constants::character::kLf) {
            return common::deserialize_result::kInvalidSource;
          }
          crlf = &buffer_[off];
          break;
        }
        if (!helpers::is_vchar(buffer_[off]) &&
            !helpers::is_obs_text(buffer_[off]) &&
            !helpers::is_ows(buffer_[off])) {
          return common::deserialize_result::kInvalidSource;
        }
        off++;
      }
      if (!crlf) {
        return common::deserialize_result::kMoreBytesNeeded;
      }
      // let's remove external OWS around the field-value..
      while (fvs < crlf && helpers::is_ows(*fvs)) fvs++;
      while ((crlf - 1) >= fvs && helpers::is_ows(*(crlf - 1))) crlf--;
      // space left for next header?
      if (headers_len_ >= constants::limits::kDefaultRequestMaxHeaders) {
        return common::deserialize_result::kInvalidSource;
      }
      // let's add current header marker to internal structures..
      headers_.data_[headers_len_].name.start = fns - buffer_;
      headers_.data_[headers_len_].name.length = fnl;
      headers_.data_[headers_len_].value.start = fvs - buffer_;
      headers_.data_[headers_len_].value.length = crlf - fvs;
      // let's check current header value..
      auto itr_header_checker = headers_fns_.find(std::string_view(fns, fnl));
      if (itr_header_checker != headers_fns_.end()) {
        if (!itr_header_checker->second(std::string_view(fvs, crlf - fvs))) {
          return common::deserialize_result::kInvalidSource;
        }
      }
      headers_len_++;
      off += 2;
    }
    if (end_of_headers_found) {
      off_ = off;
      return common::deserialize_result::kSucceeded;
    }
    return common::deserialize_result::kMoreBytesNeeded;
  }
  // +=========================================================================+
  // |                                                          abosolute-path |
  // +=========================================================================+
  // +----------------+--------------------------------------------------------+
  // | Field          | Definition                                             |
  // +----------------+--------------------------------------------------------+
  // | abosulte-path  | 1*( "/" segment )                                      |
  // | segment        | *pchar                                                 |
  // | pchar          | unreserved / pct-encoded / sub-delims / ":" / "@"      |
  // | unreserved     | ALPHA / DIGIT / "-" / "." / "_" / "~"                  |
  // | pct-encoded    | "%" HEXDIG HEXDIG                                      |
  // | sub-delims     | "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" /        |
  // |                | "," / ";" / "="                                        |
  // +----------------+--------------------------------------------------------+
  // | source: https://datatracker.ietf.org/doc/html/rfc9110                   |
  // +----------------+--------------------------------------------------------+
  inline bool decode_absolute_path(std::size_t off, std::size_t& used) {
    std::size_t i = 0;
    char* p = &buffer_[off];
    if (off >= length_ || p[i++] != constants::character::kSlash) return false;
    const std::size_t effective_len = length_ - off;
    while (i < length_) {
      uint8_t c = static_cast<uint8_t>(p[i]);
      if (c == constants::character::kSpace ||
          c == constants::character::kQuestion) {
        break;
      }
      if (c == constants::character::kPercent) {
        if (i + 2 >= length_) return false;
        if (!helpers::is_hex_digit(static_cast<uint8_t>(p[i + 1])) ||
            !helpers::is_hex_digit(static_cast<uint8_t>(p[i + 2]))) {
          return false;
        }
        i += 3;
        continue;
      }
      if (!helpers::is_pchar(c) && c != constants::character::kSlash) {
        return false;
      }
      i++;
    }
    absolute_path_.start = p - buffer_;
    absolute_path_.length = used = i;
    return true;
  }
  // +=========================================================================+
  // |                                                              query-part |
  // +=========================================================================+
  // +----------------+--------------------------------------------------------+
  // | Field          | Definition                                             |
  // +----------------+--------------------------------------------------------+
  // | query          | *( pchar / "/" / "?" )                                 |
  // | pchar          | unreserved / pct-encoded / sub-delims / ":" / "@"      |
  // | unreserved     | ALPHA / DIGIT / "-" / "." / "_" / "~"                  |
  // | pct-encoded    | "%" HEXDIG HEXDIG                                      |
  // | sub-delims     | "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" /        |
  // |                | "," / ";" / "="                                        |
  // +----------------+--------------------------------------------------------+
  // | source: https://datatracker.ietf.org/doc/html/rfc9110                   |
  // +----------------+--------------------------------------------------------+
  inline common::deserialize_result decode_query_part(std::size_t off,
                                                      std::size_t& used) {
    if (off >= length_) return common::deserialize_result::kMoreBytesNeeded;
    char* p = &buffer_[off];
    std::size_t i = 0;
    const std::size_t effective_length = length_ - off;
    while (i < effective_length) {
      uint8_t c = static_cast<uint8_t>(p[i]);
      if (c == constants::character::kSpace) break;
      if (c == constants::character::kPercent) {
        if (i + 2 >= effective_length) {
          return common::deserialize_result::kMoreBytesNeeded;
        }
        if (!helpers::is_hex_digit(static_cast<uint8_t>(p[i + 1])) ||
            !helpers::is_hex_digit(static_cast<uint8_t>(p[i + 2]))) {
          return common::deserialize_result::kInvalidSource;
        }
        i += 3;
        continue;
      }
      if (!helpers::is_pchar(c) && c != constants::character::kSlash &&
          c != constants::character::kQuestion) {
        return common::deserialize_result::kInvalidSource;
      }
      i++;
    }
    query_part_.start = p - buffer_;
    query_part_.length = used = i;
    return common::deserialize_result::kSucceeded;
  }
  void setup_header_checkers_functions() {
    // +-----------------------------------------------------------------------+
    // |                                                       header-checkers |
    // +-----------------------------------------------------------------------+
    // | [x] Host                                                              |
    // | [x] Content-Length                                                    |
    // | [x] Connection                                                        |
    // | [x] Date                                                              |
    // | [ ] Transfer-Encoding                                                 |
    // | [ ] TE                                                                |
    // | [ ] Content-Type                                                      |
    // | [ ] Accept                                                            |
    // | [ ] Allow                                                             |
    // | [ ] Server                                                            |
    // | [ ] User-Agent                                                        |
    // | [ ] Expect                                                            |
    // | [ ] Upgrade                                                           |
    // | [ ] Range                                                             |
    // | [ ] If-Modified-Since                                                 |
    // | [ ] Cache-Control                                                     |
    // | [ ] ETag                                                              |
    // | [ ] Location                                                          |
    // | [ ] Access-Control-*                                                  |
    // | [ ] Trailer                                                           |
    // | [ ] Vary                                                              |
    // +-----------------------------------------------------------------------+
    headers_fns_[headers::kHost] = checkers::host_fn;
    headers_fns_[headers::kContentLength] = checkers::content_length_fn;
    headers_fns_[headers::kConnection] = checkers::connection_fn;
    headers_fns_[headers::kDate] = checkers::date_fn;
    headers_fns_[headers::kTransferEncoding] = checkers::transfer_encoding_fn;
  }
  // ---------------------------------------------------------------------------
  // ATTRIBUTEs                                                      ( private )
  //
  char buffer_[BFsz]{};
  std::size_t off_ = 0;
  std::size_t length_ = 0;
  target target_ = target::kUnknown;
  internal::marker query_part_{0, 0};
  internal::marker absolute_path_{0, 0};
  internal::marker method_{0, 0};
  internal::headers_markers headers_;
  std::size_t headers_len_ = 0;
  std::unordered_map<std::string_view, on_header_check_delegate> headers_fns_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
