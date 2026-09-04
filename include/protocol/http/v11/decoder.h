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

#ifndef martianlabs_doba_protocol_http_v11_decoder_h
#define martianlabs_doba_protocol_http_v11_decoder_h

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "common/byte_storage.h"
#include "common/hash_map.h"
#include "common/writer.h"
#include "protocol/deserialization.h"
#include "protocol/http/common/header.h"
#include "protocol/http/common/helpers.h"
#include "protocol/http/common/method_names.h"
#include "protocol/http/common/query_parameter.h"
#include "protocol/http/common/request_getter.h"
#include "protocol/http/common/target.h"
#include "protocol/http/common/headers/accept.h"
#include "protocol/http/common/headers/accept_charset.h"
#include "protocol/http/common/headers/accept_encoding.h"
#include "protocol/http/common/headers/accept_language.h"
#include "protocol/http/common/headers/accept_ranges.h"
#include "protocol/http/common/headers/access_control_allow_credentials.h"
#include "protocol/http/common/headers/access_control_allow_headers.h"
#include "protocol/http/common/headers/access_control_allow_methods.h"
#include "protocol/http/common/headers/access_control_allow_origin.h"
#include "protocol/http/common/headers/access_control_expose_headers.h"
#include "protocol/http/common/headers/access_control_max_age.h"
#include "protocol/http/common/headers/access_control_request_headers.h"
#include "protocol/http/common/headers/access_control_request_method.h"
#include "protocol/http/common/headers/age.h"
#include "protocol/http/common/headers/allow.h"
#include "protocol/http/common/headers/authentication_info.h"
#include "protocol/http/common/headers/authorization.h"
#include "protocol/http/common/headers/cache_control.h"
#include "protocol/http/common/headers/content_encoding.h"
#include "protocol/http/common/headers/content_language.h"
#include "protocol/http/common/headers/content_location.h"
#include "protocol/http/common/headers/content_range.h"
#include "protocol/http/common/headers/content_type.h"
#include "protocol/http/common/headers/cookie.h"
#include "protocol/http/common/headers/date.h"
#include "protocol/http/common/headers/etag.h"
#include "protocol/http/common/headers/expires.h"
#include "protocol/http/common/headers/from.h"
#include "protocol/http/common/headers/if_match.h"
#include "protocol/http/common/headers/if_modified_since.h"
#include "protocol/http/common/headers/if_none_match.h"
#include "protocol/http/common/headers/if_range.h"
#include "protocol/http/common/headers/if_unmodified_since.h"
#include "protocol/http/common/headers/keep_alive.h"
#include "protocol/http/common/headers/last_modified.h"
#include "protocol/http/common/headers/location.h"
#include "protocol/http/common/headers/origin.h"
#include "protocol/http/common/headers/pragma.h"
#include "protocol/http/common/headers/proxy_connection.h"
#include "protocol/http/common/headers/range.h"
#include "protocol/http/common/headers/referer.h"
#include "protocol/http/common/headers/retry_after.h"
#include "protocol/http/common/headers/sec_websocket_accept.h"
#include "protocol/http/common/headers/sec_websocket_extensions.h"
#include "protocol/http/common/headers/sec_websocket_key.h"
#include "protocol/http/common/headers/sec_websocket_protocol.h"
#include "protocol/http/common/headers/sec_websocket_version.h"
#include "protocol/http/common/headers/server.h"
#include "protocol/http/common/headers/set_cookie.h"
#include "protocol/http/common/headers/user_agent.h"
#include "protocol/http/common/headers/vary.h"
#include "protocol/http/common/headers/www_authenticate.h"
#include "protocol/http/v11/body/framer_raw.h"
#include "protocol/http/v11/body/framer_chunked.h"
#include "protocol/http/v11/context.h"
#include "protocol/http/v11/headers/connection.h"
#include "protocol/http/v11/headers/content_length.h"
#include "protocol/http/v11/headers/expect.h"
#include "protocol/http/v11/headers/forwarded.h"
#include "protocol/http/v11/headers/host.h"
#include "protocol/http/v11/headers/max_forwards.h"
#include "protocol/http/v11/headers/te.h"
#include "protocol/http/v11/headers/trailer.h"
#include "protocol/http/v11/headers/transfer_encoding.h"
#include "protocol/http/v11/headers/upgrade.h"
#include "protocol/http/v11/headers/via.h"
#include "protocol/http/v11/headers/x_forwarded_for.h"
#include "protocol/http/v11/headers/x_forwarded_host.h"
#include "protocol/http/v11/headers/x_forwarded_proto.h"
#include "protocol/http/v11/headers/rules/directives.h"
#include "protocol/http/v11/headers/rules/framing.h"
#include "protocol/http/v11/headers/rules/policy.h"
#include "protocol/http/v11/headers/rules/routing.h"
#include "protocol/http/v11/limits.h"
#include "protocol/http/v11/parsed_types.h"
#include "protocol/http/v11/rejection_reason.h"
#include "protocol/http/v11/verdict.h"

namespace martianlabs::doba::protocol::http::v11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] decoder                                                     ( class ) |
// +---------------------------------------------------------------------------+
// | This class holds for the http 1.1 decoder implementation.                 |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   RQty - request being used.                                              |
// |   RSty - response being used.                                             |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty>
class decoder {
  // +=========================================================================+
  // | [>] USINGs                                                  ( private ) |
  // +=========================================================================+
  using body_framer_t = std::variant<body::framer_chunked, body::framer_raw>;

 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  decoder()
      : buffer_(std::make_unique_for_overwrite<char[]>(
            limits::kDecodingBufferSize)) {}
  decoder(const decoder&) = delete;
  decoder(decoder&&) noexcept = delete;
  ~decoder() = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  decoder& operator=(const decoder&) = delete;
  decoder& operator=(decoder&&) noexcept = delete;
  // +=========================================================================+
  // | [>] accumulate                                               ( public ) |
  // +=========================================================================+
  std::size_t accumulate(char* const buffer, std::size_t size) {
    if (!buffer || !size) return 0;
    std::size_t space_left = limits::kDecodingBufferSize - off_;
    std::size_t bytes_to_copy = std::min(space_left, size);
    std::memcpy(buffer_.get() + off_, buffer, bytes_to_copy);
    off_ += bytes_to_copy;
    return bytes_to_copy;
  }
  // +=========================================================================+
  // | [>] deserialize                                              ( public ) |
  // +=========================================================================+
  deserialization_result<RQty> deserialize() {
    deserialization_result<RQty> result =
        body_framer_ ? parse_body() : parse_core();
    // Any rejection reason recorded along the way (by a header interpreter,
    // a transversal rule, or the HTTP-version check) is surfaced here, at the
    // single point where every parsing path converges, so callers only ever
    // have to look at the top-level result.
    if (result.code == deserialization_status::kInvalidSource) {
      result.reason = static_cast<int>(context_.rejection_reason);
    }
    return result;
  }

 private:
  // +=========================================================================+
  // | [>] parse_core                                               ( public ) |
  // +=========================================================================+
  deserialization_result<RQty> parse_core() {
    reset_decoding_attributes();
    std::string_view sv(buffer_.get(), off_);
    std::size_t i = 0;
    // +-----------------------------------------------------------------------+
    // | request-line = method SP request-target SP HTTP-version               |
    // +-----------------------------------------------------------------------+
    // +-----------------------------------------------------------------------+
    // | [method] part!                                                        |
    // +-----------------------------------------------------------------------+
    if (!off_) return deserialization_status::kMoreBytesNeeded;
    method_ = helpers::consume_token(sv);
    if (method_.empty()) return deserialization_status::kInvalidSource;
    i += method_.size();
    if (i >= off_) return deserialization_status::kMoreBytesNeeded;
    if (sv[i++] != ' ') return deserialization_status::kInvalidSource;
    // +-----------------------------------------------------------------------+
    // | [request-target] part!                                                |
    // +-----------------------------------------------------------------------+
    // | request-target = origin-form                                          |
    // |                / absolute-form                                        |
    // |                / authority-form                                       |
    // |                / asterisk-form                                        |
    // +-----------------------------------------------------------------------+
    if (i >= off_) return deserialization_status::kMoreBytesNeeded;
    deserialization_status status;
    std::size_t bytes_used = 0;
    if (method_ == method_names::kConnect) {
      // Let's try to parse the request-target as authority-form, which is the
      // only valid form for the CONNECT method.
      std::string_view authority_host, authority_port;
      helpers::host_type authority_type;
      status = helpers::try_to_deserialize_as_authority_form(
          sv.substr(i), authority_host, authority_port, authority_type,
          bytes_used);
      if (status == deserialization_status::kSucceeded) {
        target_ = target::kAuthorityForm;
        context_.has_target_authority = true;
        context_.target_authority = {authority_host, authority_port,
                                     authority_type, {}};
      }
    } else if (method_ == method_names::kOptions && sv[i] == '*') {
      // Let's try to parse the request-target as asterisk-form, which is the
      // only valid form for the OPTIONS method with "*".
      status = helpers::try_to_deserialize_as_asterisk_form(sv.substr(i),
                                                            bytes_used);
      if (status == deserialization_status::kSucceeded) {
        target_ = target::kAsteriskForm;
      }
    } else {
      // Let's try to parse the request-target as origin-form, which is the most
      // common form for all other methods.
      status = helpers::try_to_deserialize_as_origin_form(
          sv.substr(i), absolute_path_, query_, bytes_used);
      if (status == deserialization_status::kSucceeded) {
        target_ = target::kOriginForm;
      } else if (status == deserialization_status::kInvalidSource) {
        // If we couldn't parse the request-target as origin-form, let's try to
        // parse it as absolute-form, which is the only other valid form for all
        // other methods.
        bool has_authority = false;
        std::string_view authority_host, authority_port, authority_scheme;
        helpers::host_type authority_type;
        status = helpers::try_to_deserialize_as_absolute_form(
            sv.substr(i), absolute_path_, query_, has_authority, authority_host,
            authority_port, authority_type, authority_scheme, bytes_used);
        if (status == deserialization_status::kSucceeded) {
          target_ = target::kAbsoluteForm;
          if (has_authority) {
            context_.has_target_authority = true;
            context_.target_authority = {authority_host, authority_port,
                                         authority_type, authority_scheme};
          }
        }
      }
    }
    if (status != deserialization_status::kSucceeded) return status;
    if (context_.policies.max_uri_length &&
        bytes_used > context_.policies.max_uri_length) {
      context_.rejection_reason = rejection_reason::kUriTooLong;
      return deserialization_status::kInvalidSource;
    }
    // Validate that every "%HH" triplet in the path decodes to a non-NUL
    // byte. The decoder never mutates its own source buffer ('buffer_'); the
    // actual decoding into the resulting path happens later, once ownership
    // has moved to a buffer the 'request' instance controls (see
    // 'request::request').
    if (!absolute_path_.empty() &&
        !helpers::percent_decode_validate(absolute_path_)) {
      context_.rejection_reason = rejection_reason::kSyntax;
      return deserialization_status::kInvalidSource;
    }
    i += bytes_used;
    if (i >= off_) return deserialization_status::kMoreBytesNeeded;
    if (sv[i++] != ' ') return deserialization_status::kInvalidSource;
    // +-----------------------------------------------------------------------+
    // | [HTTP-version] part!                                                  |
    // +----------------+------------------------------------------------------+
    // | HTTP-version   | HTTP-name "/" DIGIT "." DIGIT                        |
    // | HTTP-name      | %s"HTTP"                                             |
    // +----------------+------------------------------------------------------+
    constexpr std::string_view kHttpVersion = "HTTP/0.0";
    constexpr std::size_t kHttpVersionLength = kHttpVersion.size();
    const std::size_t available = std::min(kHttpVersionLength, off_ - i);
    for (std::size_t version_index = 0; version_index < available;
         version_index++) {
      const char expected = kHttpVersion[version_index];
      if ((expected == '0' && !helpers::is_digit(sv[i + version_index])) ||
          (expected != '0' && sv[i + version_index] != expected)) {
        return deserialization_status::kInvalidSource;
      }
    }
    if (available < kHttpVersionLength) {
      return deserialization_status::kMoreBytesNeeded;
    }
    if (sv[i + 5] != '1' || sv[i + 7] != '1') {
      // The grammar is well-formed but the version is not HTTP/1.1. A
      // numerically higher version (e.g. 1.2, 2.0) is a request this server
      // simply does not speak yet, which maps to 505 HTTP Version Not
      // Supported. Anything at or below HTTP/1.0 is out of scope for now and
      // stays a plain 400.
      if (sv[i + 5] > '1' || (sv[i + 5] == '1' && sv[i + 7] > '1')) {
        context_.rejection_reason = rejection_reason::kVersionNotSupported;
      }
      return deserialization_status::kInvalidSource;
    }
    i += 8;
    if (i >= off_) return deserialization_status::kMoreBytesNeeded;
    if (sv[i] != '\r') return deserialization_status::kInvalidSource;
    if (i + 1 >= off_) return deserialization_status::kMoreBytesNeeded;
    if (sv[i + 1] != '\n') {
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
    const std::size_t headers_start = i;
    while (i < off_) {
      if (context_.policies.max_header_section_size &&
          (i - headers_start) > context_.policies.max_header_section_size) {
        context_.rejection_reason = rejection_reason::kHeaderFieldsTooLarge;
        return deserialization_status::kInvalidSource;
      }
      if (!field_name_decoded) {
        if (sv[i] == '\r') {
          if (i != fn_start) return deserialization_status::kInvalidSource;
          if (i + 1 >= off_) return deserialization_status::kMoreBytesNeeded;
          if (sv[i + 1] != '\n') return deserialization_status::kInvalidSource;
          i += 2;
          // Every modelled header was already parsed once and interpreted in
          // place during the loop above, populating ctx and the connection
          // state. All that remains is to apply the transversal rules that no
          // single header can decide, over the fully populated context. Any
          // rejection fails the whole deserialization.
          if (headers::rules::framing::apply(context_) == verdict::kReject ||
              headers::rules::routing::apply(context_) == verdict::kReject ||
              headers::rules::directives::apply(context_) == verdict::kReject ||
              headers::rules::policy::apply(context_) == verdict::kReject) {
            // If any of the transversal rules reject the request, then the
            // source is invalid and we cannot parse the request!
            return deserialization_status::kInvalidSource;
          }
          // Check for the presence of a body and build the body writer for this
          // request. We only support chunked and raw framing, and only if the
          // request has a body.
          bool body_expected =
              context_.connection.chunked ||
              (context_.has_content_length && context_.content_length > 0);
          if (body_expected) {
            body_buffer_ = common::writer(common::byte_storage_options{
                .spill_threshold = 65535,  // 64 KiB!
                .spill_dir = {},
            });
            if (context_.connection.chunked) {
              body_framer_ = body::framer_chunked();
            } else {
              body_framer_ = body::framer_raw(context_.content_length);
            }
          }
          // Mount the request object and keep the result!
          mount_request_getter(i);
          // In case of no body writer, we are done and can return the request.
          // Otherwise, we need to continue parsing the body, so we remove the
          // already consumed bytes from the input and call parse() again to
          // continue parsing the body.
          if (!body_framer_) return dispatch(std::nullopt);
          // In case of a body writer, we need to continue parsing the body, so
          // we remove the already consumed bytes from the input and call
          // parse() again to continue parsing the body.
          deserialization_result<RQty> result = deserialize();
          // RFC 9110 S10.1.1: the client is waiting for an interim response
          // before sending the body, so it is handed to the transport here.
          // This branch runs once per request, right as the body starts.
          // It is deliberately skipped when the call above already completed
          // the message: an optimistic client may send Expect together with
          // the whole body, and once that body is in there is nothing left to
          // wait for, so the interim is omitted (the spec allows it) instead
          // of being emitted behind the data it was meant to precede.
          if (context_.connection.expects_continue &&
              result.code == deserialization_status::kMoreBytesNeeded) {
            result.interim = k100_continue_interim_;
          }
          return result;
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
      if (i >= off_) return deserialization_status::kMoreBytesNeeded;
      if (sv[i++] != ':') return deserialization_status::kInvalidSource;
      std::string_view field_name = sv.substr(fn_start, i - 1 - fn_start);
      std::size_t fv_start = i;
      while (i < off_) {
        if (sv[i] == '\r') break;
        if (!helpers::is_vchar(sv[i]) && !helpers::is_obs_text(sv[i]) &&
            !helpers::is_ows(sv[i])) {
          // If the next character is not a valid field-value character, then
          // the source is invalid because we have an incomplete header field!
          return deserialization_status::kInvalidSource;
        }
        i++;
      }
      if (i + 1 >= off_) return deserialization_status::kMoreBytesNeeded;
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
        if (itr_dispatch->second(field_value, context_) == verdict::kReject) {
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
  deserialization_result<RQty> parse_body() {
    body::framer_state state = std::visit(
        [this](auto& arg) -> body::framer_state {
          std::span<const std::byte> byte_span{
              reinterpret_cast<const std::byte*>(buffer_.get()), off_};
          return arg.write(byte_span, *body_buffer_);
        },
        *body_framer_);
    if (state.has_error || state.consumed > off_) {
      return deserialization_status::kInvalidSource;
    }
    std::memmove(buffer_.get(), buffer_.get() + state.consumed,
                 off_ - state.consumed);
    off_ -= state.consumed;
    if (!state.complete) return deserialization_status::kMoreBytesNeeded;
    return dispatch(body_buffer_->release());
  }
  // +=========================================================================+
  // | [>] mount_request                                            ( public ) |
  // +=========================================================================+
  void mount_request_getter(std::size_t bytes_used) {
    // Only if the query part is not empty, we will split it into key-value
    // pairs and set it in the request.
    std::vector<query_parameter_view> query_parameters;
    if (!query_.empty()) {
      std::array<std::string_view, kMaxQueryParameters> keys;
      std::array<std::string_view, kMaxQueryParameters> values;
      std::size_t qc = helpers::split_query_parameters(query_, keys, values);
      query_parameters.reserve(qc);
      for (std::size_t q = 0; q < qc; q++) {
        query_parameters.emplace_back(keys[q], values[q]);
      }
    }
    // Check if the request has a host set it in the request.
    std::optional<std::string_view> host_host;
    std::optional<std::string_view> host_port;
    std::optional<helpers::host_type> host_type;
    if (context_.has_host) {
      host_host = context_.host.host;
      host_port = context_.host.port;
      host_type = context_.host.type;
    }
    // Check if the request has a target authority and set it in the request.
    std::optional<std::string_view> target_authority_host;
    std::optional<std::string_view> target_authority_port;
    std::optional<helpers::host_type> target_authority_type;
    if (context_.has_target_authority) {
      target_authority_host = context_.target_authority.host;
      target_authority_port = context_.target_authority.port;
      target_authority_type = context_.target_authority.type;
    }
    // Now that the request is fully validated, we can build the request object
    std::string_view buffer_view(buffer_.get(), bytes_used);
    request_getter_ = RQty::from(
        buffer_view, method_, absolute_path_, target_, headers_,
        query_parameters, host_host, host_port, host_type,
        target_authority_host, target_authority_port, target_authority_type,
        context_.connection.chunked, context_.content_length,
        context_.connection.close_requested);
    // Let's adjust the buffer to remove the bytes that were used!
    std::memmove(buffer_.get(), buffer_.get() + bytes_used,
                 off_ - bytes_used);
    off_ -= bytes_used;
  }
  // +=========================================================================+
  // | [>] dispatch                                                ( private ) |
  // +=========================================================================+
  deserialization_result<RQty> dispatch(
      std::optional<common::byte_storage> byte_storage) {
    // Let's return the request object!
    deserialization_result<RQty> result = deserialization_result<RQty>(
        request_getter_(std::move(byte_storage)),
        context_.connection.close_requested ? channel_intent::kClose
                                            : channel_intent::kKeep);
    reset_decoding_attributes();
    return result;
  }
  // +=========================================================================+
  // | [>] reset_decoding_attributes                               ( private ) |
  // +=========================================================================+
  void reset_decoding_attributes() {
    method_ = {};
    target_ = target::kUnknown;
    absolute_path_ = {};
    query_ = {};
    headers_ = {};
    context_ = {};
    body_framer_ = std::nullopt;
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
                                      protocol::http::v11::context&);
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
  static verdict dispatch_host(std::string_view host_content,
                               context& context_rules) {
    if (context_rules.has_host) context_rules.multiple_host = true;
    context_rules.has_host = true;
    parsed_host_port parsed;
    if (!headers::host::check(host_content, parsed)) {
      return verdict::kReject;
    }
    context_rules.host = parsed;
    return headers::host::interpret(parsed, context_rules.connection,
                                    context_rules.policies);
  }
  // +=========================================================================+
  // | [>] dispatch_content_length (modelled header)               ( private ) |
  // +=========================================================================+
  static verdict dispatch_content_length(
      std::string_view content_length_content, context& context_rules) {
    std::size_t parsed = 0;
    if (!headers::content_length::check(content_length_content, parsed)) {
      return verdict::kReject;
    }
    if (context_rules.has_content_length) {
      context_rules.multiple_content_length = true;
    }
    context_rules.has_content_length = true;
    context_rules.content_length = parsed;
    verdict result = headers::content_length::interpret(
        parsed, context_rules.connection, context_rules.policies);
    if (result == verdict::kReject) {
      context_rules.rejection_reason = rejection_reason::kPayloadTooLarge;
    }
    return result;
  }
  // +=========================================================================+
  // | [>] dispatch_transfer_encoding (modelled header)            ( private ) |
  // +=========================================================================+
  static verdict dispatch_transfer_encoding(
      std::string_view transfer_encoding_content, context& context_rules) {
    parsed_parameter_list parsed;
    if (!headers::transfer_encoding::check(transfer_encoding_content, parsed)) {
      return verdict::kReject;
    }
    context_rules.has_transfer_encoding = true;
    verdict result = headers::transfer_encoding::interpret(
        parsed, context_rules.connection, context_rules.policies);
    if (result == verdict::kReject) {
      context_rules.rejection_reason = rejection_reason::kUnsupportedFeature;
    }
    return result;
  }
  // +=========================================================================+
  // | [>] dispatch_connection (modelled header)                   ( private ) |
  // +=========================================================================+
  static verdict dispatch_connection(std::string_view connection_content,
                                     context& context_rules) {
    parsed_token_list parsed;
    if (!headers::connection::check(connection_content, parsed)) {
      return verdict::kReject;
    }
    return headers::connection::interpret(parsed, context_rules.connection,
                                          context_rules.policies);
  }
  // +=========================================================================+
  // | [>] dispatch_te (modelled header)                           ( private ) |
  // +=========================================================================+
  static verdict dispatch_te(std::string_view te_content,
                             context& context_rules) {
    parsed_parameter_list parsed;
    if (!headers::te::check(te_content, parsed)) {
      return verdict::kReject;
    }
    return headers::te::interpret(parsed, context_rules.connection,
                                  context_rules.policies);
  }
  // +=========================================================================+
  // | [>] dispatch_trailer (modelled header)                      ( private ) |
  // +=========================================================================+
  static verdict dispatch_trailer(std::string_view trailer_content,
                                  context& context_rules) {
    parsed_token_list parsed;
    if (!headers::trailer::check(trailer_content, parsed)) {
      return verdict::kReject;
    }
    return headers::trailer::interpret(parsed, context_rules.connection,
                                       context_rules.policies);
  }
  // +=========================================================================+
  // | [>] dispatch_expect (modelled header)                       ( private ) |
  // +=========================================================================+
  static verdict dispatch_expect(std::string_view expect_content,
                                 context& context_rules) {
    parsed_parameter_list parsed;
    if (!headers::expect::check(expect_content, parsed)) {
      return verdict::kReject;
    }
    verdict result = headers::expect::interpret(
        parsed, context_rules.connection, context_rules.policies);
    if (result == verdict::kReject) {
      // RFC 9110 S10.1.1: an expectation this server cannot meet is answered
      // with 417, not with the generic 400 a syntactic failure would yield.
      context_rules.rejection_reason = rejection_reason::kExpectationFailed;
    }
    return result;
  }
  // +=========================================================================+
  // | [>] dispatch_upgrade (modelled header)                      ( private ) |
  // +=========================================================================+
  static verdict dispatch_upgrade(std::string_view upgrade_content,
                                  context& context_rules) {
    parsed_token_list parsed_content;
    if (!headers::upgrade::check(upgrade_content, parsed_content)) {
      return verdict::kReject;
    }
    verdict result = headers::upgrade::interpret(
        parsed_content, context_rules.connection, context_rules.policies);
    if (result == verdict::kReject) {
      context_rules.rejection_reason = rejection_reason::kUnsupportedFeature;
    }
    return result;
  }
  // +=========================================================================+
  // | [>] dispatch_max_forwards (modelled header)                 ( private ) |
  // +=========================================================================+
  static verdict dispatch_max_forwards(std::string_view max_forwards_content,
                                       context& context_rules) {
    std::size_t parsed = 0;
    if (!headers::max_forwards::check(max_forwards_content, parsed)) {
      return verdict::kReject;
    }
    return headers::max_forwards::interpret(parsed, context_rules.connection,
                                            context_rules.policies);
  }
  // +=========================================================================+
  // | [>] dispatch_via (modelled header)                          ( private ) |
  // +=========================================================================+
  static verdict dispatch_via(std::string_view via_content,
                              context& context_rules) {
    parsed_via_list parsed;
    if (!headers::via::check(via_content, parsed)) {
      return verdict::kReject;
    }
    context_rules.forwarding_hops += parsed.elements.size();
    return headers::via::interpret(parsed, context_rules.connection,
                                   context_rules.policies);
  }
  // +=========================================================================+
  // | [>] dispatch_forwarded (modelled header)                    ( private ) |
  // +=========================================================================+
  static verdict dispatch_forwarded(std::string_view forwarded_content,
                                    context& context_rules) {
    parsed_forwarded_list parsed;
    if (!headers::forwarded::check(forwarded_content, parsed)) {
      return verdict::kReject;
    }
    context_rules.forwarding_hops += parsed.elements.size();
    return headers::forwarded::interpret(parsed, context_rules.connection,
                                         context_rules.policies);
  }
  // +=========================================================================+
  // | [>] dispatch_x_forwarded_for (modelled header)              ( private ) |
  // +=========================================================================+
  static verdict dispatch_x_forwarded_for(
      std::string_view x_forwarded_for_content, context& context_rules) {
    parsed_token_list parsed;
    if (!headers::x_forwarded_for::check(x_forwarded_for_content, parsed)) {
      return verdict::kReject;
    }
    context_rules.forwarding_hops += parsed.elements.size();
    return headers::x_forwarded_for::interpret(parsed, context_rules.connection,
                                               context_rules.policies);
  }
  // +=========================================================================+
  // | [>] dispatch_x_forwarded_host (modelled header)             ( private ) |
  // +=========================================================================+
  static verdict dispatch_x_forwarded_host(
      std::string_view x_forwarded_host_content, context& context_rules) {
    parsed_host_port parsed;
    if (!headers::x_forwarded_host::check(x_forwarded_host_content, parsed)) {
      return verdict::kReject;
    }
    return headers::x_forwarded_host::interpret(
        parsed, context_rules.connection, context_rules.policies);
  }
  // +=========================================================================+
  // | [>] dispatch_x_forwarded_proto (modelled header)            ( private ) |
  // +=========================================================================+
  static verdict dispatch_x_forwarded_proto(
      std::string_view x_forwarded_proto_content, context& context_rules) {
    parsed_token_list parsed;
    if (!headers::x_forwarded_proto::check(x_forwarded_proto_content, parsed)) {
      return verdict::kReject;
    }
    return headers::x_forwarded_proto::interpret(
        parsed, context_rules.connection, context_rules.policies);
  }
  // +=========================================================================+
  // | [>] CONSTANTs                                               ( private ) |
  // +=========================================================================+
  static constexpr std::size_t kMaxQueryParameters =
      limits::kMaxQueryParameters;
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
           &dispatch<http::headers::content_type>},
          {"Content-Encoding",  // check only!
           &dispatch<http::headers::content_encoding>},
          {"Date",  // check only!
           &dispatch<http::headers::date>},
          {"Accept",  // check only!
           &dispatch<http::headers::accept>},
          {"Accept-Encoding",  // check only!
           &dispatch<http::headers::accept_encoding>},
          {"Accept-Language",  // check only!
           &dispatch<http::headers::accept_language>},
          {"Content-Language",  // check only!
           &dispatch<http::headers::content_language>},
          {"Content-Location",  // check only!
           &dispatch<http::headers::content_location>},
          {"Range",  // check only!
           &dispatch<http::headers::range>},
          {"Content-Range",  // check only!
           &dispatch<http::headers::content_range>},
          {"Accept-Ranges",  // check only!
           &dispatch<http::headers::accept_ranges>},
          {"If-Range",  // check only!
           &dispatch<http::headers::if_range>},
          {"ETag",  // check only!
           &dispatch<http::headers::etag>},
          {"Last-Modified",  // check only!
           &dispatch<http::headers::last_modified>},
          {"If-Match",  // check only!
           &dispatch<http::headers::if_match>},
          {"If-None-Match",  // check only!
           &dispatch<http::headers::if_none_match>},
          {"If-Modified-Since",  // check only!
           &dispatch<http::headers::if_modified_since>},
          {"If-Unmodified-Since",  // check only!
           &dispatch<http::headers::if_unmodified_since>},
          {"Cache-Control",  // check only!
           &dispatch<http::headers::cache_control>},
          {"Vary",  // check only!
           &dispatch<http::headers::vary>},
          {"Age",  // check only!
           &dispatch<http::headers::age>},
          {"Expires",  // check only!
           &dispatch<http::headers::expires>},
          {"Pragma",  // check only!
           &dispatch<http::headers::pragma>},
          {"Location",  // check only!
           &dispatch<http::headers::location>},
          {"Allow",  // check only!
           &dispatch<http::headers::allow>},
          {"Retry-After",  // check only!
           &dispatch<http::headers::retry_after>},
          {"Authorization",  // check only!
           &dispatch<http::headers::authorization>},
          {"WWW-Authenticate",  // check only!
           &dispatch<http::headers::www_authenticate>},
          {"Authentication-Info",  // check only!
           &dispatch<http::headers::authentication_info>},
          {"Cookie",  // check only!
           &dispatch<http::headers::cookie>},
          {"Set-Cookie",  // check only!
           &dispatch<http::headers::set_cookie>},
          {"User-Agent",  // check only!
           &dispatch<http::headers::user_agent>},
          {"Server",  // check only!
           &dispatch<http::headers::server>},
          {"Referer",  // check only!
           &dispatch<http::headers::referer>},
          {"Max-Forwards",  // check only!
           &dispatch_max_forwards},
          {"From",  // check only!
           &dispatch<http::headers::from>},
          {"Accept-Charset",  // check only!
           &dispatch<http::headers::accept_charset>},
          {"Origin",  // check only!
           &dispatch<http::headers::origin>},
          {"Access-Control-Request-Method",  // check only!
           &dispatch<http::headers::access_control_request_method>},
          {"Access-Control-Request-Headers",  // check only!
           &dispatch<http::headers::access_control_request_headers>},
          {"Access-Control-Allow-Origin",  // check only!
           &dispatch<http::headers::access_control_allow_origin>},
          {"Access-Control-Allow-Methods",  // check only!
           &dispatch<http::headers::access_control_allow_methods>},
          {"Access-Control-Allow-Headers",  // check only!
           &dispatch<http::headers::access_control_allow_headers>},
          {"Access-Control-Allow-Credentials",  // check only!
           &dispatch<http::headers::access_control_allow_credentials>},
          {"Access-Control-Expose-Headers",  // check only!
           &dispatch<http::headers::access_control_expose_headers>},
          {"Access-Control-Max-Age",  // check only!
           &dispatch<http::headers::access_control_max_age>},
          {"Sec-WebSocket-Key",  // check only!
           &dispatch<http::headers::sec_websocket_key>},
          {"Sec-WebSocket-Accept",  // check only!
           &dispatch<http::headers::sec_websocket_accept>},
          {"Sec-WebSocket-Version",  // check only!
           &dispatch<http::headers::sec_websocket_version>},
          {"Sec-WebSocket-Protocol",  // check only!
           &dispatch<http::headers::sec_websocket_protocol>},
          {"Sec-WebSocket-Extensions",  // check only!
           &dispatch<http::headers::sec_websocket_extensions>},
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
           &dispatch<http::headers::x_keep_alive>},
          {"Proxy-Connection",  // check only!
           &dispatch<http::headers::x_proxy_connection>},
  };
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  // A complete interim response: the status-line already carries its CRLF, so
  // only the empty line that closes the (absent) header section is added here.
  static constexpr char k100_continue_interim_[] =
      "HTTP/1.1 100 Continue\r\n\r\n";
  std::unique_ptr<char[]> buffer_;
  std::optional<common::writer> body_buffer_ = std::nullopt;
  std::optional<body_framer_t> body_framer_ = std::nullopt;
  std::size_t off_ = 0;
  context context_;
  std::string_view query_;
  std::string_view method_;
  std::string_view absolute_path_;
  target target_ = target::kUnknown;
  std::vector<header_view> headers_;
  request_getter<RQty> request_getter_;
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
