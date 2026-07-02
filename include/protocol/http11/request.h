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

#ifndef martianlabs_doba_protocol_http11_request_h
#define martianlabs_doba_protocol_http11_request_h

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "target.h"
#include "helpers.h"
#include "headers.h"
#include "common/hash_map.h"
#include "protocol/deserialization.h"
#include "checkers/headers/host.h"
#include "checkers/headers/content_length.h"
#include "checkers/headers/transfer_encoding.h"
#include "checkers/headers/connection.h"
#include "checkers/headers/te.h"
#include "checkers/headers/trailer.h"
#include "checkers/headers/expect.h"
#include "checkers/headers/upgrade.h"
#include "checkers/headers/content_type.h"
#include "checkers/headers/content_encoding.h"
#include "checkers/headers/date.h"
#include "checkers/headers/range.h"

namespace martianlabs::doba::protocol::http11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] request                                                     ( class ) |
// +---------------------------------------------------------------------------+
// | This class holds for the http 1.1 request implementation.                 |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class request {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  request() = default;
  request(const request&) = delete;
  request(request&&) noexcept = delete;
  ~request() = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  request& operator=(const request&) = delete;
  request& operator=(request&&) noexcept = delete;
  // +=========================================================================+
  // | [>] GETTERs                                                  ( public ) |
  // +=========================================================================+
  auto get_method() const { return method_; }
  auto get_target() const { return target_; }
  auto get_absolute_path() const { return absolute_path_; }
  auto get_query() const { return query_part_; }
  auto get_header(std::size_t i) const { return headers_[i]; }
  auto get_headers_length() const { return headers_.size(); }
  auto has_body() const { return false; }
  auto get_body_length() const { return 0; }
  // +=========================================================================+
  // | [>] deserialize                                              ( public ) |
  // +=========================================================================+
  static deserialization_result<request> deserialize(std::string_view sv) {
    std::size_t i = 0;
    std::string_view query_tmp;
    std::string_view method_tmp;
    std::string_view absolute_path_tmp;
    target target_tmp = target::kUnknown;
    const std::size_t sv_size = sv.size();
    std::vector<std::pair<std::string, std::string>> headers_tmp;
    // +-----------------------------------------------------------------------+
    // | request-line = method SP request-target SP HTTP-version               |
    // +-----------------------------------------------------------------------+
    // +-----------------------------------------------------------------------+
    // | [method] part!                                                        |
    // +-----------------------------------------------------------------------+
    if (!sv_size) return deserialization_status::kMoreBytesNeeded;
    method_tmp = helpers::consume_token(sv);
    if (method_tmp.empty()) return deserialization_status::kInvalidSource;
    i += method_tmp.size();
    if (i >= sv_size) return deserialization_status::kMoreBytesNeeded;
    if (sv[i++] != ' ') return deserialization_status::kInvalidSource;
    // +-----------------------------------------------------------------------+
    // | [request-target] part!                                                |
    // +-----------------------------------------------------------------------+
    // | request-target = origin-form                                          |
    // |                / absolute-form                                        |
    // |                / authority-form                                       |
    // |                / asterisk-form                                        |
    // +-----------------------------------------------------------------------+
    if (i >= sv_size) return deserialization_status::kMoreBytesNeeded;
    deserialization_status status;
    std::size_t bytes_used = 0;
    if (method_tmp == methods::kConnect) {
      status = helpers::try_to_deserialize_as_authority_form(sv);
    } else if (method_tmp == methods::kOptions && sv.front() == '*') {
      status = helpers::try_to_deserialize_as_asterisk_form(sv, bytes_used);
      if (status == deserialization_status::kSucceeded) {
        target_tmp = target::kAsteriskForm;
      }
    } else {
      status = helpers::try_to_deserialize_as_origin_form(
          sv.substr(i), absolute_path_tmp, query_tmp, bytes_used);
      if (status == deserialization_status::kSucceeded) {
        target_tmp = target::kOriginForm;
      } else {
        status = helpers::try_to_deserialize_as_absolute_form(sv);
      }
    }
    if (status != deserialization_status::kSucceeded) return status;
    i += bytes_used;
    if (i >= sv_size) return deserialization_status::kMoreBytesNeeded;
    if (sv[i++] != ' ') return deserialization_status::kInvalidSource;
    // +-----------------------------------------------------------------------+
    // | [HTTP-version] part!                                                  |
    // +----------------+------------------------------------------------------+
    // | HTTP-version   | HTTP-name "/" DIGIT "." DIGIT                        |
    // | HTTP-name      | %s"HTTP"                                             |
    // +----------------+------------------------------------------------------+
    // +-----------------------------------------------------------------------+
    // | Let's try to decode [HTTP-version]] part!                             |
    // +----------------+------------------------------------------------------+
    // | HTTP-version   | HTTP-name "/" DIGIT "." DIGIT                        |
    // | HTTP-name      | %s"HTTP"                                             |
    // +----------------+------------------------------------------------------+
    if (i + 7 >= sv_size) return deserialization_status::kMoreBytesNeeded;
    if (sv[i + 0] != 'H' || sv[i + 1] != 'T' || sv[i + 2] != 'T' ||
        sv[i + 3] != 'P' || sv[i + 4] != '/' || !helpers::is_digit(sv[i + 5]) ||
        sv[i + 6] != '.' || !helpers::is_digit(sv[i + 7])) {
      return deserialization_status::kInvalidSource;
    }
    if (sv[i + 5] != '1' || sv[i + 7] != '1') {
      return deserialization_status::kInvalidSource;
    }
    i += 8;
    if (i + 1 >= sv_size) return deserialization_status::kMoreBytesNeeded;
    if (sv[i + 0] != '\r' || sv[i + 1] != '\n') {
      return deserialization_status::kInvalidSource;
    }
    i += 2;
    // +-----------------------------------------------------------------------+
    // | [headers] part!                                                       |
    // +-----------------------------------------------------------------------+
    // +-----------------+-----------------------------------------------------+
    // | Rule            | Definition                                          |
    // +-----------------+-----------------------------------------------------+
    // | header-field    | field-name ":" OWS field-value OWS                  |
    // | field-name      | token                                               |
    // | token           | 1*tchar                                             |
    // | tchar           | ALPHA / DIGIT / "!" / "#" / "$" / "%" / "&" / "'" / |
    // |                 | "*" / "+" / "-" / "." / "^" / "_" / "`" / "|" / "~" |
    // | field-value     | *( field-content )                                  |
    // | field-content   | field-vchar [ *( SP / HTAB ) field-vchar ]          |
    // | field-vchar     | VCHAR / obs-text                                    |
    // | OWS             | *( SP / HTAB )                                      |
    // | obs-fold        | CRLF 1*( SP / HTAB ) ; obsolete, not supported      |
    // +-----------------+-----------------------------------------------------+
    bool field_name_decoded = false;
    std::size_t fn_start = i;
    while (i < sv_size) {
      if (!field_name_decoded) {
        if (sv[i] == '\r') {
          if (i != fn_start) return deserialization_status::kInvalidSource;
          if (i + 1 >= sv_size) return deserialization_status::kMoreBytesNeeded;
          if (sv[i + 1] != '\n') return deserialization_status::kInvalidSource;
          i += 2;
          // Let's build the request to return it!
          std::shared_ptr<request> req = std::make_shared<request>();
          req->method_ = std::move(method_tmp);
          req->absolute_path_ = std::move(absolute_path_tmp);
          req->query_part_ = std::move(query_tmp);
          req->headers_ = std::move(headers_tmp);
          req->target_ = target_tmp;
          return deserialization_result(req, i);
        }
        if (sv[i] == '\n') return deserialization_status::kInvalidSource;
        // [field-name] decoding..
        if (sv[i] != ':') {
          if (!helpers::is_token(sv[i++])) {
            return deserialization_status::kInvalidSource;
          }
        } else {
          if (i == fn_start) return deserialization_status::kInvalidSource;
          field_name_decoded = true;
        }
        continue;
      }
      // [field-value] decoding..
      if (i >= sv_size) return deserialization_status::kMoreBytesNeeded;
      if (sv[i++] != ':') return deserialization_status::kInvalidSource;
      std::string_view field_name = sv.substr(fn_start, i - fn_start);
      std::size_t fv_start = i;
      while (i < sv_size) {
        if (sv[i] == '\r') break;
        if (!helpers::is_vchar(sv[i]) && !helpers::is_obs_text(sv[i]) &&
            !helpers::is_ows(sv[i])) {
          return deserialization_status::kInvalidSource;
        }
        i++;
      }
      if (i + 1 >= sv_size) return deserialization_status::kMoreBytesNeeded;
      if (sv[i + 0] != '\r' || sv[i + 1] != '\n') {
        return deserialization_status::kInvalidSource;
      }
      std::string_view field_value = sv.substr(fv_start, i - fv_start);
      helpers::ows_trim(field_value);
      auto const itr_checker = header_checkers_.find(field_name);
      if (itr_checker != header_checkers_.end()) {
        if (!itr_checker->second(field_value)) {
          return deserialization_status::kInvalidSource;
        }
      }
      headers_tmp.emplace_back(field_name, field_value);
      i += 2;
      fn_start = i;
      field_name_decoded = false;
    }
    return deserialization_status::kMoreBytesNeeded;
  }

 private:
  // +=========================================================================+
  // |                      HTTP/1.1 SERVER HEADER CHECKLIST                   |
  // +=========================================================================+
  // +------------------------------------------------------------+------------+
  // | Header                                                     |  Supported |
  // +------------------------------------------------------------+------------+
  // | Host                                                       |     [x]    |
  // | Content-Length                                             |     [x]    |
  // | Transfer-Encoding                                          |     [x]    |
  // | Connection                                                 |     [x]    |
  // | TE                                                         |     [x]    |
  // | Trailer                                                    |     [x]    |
  // | Expect                                                     |     [x]    |
  // | Upgrade                                                    |     [x]    |
  // | Content-Type                                               |     [x]    |
  // | Content-Encoding                                           |     [x]    |
  // | Date                                                       |     [x]    |
  // | Accept                                                     |     [ ]    |
  // | Accept-Encoding                                            |     [ ]    |
  // | Accept-Language                                            |     [ ]    |
  // | Content-Language                                           |     [ ]    |
  // | Content-Location                                           |     [ ]    |
  // | Range                                                      |     [x]    |
  // | Content-Range                                              |     [ ]    |
  // | Accept-Ranges                                              |     [ ]    |
  // | If-Range                                                   |     [ ]    |
  // | ETag                                                       |     [ ]    |
  // | Last-Modified                                              |     [ ]    |
  // | If-Match                                                   |     [ ]    |
  // | If-None-Match                                              |     [ ]    |
  // | If-Modified-Since                                          |     [ ]    |
  // | If-Unmodified-Since                                        |     [ ]    |
  // | Cache-Control                                              |     [ ]    |
  // | Vary                                                       |     [ ]    |
  // | Age                                                        |     [ ]    |
  // | Expires                                                    |     [ ]    |
  // | Pragma                                                     |     [ ]    |
  // | Location                                                   |     [ ]    |
  // | Allow                                                      |     [ ]    |
  // | Retry-After                                                |     [ ]    |
  // | Authorization                                              |     [ ]    |
  // | WWW-Authenticate                                           |     [ ]    |
  // | Authentication-Info                                        |     [ ]    |
  // | Cookie                                                     |     [ ]    |
  // | Set-Cookie                                                 |     [ ]    |
  // | User-Agent                                                 |     [ ]    |
  // | Server                                                     |     [ ]    |
  // | Referer                                                    |     [ ]    |
  // | Max-Forwards                                               |     [ ]    |
  // | From                                                       |     [ ]    |
  // | Accept-Charset                                             |     [ ]    |
  // | Origin                                                     |     [ ]    |
  // | Access-Control-Request-Method                              |     [ ]    |
  // | Access-Control-Request-Headers                             |     [ ]    |
  // | Access-Control-Allow-Origin                                |     [ ]    |
  // | Access-Control-Allow-Methods                               |     [ ]    |
  // | Access-Control-Allow-Headers                               |     [ ]    |
  // | Access-Control-Allow-Credentials                           |     [ ]    |
  // | Access-Control-Expose-Headers                              |     [ ]    |
  // | Access-Control-Max-Age                                     |     [ ]    |
  // | Sec-WebSocket-Key                                          |     [ ]    |
  // | Sec-WebSocket-Accept                                       |     [ ]    |
  // | Sec-WebSocket-Version                                      |     [ ]    |
  // | Sec-WebSocket-Protocol                                     |     [ ]    |
  // | Sec-WebSocket-Extensions                                   |     [ ]    |
  // | Via                                                        |     [ ]    |
  // | Forwarded                                                  |     [ ]    |
  // | X-Forwarded-For                                            |     [ ]    |
  // | X-Forwarded-Host                                           |     [ ]    |
  // | X-Forwarded-Proto                                          |     [ ]    |
  // | Keep-Alive                                                 |     [ ]    |
  // | Proxy-Connection                                           |     [ ]    |
  // +------------------------------------------------------------+------------+
  // +=========================================================================+
  // | [>] TYPEs                                                   ( private ) |
  // +=========================================================================+
  using header_check_delegate = std::function<bool(std::string_view)>;
  // +=========================================================================+
  // | [>] CONSTANTs                                               ( private ) |
  // +=========================================================================+
  static const inline common::hash_map<std::string_view, header_check_delegate>
      header_checkers_ = {
          {"Host", checkers::headers::host::check},
          {"Content-Length", checkers::headers::content_length::check},
          {"Transfer-Encoding", checkers::headers::transfer_encoding::check},
          {"Connection", checkers::headers::connection::check},
          {"TE", checkers::headers::te::check},
          {"Trailer", checkers::headers::trailer::check},
          {"Expect", checkers::headers::expect::check},
          {"Upgrade", checkers::headers::upgrade::check},
          {"Content-Type", checkers::headers::content_type::check},
          {"Content-Encoding", checkers::headers::content_encoding::check},
          {"Date", checkers::headers::date::check},
          {"Range", checkers::headers::range::check},
  };
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  std::string method_;
  std::string query_part_;
  std::string absolute_path_;
  target target_ = target::kUnknown;
  std::vector<std::pair<std::string, std::string>> headers_;
  std::vector<std::pair<std::string, std::string>> query_parameters_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
