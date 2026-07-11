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
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "headers/accept.h"
#include "headers/accept_charset.h"
#include "headers/accept_encoding.h"
#include "headers/accept_language.h"
#include "headers/accept_ranges.h"
#include "headers/access_control_allow_credentials.h"
#include "headers/access_control_allow_headers.h"
#include "headers/access_control_allow_methods.h"
#include "headers/access_control_allow_origin.h"
#include "headers/access_control_expose_headers.h"
#include "headers/access_control_max_age.h"
#include "headers/access_control_request_headers.h"
#include "headers/access_control_request_method.h"
#include "headers/age.h"
#include "headers/allow.h"
#include "headers/authentication_info.h"
#include "headers/authorization.h"
#include "headers/cache_control.h"
#include "headers/connection.h"
#include "headers/content_encoding.h"
#include "headers/content_language.h"
#include "headers/content_length.h"
#include "headers/content_location.h"
#include "headers/content_range.h"
#include "headers/content_type.h"
#include "headers/cookie.h"
#include "headers/date.h"
#include "headers/etag.h"
#include "headers/expect.h"
#include "headers/expires.h"
#include "headers/forwarded.h"
#include "headers/from.h"
#include "headers/host.h"
#include "headers/if_match.h"
#include "headers/if_modified_since.h"
#include "headers/if_none_match.h"
#include "headers/if_range.h"
#include "headers/if_unmodified_since.h"
#include "headers/keep_alive.h"
#include "headers/last_modified.h"
#include "headers/location.h"
#include "headers/max_forwards.h"
#include "headers/origin.h"
#include "headers/pragma.h"
#include "headers/proxy_connection.h"
#include "headers/range.h"
#include "headers/referer.h"
#include "headers/retry_after.h"
#include "headers/sec_websocket_accept.h"
#include "headers/sec_websocket_extensions.h"
#include "headers/sec_websocket_key.h"
#include "headers/sec_websocket_protocol.h"
#include "headers/sec_websocket_version.h"
#include "headers/server.h"
#include "headers/set_cookie.h"
#include "headers/te.h"
#include "headers/trailer.h"
#include "headers/transfer_encoding.h"
#include "headers/upgrade.h"
#include "headers/user_agent.h"
#include "headers/vary.h"
#include "headers/via.h"
#include "headers/www_authenticate.h"
#include "headers/x_forwarded_for.h"
#include "headers/x_forwarded_host.h"
#include "headers/x_forwarded_proto.h"
#include "common/hash_map.h"
#include "header_names.h"
#include "helpers.h"
#include "protocol/http11/body_deserializer.h"
#include "protocol/deserialization.h"
#include "protocol/http11/connection.h"
#include "protocol/http11/context.h"
#include "protocol/http11/headers/connection.h"
#include "protocol/http11/headers/content_length.h"
#include "protocol/http11/headers/expect.h"
#include "protocol/http11/headers/forwarded.h"
#include "protocol/http11/headers/host.h"
#include "protocol/http11/headers/max_forwards.h"
#include "protocol/http11/headers/te.h"
#include "protocol/http11/headers/trailer.h"
#include "protocol/http11/headers/transfer_encoding.h"
#include "protocol/http11/headers/upgrade.h"
#include "protocol/http11/headers/via.h"
#include "protocol/http11/headers/x_forwarded_for.h"
#include "protocol/http11/headers/x_forwarded_host.h"
#include "protocol/http11/headers/x_forwarded_proto.h"
#include "protocol/http11/headers/rules/directives.h"
#include "protocol/http11/headers/rules/framing.h"
#include "protocol/http11/headers/rules/policy.h"
#include "protocol/http11/headers/rules/routing.h"
#include "parsed_types.h"
#include "platform.h"
#include "policies.h"
#include "verdict.h"
#include "target.h"

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
  // | [>] TYPEs                                                    ( public ) |
  // +=========================================================================+
  using body_deserializer_t =
      std::variant<body_deserializer<body_codec_raw>,
                   body_deserializer<body_codec_chunked>>;
  // +=========================================================================+
  // | [>] GETTERs                                                  ( public ) |
  // +=========================================================================+
  INLINE auto get_method() const { return method_; }
  INLINE auto get_target() const { return target_; }
  INLINE auto get_absolute_path() const { return absolute_path_; }
  INLINE auto get_query() const { return query_part_; }
  INLINE auto get_query_parameter(std::size_t i) const {
    return query_parameters_[i];
  }
  INLINE auto get_query_parameters_length() const {
    return query_parameters_.size();
  }
  INLINE auto has_host() const { return has_host_; }
  INLINE auto get_host() const { return host_; }
  INLINE auto get_host_port() const { return host_port_; }
  INLINE auto get_host_type() const { return host_type_; }
  INLINE auto has_target_authority() const { return has_target_authority_; }
  INLINE auto get_target_authority_host() const {
    return target_authority_host_;
  }
  INLINE auto get_target_authority_port() const {
    return target_authority_port_;
  }
  INLINE auto get_target_authority_type() const {
    return target_authority_type_;
  }
  INLINE auto get_header(std::size_t i) const { return headers_[i]; }
  INLINE auto get_headers_length() const { return headers_.size(); }
  INLINE auto has_body() const { return body_.has_value(); }
  INLINE auto get_body_deserializer() const -> body_deserializer_t& {
    return *body_;
  }
  // +=========================================================================+
  // | [>] deserialize                                              ( public ) |
  // +=========================================================================+
  static deserialization_result<request> deserialize(std::string_view sv) {
    std::size_t i = 0;
    const std::size_t sv_size = sv.size();
    // [first] pass..
    std::string_view query_tmp;
    std::string_view method_tmp;
    std::string_view absolute_path_tmp;
    target target_tmp = target::kUnknown;
    std::vector<std::pair<std::string, std::string>> headers_tmp;
    // [second] pass..
    policies policies_tmp;
    connection connection_tmp;
    context ctx{connection_tmp, policies_tmp};
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
    if (method_tmp == method_names::kConnect) {
      std::string_view authority_host, authority_port;
      helpers::host_type authority_type;
      status = helpers::try_to_deserialize_as_authority_form(
          sv.substr(i), authority_host, authority_port, authority_type,
          bytes_used);
      if (status == deserialization_status::kSucceeded) {
        target_tmp = target::kAuthorityForm;
        ctx.has_target_authority = true;
        ctx.target_authority = {authority_host, authority_port, authority_type};
      }
    } else if (method_tmp == method_names::kOptions && sv[i] == '*') {
      status = helpers::try_to_deserialize_as_asterisk_form(sv.substr(i),
                                                            bytes_used);
      if (status == deserialization_status::kSucceeded) {
        target_tmp = target::kAsteriskForm;
      }
    } else {
      status = helpers::try_to_deserialize_as_origin_form(
          sv.substr(i), absolute_path_tmp, query_tmp, bytes_used);
      if (status == deserialization_status::kSucceeded) {
        target_tmp = target::kOriginForm;
      } else {
        bool has_authority = false;
        std::string_view authority_host, authority_port, authority_scheme;
        helpers::host_type authority_type;
        status = helpers::try_to_deserialize_as_absolute_form(
            sv.substr(i), absolute_path_tmp, query_tmp, has_authority,
            authority_host, authority_port, authority_type, authority_scheme,
            bytes_used);
        if (status == deserialization_status::kSucceeded) {
          target_tmp = target::kAbsoluteForm;
          if (has_authority) {
            ctx.has_target_authority = true;
            ctx.target_authority = {authority_host, authority_port,
                                    authority_type, authority_scheme};
          }
        }
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
    if (i + 7 >= sv_size)
      if (sv[i + 0] != 'H' || sv[i + 1] != 'T' || sv[i + 2] != 'T' ||
          sv[i + 3] != 'P' || sv[i + 4] != '/' ||
          !helpers::is_digit(sv[i + 5]) || sv[i + 6] != '.' ||
          !helpers::is_digit(sv[i + 7])) {
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
          // The raw query component is preserved verbatim in query_part_ above;
          // here we also expose it as structured key/value pairs. The helper
          // yields zero-copy views over query_part_ (kept alive by req), which
          // we copy into owned std::string pairs so query_parameters_ survives
          // past deserialize() independently of the source buffer, exactly like
          // method_/absolute_path_/query_part_.
          if (!req->query_part_.empty()) {
            std::array<std::string_view, kMaxQueryParameters> keys_tmp;
            std::array<std::string_view, kMaxQueryParameters> values_tmp;
            std::size_t qp_count = helpers::split_query_parameters(
                req->query_part_, keys_tmp, values_tmp);
            req->query_parameters_.reserve(qp_count);
            for (std::size_t q = 0; q < qp_count; q++) {
              req->query_parameters_.emplace_back(std::string(keys_tmp[q]),
                                                  std::string(values_tmp[q]));
            }
          }
          // ctx.host / ctx.target_authority are zero-copy views over sv, which
          // does not outlive deserialize(); materialize them into req's own
          // storage here, exactly like method_/absolute_path_/query_part_
          // above.
          req->has_host_ = ctx.has_host;
          req->host_ = ctx.host.host;
          req->host_port_ = ctx.host.port;
          req->host_type_ = ctx.host.type;
          req->has_target_authority_ = ctx.has_target_authority;
          req->target_authority_host_ = ctx.target_authority.host;
          req->target_authority_port_ = ctx.target_authority.port;
          req->target_authority_type_ = ctx.target_authority.type;
          // Every modelled header was already parsed once and interpreted in
          // place during the loop above, populating ctx and the connection
          // state. All that remains is to apply the transversal rules that no
          // single header can decide, over the fully populated context. Any
          // rejection fails the whole deserialization.
          if (headers::rules::framing::apply(ctx) == verdict::kReject ||
              headers::rules::routing::apply(ctx) == verdict::kReject ||
              headers::rules::directives::apply(ctx) == verdict::kReject ||
              headers::rules::policy::apply(ctx) == verdict::kReject) {
            return deserialization_status::kInvalidSource;
          }
          // Build the body reader for this request.  Framing is already
          // resolved: chunked (Transfer-Encoding: chunked) takes precedence
          // over Content-Length (RFC 9112 §6.3).  Methods with no framing
          // signal carry no body.
          if (connection_tmp.chunked) {
            req->body_ = body_deserializer<body_codec_chunked>::chunked();
          } else if (ctx.has_content_length && ctx.content_length > 0) {
            req->body_ =
                body_deserializer<body_codec_raw>::raw({}, ctx.content_length);
          }
          // Translate the HTTP/1.1 connection state into the generic,
          // closed-vocabulary channel intent the transport understands.
          // kUpgrade is reserved for a later phase (e.g. WebSocket).
          channel_intent channel = connection_tmp.close_requested
                                       ? channel_intent::kClose
                                       : channel_intent::kKeep;
          return deserialization_result(req, i, channel);
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
      std::string_view field_name = sv.substr(fn_start, i - 1 - fn_start);
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
      // Single pass: the dispatcher validates the field's syntax and, for a
      // modelled header, produces its parsed_T once and runs the intra-header
      // interpreter, recording any cross-header signal into ctx. A semantic
      // rejection fails deserialization.
      auto const itr_dispatch = header_dispatchers_.find(field_name);
      if (itr_dispatch != header_dispatchers_.end()) {
        if (itr_dispatch->second(field_value, ctx) == verdict::kReject) {
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
  // | Accept                                                     |     [x]    |
  // | Accept-Encoding                                            |     [x]    |
  // | Accept-Language                                            |     [x]    |
  // | Content-Language                                           |     [x]    |
  // | Content-Location                                           |     [x]    |
  // | Range                                                      |     [x]    |
  // | Content-Range                                              |     [x]    |
  // | Accept-Ranges                                              |     [x]    |
  // | If-Range                                                   |     [x]    |
  // | ETag                                                       |     [x]    |
  // | Last-Modified                                              |     [x]    |
  // | If-Match                                                   |     [x]    |
  // | If-None-Match                                              |     [x]    |
  // | If-Modified-Since                                          |     [x]    |
  // | If-Unmodified-Since                                        |     [x]    |
  // | Cache-Control                                              |     [x]    |
  // | Vary                                                       |     [x]    |
  // | Age                                                        |     [x]    |
  // | Expires                                                    |     [x]    |
  // | Pragma                                                     |     [x]    |
  // | Location                                                   |     [x]    |
  // | Allow                                                      |     [x]    |
  // | Retry-After                                                |     [x]    |
  // | Authorization                                              |     [x]    |
  // | WWW-Authenticate                                           |     [x]    |
  // | Authentication-Info                                        |     [x]    |
  // | Cookie                                                     |     [x]    |
  // | Set-Cookie                                                 |     [x]    |
  // | User-Agent                                                 |     [x]    |
  // | Server                                                     |     [x]    |
  // | Referer                                                    |     [x]    |
  // | Max-Forwards                                               |     [x]    |
  // | From                                                       |     [x]    |
  // | Accept-Charset                                             |     [x]    |
  // | Origin                                                     |     [x]    |
  // | Access-Control-Request-Method                              |     [x]    |
  // | Access-Control-Request-Headers                             |     [x]    |
  // | Access-Control-Allow-Origin                                |     [x]    |
  // | Access-Control-Allow-Methods                               |     [x]    |
  // | Access-Control-Allow-Headers                               |     [x]    |
  // | Access-Control-Allow-Credentials                           |     [x]    |
  // | Access-Control-Expose-Headers                              |     [x]    |
  // | Access-Control-Max-Age                                     |     [x]    |
  // | Sec-WebSocket-Key                                          |     [x]    |
  // | Sec-WebSocket-Accept                                       |     [x]    |
  // | Sec-WebSocket-Version                                      |     [x]    |
  // | Sec-WebSocket-Protocol                                     |     [x]    |
  // | Sec-WebSocket-Extensions                                   |     [x]    |
  // | Via                                                        |     [x]    |
  // | Forwarded                                                  |     [x]    |
  // | X-Forwarded-For                                            |     [x]    |
  // | X-Forwarded-Host                                           |     [x]    |
  // | X-Forwarded-Proto                                          |     [x]    |
  // | Keep-Alive                                                 |     [x]    |
  // | Proxy-Connection                                           |     [x]    |
  // +------------------------------------------------------------+------------+
  // +=========================================================================+
  // | [>] TYPEs                                                   ( private ) |
  // +=========================================================================+
  using header_dispatch = verdict (*)(std::string_view, context&);
  // +=========================================================================+
  // | [>] dispatch                                                ( private ) |
  // +=========================================================================+
  // | The dispatcher for every header the semantic layer does not model. It   |
  // | runs the header's single syntactic checker and never touches the        |
  // | context: a value the checker rejects fails deserialization, otherwise it|
  // | is accepted as-is. One template instantiation per checker keeps each    |
  // | registry entry a direct, inlinable call.                                |
  // +=========================================================================+
  template <typename CHty>
  static constexpr verdict dispatch(std::string_view sv, context&) {
    return CHty::check(sv) ? verdict::kAccept : verdict::kReject;
  }
  // +=========================================================================+
  // | [>] dispatch_host (modelled header)                         ( private ) |
  // +=========================================================================+
  static verdict dispatch_host(std::string_view sv, context& ctx) {
    if (ctx.has_host) ctx.multiple_host = true;
    ctx.has_host = true;
    parsed_host_port parsed;
    if (!headers::host::check(sv, parsed)) return verdict::kReject;
    ctx.host = parsed;
    return headers::host::interpret(parsed, ctx.connection, ctx.policies);
  }
  // +=========================================================================+
  // | [>] dispatch_content_length (modelled header)               ( private ) |
  // +=========================================================================+
  static verdict dispatch_content_length(std::string_view sv, context& ctx) {
    std::size_t parsed = 0;
    if (!headers::content_length::check(sv, parsed)) {
      return verdict::kReject;
    }
    if (ctx.has_content_length) ctx.multiple_content_length = true;
    ctx.has_content_length = true;
    ctx.content_length = parsed;
    return headers::content_length::interpret(parsed, ctx.connection,
                                              ctx.policies);
  }
  // +=========================================================================+
  // | [>] dispatch_transfer_encoding (modelled header)            ( private ) |
  // +=========================================================================+
  static verdict dispatch_transfer_encoding(std::string_view sv, context& ctx) {
    parsed_parameter_list parsed;
    if (!headers::transfer_encoding::check(sv, parsed)) {
      return verdict::kReject;
    }
    ctx.has_transfer_encoding = true;
    return headers::transfer_encoding::interpret(parsed, ctx.connection,
                                                 ctx.policies);
  }
  // +=========================================================================+
  // | [>] dispatch_connection (modelled header)                   ( private ) |
  // +=========================================================================+
  static verdict dispatch_connection(std::string_view sv, context& ctx) {
    parsed_token_list parsed;
    if (!headers::connection::check(sv, parsed)) {
      return verdict::kReject;
    }
    return headers::connection::interpret(parsed, ctx.connection, ctx.policies);
  }
  // +=========================================================================+
  // | [>] dispatch_te (modelled header)                           ( private ) |
  // +=========================================================================+
  static verdict dispatch_te(std::string_view sv, context& ctx) {
    parsed_parameter_list parsed;
    if (!headers::te::check(sv, parsed)) return verdict::kReject;
    return headers::te::interpret(parsed, ctx.connection, ctx.policies);
  }
  // +=========================================================================+
  // | [>] dispatch_trailer (modelled header)                      ( private ) |
  // +=========================================================================+
  static verdict dispatch_trailer(std::string_view sv, context& ctx) {
    parsed_token_list parsed;
    if (!headers::trailer::check(sv, parsed)) {
      return verdict::kReject;
    }
    return headers::trailer::interpret(parsed, ctx.connection, ctx.policies);
  }
  // +=========================================================================+
  // | [>] dispatch_expect (modelled header)                       ( private ) |
  // +=========================================================================+
  static verdict dispatch_expect(std::string_view sv, context& ctx) {
    parsed_parameter_list parsed;
    if (!headers::expect::check(sv, parsed)) {
      return verdict::kReject;
    }
    return headers::expect::interpret(parsed, ctx.connection, ctx.policies);
  }
  // +=========================================================================+
  // | [>] dispatch_upgrade (modelled header)                      ( private ) |
  // +=========================================================================+
  static verdict dispatch_upgrade(std::string_view sv, context& ctx) {
    parsed_token_list parsed;
    if (!headers::upgrade::check(sv, parsed)) {
      return verdict::kReject;
    }
    return headers::upgrade::interpret(parsed, ctx.connection, ctx.policies);
  }
  // +=========================================================================+
  // | [>] dispatch_max_forwards (modelled header)                 ( private ) |
  // +=========================================================================+
  static verdict dispatch_max_forwards(std::string_view sv, context& ctx) {
    std::size_t parsed = 0;
    if (!headers::max_forwards::check(sv, parsed)) {
      return verdict::kReject;
    }
    return headers::max_forwards::interpret(parsed, ctx.connection,
                                            ctx.policies);
  }
  // +=========================================================================+
  // | [>] dispatch_via (modelled header)                          ( private ) |
  // +=========================================================================+
  static verdict dispatch_via(std::string_view sv, context& ctx) {
    parsed_via_list parsed;
    if (!headers::via::check(sv, parsed)) return verdict::kReject;
    ctx.forwarding_hops += parsed.elements.size();
    return headers::via::interpret(parsed, ctx.connection, ctx.policies);
  }
  // +=========================================================================+
  // | [>] dispatch_forwarded (modelled header)                    ( private ) |
  // +=========================================================================+
  static verdict dispatch_forwarded(std::string_view sv, context& ctx) {
    parsed_forwarded_list parsed;
    if (!headers::forwarded::check(sv, parsed)) {
      return verdict::kReject;
    }
    ctx.forwarding_hops += parsed.elements.size();
    return headers::forwarded::interpret(parsed, ctx.connection, ctx.policies);
  }
  // +=========================================================================+
  // | [>] dispatch_x_forwarded_for (modelled header)              ( private ) |
  // +=========================================================================+
  static verdict dispatch_x_forwarded_for(std::string_view sv, context& ctx) {
    parsed_token_list parsed;
    if (!headers::x_forwarded_for::check(sv, parsed)) {
      return verdict::kReject;
    }
    ctx.forwarding_hops += parsed.elements.size();
    return headers::x_forwarded_for::interpret(parsed, ctx.connection,
                                               ctx.policies);
  }
  // +=========================================================================+
  // | [>] dispatch_x_forwarded_host (modelled header)             ( private ) |
  // +=========================================================================+
  static verdict dispatch_x_forwarded_host(std::string_view sv, context& ctx) {
    parsed_host_port parsed;
    if (!headers::x_forwarded_host::check(sv, parsed)) {
      return verdict::kReject;
    }
    return headers::x_forwarded_host::interpret(parsed, ctx.connection,
                                                ctx.policies);
  }
  // +=========================================================================+
  // | [>] dispatch_x_forwarded_proto (modelled header)            ( private ) |
  // +=========================================================================+
  static verdict dispatch_x_forwarded_proto(std::string_view sv, context& ctx) {
    parsed_token_list parsed;
    if (!headers::x_forwarded_proto::check(sv, parsed)) {
      return verdict::kReject;
    }
    return headers::x_forwarded_proto::interpret(parsed, ctx.connection,
                                                 ctx.policies);
  }
  // +=========================================================================+
  // | [>] CONSTANTs                                               ( private ) |
  // +=========================================================================+
  static constexpr std::size_t kMaxQueryParameters = 128;
  static const inline common::hash_map<std::string_view, header_dispatch>
      header_dispatchers_ = {
          {"Host",  // check & interpret!
           &request::dispatch_host},
          {"Content-Length",  // check & interpret!
           &request::dispatch_content_length},
          {"Transfer-Encoding",  // check & interpret!
           &request::dispatch_transfer_encoding},
          {"Connection",  // check & interpret!
           &request::dispatch_connection},
          {"TE",  // check & interpret!
           &request::dispatch_te},
          {"Trailer",  // check & interpret!
           &request::dispatch_trailer},
          {"Expect",  // check & interpret!
           &request::dispatch_expect},
          {"Upgrade",  // check & interpret!
           &request::dispatch_upgrade},
          {"Content-Type",  // check only!
           &request::dispatch<headers::content_type>},
          {"Content-Encoding",  // check only!
           &request::dispatch<headers::content_encoding>},
          {"Date",  // check only!
           &request::dispatch<headers::date>},
          {"Accept",  // check only!
           &request::dispatch<headers::accept>},
          {"Accept-Encoding",  // check only!
           &request::dispatch<headers::accept_encoding>},
          {"Accept-Language",  // check only!
           &request::dispatch<headers::accept_language>},
          {"Content-Language",  // check only!
           &request::dispatch<headers::content_language>},
          {"Content-Location",  // check only!
           &request::dispatch<headers::content_location>},
          {"Range",  // check only!
           &request::dispatch<headers::range>},
          {"Content-Range",  // check only!
           &request::dispatch<headers::content_range>},
          {"Accept-Ranges",  // check only!
           &request::dispatch<headers::accept_ranges>},
          {"If-Range",  // check only!
           &request::dispatch<headers::if_range>},
          {"ETag",  // check only!
           &request::dispatch<headers::etag>},
          {"Last-Modified",  // check only!
           &request::dispatch<headers::last_modified>},
          {"If-Match",  // check only!
           &request::dispatch<headers::if_match>},
          {"If-None-Match",  // check only!
           &request::dispatch<headers::if_none_match>},
          {"If-Modified-Since",  // check only!
           &request::dispatch<headers::if_modified_since>},
          {"If-Unmodified-Since",  // check only!
           &request::dispatch<headers::if_unmodified_since>},
          {"Cache-Control",  // check only!
           &request::dispatch<headers::cache_control>},
          {"Vary",  // check only!
           &request::dispatch<headers::vary>},
          {"Age",  // check only!
           &request::dispatch<headers::age>},
          {"Expires",  // check only!
           &request::dispatch<headers::expires>},
          {"Pragma",  // check only!
           &request::dispatch<headers::pragma>},
          {"Location",  // check only!
           &request::dispatch<headers::location>},
          {"Allow",  // check only!
           &request::dispatch<headers::allow>},
          {"Retry-After",  // check only!
           &request::dispatch<headers::retry_after>},
          {"Authorization",  // check only!
           &request::dispatch<headers::authorization>},
          {"WWW-Authenticate",  // check only!
           &request::dispatch<headers::www_authenticate>},
          {"Authentication-Info",  // check only!
           &request::dispatch<headers::authentication_info>},
          {"Cookie",  // check only!
           &request::dispatch<headers::cookie>},
          {"Set-Cookie",  // check only!
           &request::dispatch<headers::set_cookie>},
          {"User-Agent",  // check only!
           &request::dispatch<headers::user_agent>},
          {"Server",  // check only!
           &request::dispatch<headers::server>},
          {"Referer",  // check only!
           &request::dispatch<headers::referer>},
          {"Max-Forwards",  // check only!
           &request::dispatch_max_forwards},
          {"From",  // check only!
           &request::dispatch<headers::from>},
          {"Accept-Charset",  // check only!
           &request::dispatch<headers::accept_charset>},
          {"Origin",  // check only!
           &request::dispatch<headers::origin>},
          {"Access-Control-Request-Method",  // check only!
           &request::dispatch<headers::access_control_request_method>},
          {"Access-Control-Request-Headers",  // check only!
           &request::dispatch<headers::access_control_request_headers>},
          {"Access-Control-Allow-Origin",  // check only!
           &request::dispatch<headers::access_control_allow_origin>},
          {"Access-Control-Allow-Methods",  // check only!
           &request::dispatch<headers::access_control_allow_methods>},
          {"Access-Control-Allow-Headers",  // check only!
           &request::dispatch<headers::access_control_allow_headers>},
          {"Access-Control-Allow-Credentials",  // check only!
           &request::dispatch<headers::access_control_allow_credentials>},
          {"Access-Control-Expose-Headers",  // check only!
           &request::dispatch<headers::access_control_expose_headers>},
          {"Access-Control-Max-Age",  // check only!
           &request::dispatch<headers::access_control_max_age>},
          {"Sec-WebSocket-Key",  // check only!
           &request::dispatch<headers::sec_websocket_key>},
          {"Sec-WebSocket-Accept",  // check only!
           &request::dispatch<headers::sec_websocket_accept>},
          {"Sec-WebSocket-Version",  // check only!
           &request::dispatch<headers::sec_websocket_version>},
          {"Sec-WebSocket-Protocol",  // check only!
           &request::dispatch<headers::sec_websocket_protocol>},
          {"Sec-WebSocket-Extensions",  // check only!
           &request::dispatch<headers::sec_websocket_extensions>},
          {"Via",  // check & interpret!
           &request::dispatch_via},
          {"Forwarded",  // check & interpret!
           &request::dispatch_forwarded},
          {"X-Forwarded-For",  // check & interpret!
           &request::dispatch_x_forwarded_for},
          {"X-Forwarded-Host",  // check & interpret!
           &request::dispatch_x_forwarded_host},
          {"X-Forwarded-Proto",  // check & interpret!
           &request::dispatch_x_forwarded_proto},
          {"Keep-Alive",  // check only!
           &request::dispatch<headers::x_keep_alive>},
          {"Proxy-Connection",  // check only!
           &request::dispatch<headers::x_proxy_connection>},
  };
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  std::string method_;
  std::string query_part_;
  std::string absolute_path_;
  target target_ = target::kUnknown;
  bool has_host_ = false;
  std::string host_;
  std::string host_port_;
  helpers::host_type host_type_ = helpers::host_type::kUnknown;
  bool has_target_authority_ = false;
  std::string target_authority_host_;
  std::string target_authority_port_;
  helpers::host_type target_authority_type_ = helpers::host_type::kUnknown;
  std::vector<std::pair<std::string, std::string>> headers_;
  std::vector<std::pair<std::string, std::string>> query_parameters_;
  mutable std::optional<body_deserializer_t> body_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
