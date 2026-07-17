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

#ifndef martianlabs_doba_protocol_http11_decoder_h
#define martianlabs_doba_protocol_http11_decoder_h

#include <algorithm>

#include "protocol/http11/helpers.h"
#include "protocol/http11/request.h"
#include "protocol/http11/response.h"
#include "protocol/http11/context.h"
#include "protocol/http11/query_parameter.h"
#include "protocol/http11/body/writer_chunked.h"
#include "protocol/http11/body/writer_raw.h"
#include "protocol/http11/headers/accept.h"
#include "protocol/http11/headers/accept_charset.h"
#include "protocol/http11/headers/accept_encoding.h"
#include "protocol/http11/headers/accept_language.h"
#include "protocol/http11/headers/accept_ranges.h"
#include "protocol/http11/headers/access_control_allow_credentials.h"
#include "protocol/http11/headers/access_control_allow_headers.h"
#include "protocol/http11/headers/access_control_allow_methods.h"
#include "protocol/http11/headers/access_control_allow_origin.h"
#include "protocol/http11/headers/access_control_expose_headers.h"
#include "protocol/http11/headers/access_control_max_age.h"
#include "protocol/http11/headers/access_control_request_headers.h"
#include "protocol/http11/headers/access_control_request_method.h"
#include "protocol/http11/headers/age.h"
#include "protocol/http11/headers/allow.h"
#include "protocol/http11/headers/authentication_info.h"
#include "protocol/http11/headers/authorization.h"
#include "protocol/http11/headers/cache_control.h"
#include "protocol/http11/headers/connection.h"
#include "protocol/http11/headers/content_encoding.h"
#include "protocol/http11/headers/content_language.h"
#include "protocol/http11/headers/content_length.h"
#include "protocol/http11/headers/content_location.h"
#include "protocol/http11/headers/content_range.h"
#include "protocol/http11/headers/content_type.h"
#include "protocol/http11/headers/cookie.h"
#include "protocol/http11/headers/date.h"
#include "protocol/http11/headers/etag.h"
#include "protocol/http11/headers/expect.h"
#include "protocol/http11/headers/expires.h"
#include "protocol/http11/headers/forwarded.h"
#include "protocol/http11/headers/from.h"
#include "protocol/http11/headers/host.h"
#include "protocol/http11/headers/if_match.h"
#include "protocol/http11/headers/if_modified_since.h"
#include "protocol/http11/headers/if_none_match.h"
#include "protocol/http11/headers/if_range.h"
#include "protocol/http11/headers/if_unmodified_since.h"
#include "protocol/http11/headers/keep_alive.h"
#include "protocol/http11/headers/last_modified.h"
#include "protocol/http11/headers/location.h"
#include "protocol/http11/headers/max_forwards.h"
#include "protocol/http11/headers/origin.h"
#include "protocol/http11/headers/pragma.h"
#include "protocol/http11/headers/proxy_connection.h"
#include "protocol/http11/headers/range.h"
#include "protocol/http11/headers/referer.h"
#include "protocol/http11/headers/retry_after.h"
#include "protocol/http11/headers/sec_websocket_accept.h"
#include "protocol/http11/headers/sec_websocket_extensions.h"
#include "protocol/http11/headers/sec_websocket_key.h"
#include "protocol/http11/headers/sec_websocket_protocol.h"
#include "protocol/http11/headers/sec_websocket_version.h"
#include "protocol/http11/headers/server.h"
#include "protocol/http11/headers/set_cookie.h"
#include "protocol/http11/headers/te.h"
#include "protocol/http11/headers/trailer.h"
#include "protocol/http11/headers/transfer_encoding.h"
#include "protocol/http11/headers/upgrade.h"
#include "protocol/http11/headers/user_agent.h"
#include "protocol/http11/headers/vary.h"
#include "protocol/http11/headers/via.h"
#include "protocol/http11/headers/www_authenticate.h"
#include "protocol/http11/headers/x_forwarded_for.h"
#include "protocol/http11/headers/x_forwarded_host.h"
#include "protocol/http11/headers/x_forwarded_proto.h"
#include "protocol/http11/headers/rules/directives.h"
#include "protocol/http11/headers/rules/framing.h"
#include "protocol/http11/headers/rules/policy.h"
#include "protocol/http11/headers/rules/routing.h"

namespace martianlabs::doba::protocol::http11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] decoder                                                     ( class ) |
// +---------------------------------------------------------------------------+
// | This class holds for the http 1.1 decoder implementation.                 |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   RQty - request being used (http11::request by default).                 |
// |   RSty - response being used (http11::response by default).               |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty = request, typename RSty = response>
class decoder {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  decoder() = default;
  decoder(const decoder&) = delete;
  decoder(decoder&&) noexcept = delete;
  ~decoder() = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  decoder& operator=(const decoder&) = delete;
  decoder& operator=(decoder&&) noexcept = delete;
  // +=========================================================================+
  // | [>] parse                                                    ( public ) |
  // +=========================================================================+
  deserialization_result<request> parse(std::string_view sv) {
    return body_writer_ ? parse_body(sv) : parse_core(sv);
  }

 private:
  // +=========================================================================+
  // | [>] parse_core                                               ( public ) |
  // +=========================================================================+
  deserialization_result<request> parse_core(std::string_view sv) {
    std::size_t i = 0;
    const std::size_t sv_size = sv.size();
    // +-----------------------------------------------------------------------+
    // | request-line = method SP request-target SP HTTP-version               |
    // +-----------------------------------------------------------------------+
    // +-----------------------------------------------------------------------+
    // | [method] part!                                                        |
    // +-----------------------------------------------------------------------+
    if (!sv_size) return deserialization_status::kMoreBytesNeeded;
    method_ = helpers::consume_token(sv);
    if (method_.empty()) return deserialization_status::kInvalidSource;
    i += method_.size();
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
    if (method_ == method_names::kConnect) {
      std::string_view authority_host, authority_port;
      helpers::host_type authority_type;
      status = helpers::try_to_deserialize_as_authority_form(
          sv.substr(i), authority_host, authority_port, authority_type,
          bytes_used);
      if (status == deserialization_status::kSucceeded) {
        target_ = target::kAuthorityForm;
        ctx_.has_target_authority = true;
        ctx_.target_authority = {authority_host, authority_port,
                                 authority_type};
      }
    } else if (method_ == method_names::kOptions && sv[i] == '*') {
      status = helpers::try_to_deserialize_as_asterisk_form(sv.substr(i),
                                                            bytes_used);
      if (status == deserialization_status::kSucceeded) {
        target_ = target::kAsteriskForm;
      }
    } else {
      status = helpers::try_to_deserialize_as_origin_form(
          sv.substr(i), absolute_path_, query_, bytes_used);
      if (status == deserialization_status::kSucceeded) {
        target_ = target::kOriginForm;
      } else {
        bool has_authority = false;
        std::string_view authority_host, authority_port, authority_scheme;
        helpers::host_type authority_type;
        status = helpers::try_to_deserialize_as_absolute_form(
            sv.substr(i), absolute_path_, query_, has_authority, authority_host,
            authority_port, authority_type, authority_scheme, bytes_used);
        if (status == deserialization_status::kSucceeded) {
          target_ = target::kAbsoluteForm;
          if (has_authority) {
            ctx_.has_target_authority = true;
            ctx_.target_authority = {authority_host, authority_port,
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
          // Check for the presence of a body and build the body writer for this
          // request. We only support chunked and raw framing, and only if the
          // request has a body.
          if (ctx_.connection.chunked) {
            body_buffer_ = common::writer();
            body_writer_ = body::writer_chunked();
          } else if (ctx_.has_content_length && ctx_.content_length > 0) {
            body_buffer_ = common::writer();
            body_writer_ = body::writer_raw(ctx_.content_length);
          }
          // Mount the request object and keep the result!
          decoded_ = mount_request(i);
          // In case of error, we return the error code and stop parsing.
          // Otherwise, we continue parsing the body if there is a body writer,
          // or we return the request if there is no body writer.
          if (decoded_.code != deserialization_status::kSucceeded) {
            return decoded_.code;
          }
          // In case of no body writer, we are done and can return the request.
          // Otherwise, we need to continue parsing the body, so we remove the
          // already consumed bytes from the input and call parse() again to
          // continue parsing the body.
          if (!body_writer_) {
            reset();
            return decoded_;
          }
          sv.remove_prefix(i);
          return parse(sv);
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
        if (itr_dispatch->second(field_value, ctx_) == verdict::kReject) {
          return deserialization_status::kInvalidSource;
        }
      }
      headers_.emplace_back(field_name, field_value);
      i += 2;
      fn_start = i;
      field_name_decoded = false;
    }
    return deserialization_status::kMoreBytesNeeded;
  }
  // +=========================================================================+
  // | [>] parse_body                                               ( public ) |
  // +=========================================================================+
  deserialization_result<request> parse_body(std::string_view sv) {
    body::writer_state state = std::visit(
        [&sv, this](auto& arg) -> body::writer_state {
          std::span<const std::byte> byte_span{
              reinterpret_cast<const std::byte*>(sv.data()), sv.size()};
          return arg.write(byte_span, body_buffer_.value());
        },
        body_writer_.value());
    if (state.has_error) return deserialization_status::kInvalidSource;
    decoded_.bytes_used += state.consumed;
    if (!state.complete) return deserialization_status::kMoreBytesNeeded;
    reset();
    return decoded_;
  }
  // +=========================================================================+
  // | [>] mount_request                                            ( public ) |
  // +=========================================================================+
  deserialization_result<request> mount_request(std::size_t bytes_used) {
    // Every modelled header was already parsed once and interpreted in
    // place during the loop above, populating ctx and the connection
    // state. All that remains is to apply the transversal rules that no
    // single header can decide, over the fully populated context. Any
    // rejection fails the whole deserialization.
    if (headers::rules::framing::apply(ctx_) == verdict::kReject ||
        headers::rules::routing::apply(ctx_) == verdict::kReject ||
        headers::rules::directives::apply(ctx_) == verdict::kReject ||
        headers::rules::policy::apply(ctx_) == verdict::kReject) {
      return deserialization_status::kInvalidSource;
    }
    // Now that the request is fully validated, we can build the request object
    std::shared_ptr<request> req = std::make_shared<request>();
    req->set_method(method_);
    req->set_absolute_path(absolute_path_);
    req->set_target(target_);
    req->set_headers(headers_);
    // Only if the query part is not empty, we will split it into key-value
    // pairs and set it in the request.
    if (!query_.empty()) {
      std::array<std::string_view, kMaxQueryParameters> keys;
      std::array<std::string_view, kMaxQueryParameters> values;
      std::size_t qc = helpers::split_query_parameters(query_, keys, values);
      std::vector<query_parameter_view> query_parameters;
      query_parameters.reserve(qc);
      for (std::size_t q = 0; q < qc; q++) {
        query_parameters.emplace_back(keys[q], values[q]);
      }
      req->set_query_parameters(query_parameters);
    }
    // Check if the request has a host set it in the request.
    if (ctx_.has_host) {
      req->set_host(ctx_.host.host, ctx_.host.port, ctx_.host.type);
    }
    // Check if the request has a target authority and set it in the request.
    if (ctx_.has_target_authority) {
      req->set_target_authority(ctx_.target_authority.host,
                                ctx_.target_authority.port,
                                ctx_.target_authority.type);
    }
    return deserialization_result<request>(req, bytes_used,
                                           ctx_.connection.close_requested
                                               ? channel_intent::kClose
                                               : channel_intent::kKeep);
  }
  // +=========================================================================+
  // | [>] reset                                                   ( private ) |
  // +=========================================================================+
  void reset() {
    method_ = {};
    target_ = target::kUnknown;
    absolute_path_ = {};
    query_ = {};
    headers_ = {};
    ctx_ = {};
    body_writer_ = std::nullopt;
    body_buffer_ = std::nullopt;
  }
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
  using header_dispatch = verdict (*)(std::string_view,
                                      protocol::http11::context&);
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
           &dispatch_host},
          {"Content-Length",  // check & interpret!
           &dispatch_content_length},
          {"Transfer-Encoding",  // check & interpret!
           &dispatch_transfer_encoding},
          {"Connection",  // check & interpret!
           &dispatch_connection},
          {"TE",  // check & interpret!
           &dispatch_te},
          {"Trailer",  // check & interpret!
           &dispatch_trailer},
          {"Expect",  // check & interpret!
           &dispatch_expect},
          {"Upgrade",  // check & interpret!
           &dispatch_upgrade},
          {"Content-Type",  // check only!
           &dispatch<headers::content_type>},
          {"Content-Encoding",  // check only!
           &dispatch<headers::content_encoding>},
          {"Date",  // check only!
           &dispatch<headers::date>},
          {"Accept",  // check only!
           &dispatch<headers::accept>},
          {"Accept-Encoding",  // check only!
           &dispatch<headers::accept_encoding>},
          {"Accept-Language",  // check only!
           &dispatch<headers::accept_language>},
          {"Content-Language",  // check only!
           &dispatch<headers::content_language>},
          {"Content-Location",  // check only!
           &dispatch<headers::content_location>},
          {"Range",  // check only!
           &dispatch<headers::range>},
          {"Content-Range",  // check only!
           &dispatch<headers::content_range>},
          {"Accept-Ranges",  // check only!
           &dispatch<headers::accept_ranges>},
          {"If-Range",  // check only!
           &dispatch<headers::if_range>},
          {"ETag",  // check only!
           &dispatch<headers::etag>},
          {"Last-Modified",  // check only!
           &dispatch<headers::last_modified>},
          {"If-Match",  // check only!
           &dispatch<headers::if_match>},
          {"If-None-Match",  // check only!
           &dispatch<headers::if_none_match>},
          {"If-Modified-Since",  // check only!
           &dispatch<headers::if_modified_since>},
          {"If-Unmodified-Since",  // check only!
           &dispatch<headers::if_unmodified_since>},
          {"Cache-Control",  // check only!
           &dispatch<headers::cache_control>},
          {"Vary",  // check only!
           &dispatch<headers::vary>},
          {"Age",  // check only!
           &dispatch<headers::age>},
          {"Expires",  // check only!
           &dispatch<headers::expires>},
          {"Pragma",  // check only!
           &dispatch<headers::pragma>},
          {"Location",  // check only!
           &dispatch<headers::location>},
          {"Allow",  // check only!
           &dispatch<headers::allow>},
          {"Retry-After",  // check only!
           &dispatch<headers::retry_after>},
          {"Authorization",  // check only!
           &dispatch<headers::authorization>},
          {"WWW-Authenticate",  // check only!
           &dispatch<headers::www_authenticate>},
          {"Authentication-Info",  // check only!
           &dispatch<headers::authentication_info>},
          {"Cookie",  // check only!
           &dispatch<headers::cookie>},
          {"Set-Cookie",  // check only!
           &dispatch<headers::set_cookie>},
          {"User-Agent",  // check only!
           &dispatch<headers::user_agent>},
          {"Server",  // check only!
           &dispatch<headers::server>},
          {"Referer",  // check only!
           &dispatch<headers::referer>},
          {"Max-Forwards",  // check only!
           &dispatch_max_forwards},
          {"From",  // check only!
           &dispatch<headers::from>},
          {"Accept-Charset",  // check only!
           &dispatch<headers::accept_charset>},
          {"Origin",  // check only!
           &dispatch<headers::origin>},
          {"Access-Control-Request-Method",  // check only!
           &dispatch<headers::access_control_request_method>},
          {"Access-Control-Request-Headers",  // check only!
           &dispatch<headers::access_control_request_headers>},
          {"Access-Control-Allow-Origin",  // check only!
           &dispatch<headers::access_control_allow_origin>},
          {"Access-Control-Allow-Methods",  // check only!
           &dispatch<headers::access_control_allow_methods>},
          {"Access-Control-Allow-Headers",  // check only!
           &dispatch<headers::access_control_allow_headers>},
          {"Access-Control-Allow-Credentials",  // check only!
           &dispatch<headers::access_control_allow_credentials>},
          {"Access-Control-Expose-Headers",  // check only!
           &dispatch<headers::access_control_expose_headers>},
          {"Access-Control-Max-Age",  // check only!
           &dispatch<headers::access_control_max_age>},
          {"Sec-WebSocket-Key",  // check only!
           &dispatch<headers::sec_websocket_key>},
          {"Sec-WebSocket-Accept",  // check only!
           &dispatch<headers::sec_websocket_accept>},
          {"Sec-WebSocket-Version",  // check only!
           &dispatch<headers::sec_websocket_version>},
          {"Sec-WebSocket-Protocol",  // check only!
           &dispatch<headers::sec_websocket_protocol>},
          {"Sec-WebSocket-Extensions",  // check only!
           &dispatch<headers::sec_websocket_extensions>},
          {"Via",  // check & interpret!
           &dispatch_via},
          {"Forwarded",  // check & interpret!
           &dispatch_forwarded},
          {"X-Forwarded-For",  // check & interpret!
           &dispatch_x_forwarded_for},
          {"X-Forwarded-Host",  // check & interpret!
           &dispatch_x_forwarded_host},
          {"X-Forwarded-Proto",  // check & interpret!
           &dispatch_x_forwarded_proto},
          {"Keep-Alive",  // check only!
           &dispatch<headers::x_keep_alive>},
          {"Proxy-Connection",  // check only!
           &dispatch<headers::x_proxy_connection>},
  };
  // +=========================================================================+
  // | [>] USINGs                                                  ( private ) |
  // +=========================================================================+
  using body_writer_t = std::variant<body::writer_chunked, body::writer_raw>;
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  context ctx_{};
  std::string_view query_;
  std::string_view method_;
  std::string_view absolute_path_;
  target target_ = target::kUnknown;
  std::vector<std::pair<std::string_view, std::string_view>> headers_;
  std::optional<body_writer_t> body_writer_ = std::nullopt;
  std::optional<common::writer> body_buffer_;
  deserialization_result<request> decoded_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
