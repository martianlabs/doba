//                              _       _
//                           __| | ___ | |__   __ _
//                          / _` |/ _ \| '_ \ / _` |
//                         | (_| | (_) | |_) | (_| |
//                          \__,_|\___/|_.__/ \__,_|
//
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/LICENSE-2.0
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
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "common/hash_map.h"
#include "protocol/http/v11/results.h"
#include "protocol/http/common/header.h"
#include "protocol/http/common/helpers.h"
#include "protocol/http/common/method_names.h"
#include "protocol/http/common/query_parameter.h"
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
#include "protocol/http/v11/body/reader.h"
#include "protocol/http/v11/context.h"
#include "protocol/http/v11/contracts.h"
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
#include "protocol/http/v11/parsed_types.h"
#include "protocol/http/v11/policies.h"
#include "protocol/http/v11/rejection_reason.h"
#include "protocol/http/v11/verdict.h"

namespace martianlabs::doba::protocol::http::v11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] decoder                                                     ( class ) |
// +---------------------------------------------------------------------------+
// | Decode HTTP/1.1 requests a piece at a time.                               |
// +---------------------------------------------------------------------------+
// | RQty and RSty are the request and response types.                         |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <contracts::request RQty, contracts::response RSty>
class decoder {
  // +=========================================================================+
  // | [>] USINGs                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  using body_framer_t = std::variant<body::framer_chunked, body::framer_raw>;

 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  explicit decoder(policies configuration = {}) {
    if (!configuration.max_request_head_size ||
        !configuration.max_response_head_size) {
      throw std::invalid_argument("Invalid HTTP head limits");
    }
    context_.policies = configuration;
  }
  decoder(const decoder&) = delete;
  decoder(decoder&&) noexcept = delete;
  ~decoder() = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  decoder& operator=(const decoder&) = delete;
  decoder& operator=(decoder&&) noexcept = delete;
  // +=========================================================================+
  // | [>] deserialize                                              ( public ) |
  // +-------------------------------------------------------------------------+
  // | Consume one message; keep only an unfinished one between calls.         |
  // +-------------------------------------------------------------------------+
  deserialization_result<RQty, RSty> deserialize(std::string_view& input) {
    if (!reminder_active_) reminder_.clear();
    if (input.empty()) return deserialization_status::kMoreBytesNeeded;
    const auto message = input;
    deserialization_result<RQty, RSty> result;
    if (!reminder_active_) {
      result = parse_core(input);
      if (result.code == deserialization_status::kMoreBytesNeeded) {
        const auto keep =
            body_framer_ && request_->body_is_stored() ? head_end_
            : body_framer_ ? message.size()
                           : std::min(message.size(),
                                      context_.policies.max_request_head_size);
        const auto previous = reinterpret_cast<std::uintptr_t>(message.data());
        reminder_.assign(message.data(), message.data() + keep);
        reminder_active_ = true;
        if (request_) request_->rebase_bytes(previous, reminder_.data());
        input = {};
      }
    } else if (body_framer_) {
      result = parse_body(input);
    } else {
      const auto take = head_fragment_size(input);
      reminder_.insert(reminder_.end(), input.data(), input.data() + take);
      input.remove_prefix(take);
      std::string_view head(reminder_.data(), reminder_.size());
      result = parse_core(head);
      if (result.code == deserialization_status::kMoreBytesNeeded &&
          body_framer_ && !input.empty()) {
        auto body_result = parse_body(input);
        if (body_result.code == deserialization_status::kMoreBytesNeeded) {
          body_result.response = std::move(result.response);
        }
        result = std::move(body_result);
      }
    }
    if (result.code == deserialization_status::kMoreBytesNeeded &&
        !body_framer_ &&
        (reminder_.size() >= context_.policies.max_request_head_size ||
         std::string_view(reminder_.data(), reminder_.size())
                 .find("\r\n\r\n") != std::string_view::npos)) {
      // A complete head can't need more syntax bytes. Don't wait for a later
      // colon to fix a malformed CONNECT target.
      if (reminder_.size() >= context_.policies.max_request_head_size)
        context_.rejection_reason = rejection_reason::kHeaderFieldsTooLarge;
      result.code = deserialization_status::kInvalidSource;
    }
    if (result.code == deserialization_status::kSucceeded) {
      if (body_framer_ &&
          !request_->finish_body(context_.connection.chunked,
                                 context_.content_length,
                                 context_.policies.max_chunk_extension_size,
                                 context_.policies.max_trailer_section_size)) {
        storage_failed_ = true;
        result.code = deserialization_status::kInvalidSource;
      } else {
        result.request.emplace(std::move(*request_));
      }
    }
    if (result.code == deserialization_status::kInvalidSource) {
      result.response.emplace(make_response());
      if (storage_failed_) {
        result.response->internal_server_error_500();
      } else {
        switch (context_.rejection_reason) {
          case rejection_reason::kPayloadTooLarge:
            result.response->content_too_large_413();
            break;
          case rejection_reason::kUriTooLong:
            result.response->uri_too_long_414();
            break;
          case rejection_reason::kExpectationFailed:
            result.response->expectation_failed_417();
            break;
          case rejection_reason::kHeaderFieldsTooLarge:
            result.response->request_header_fields_too_large_431();
            break;
          case rejection_reason::kUnsupportedFeature:
            result.response->not_implemented_501();
            break;
          case rejection_reason::kVersionNotSupported:
            result.response->http_version_not_supported_505();
            break;
          default:
            result.response->bad_request_400();
            break;
        }
      }
      result.response->set_body(storage_failed_ ? "Internal Server Error"
                                                : "Invalid request content!");
      // RFC 9110 S9.3.2: HEAD errors retain framing but omit content.
      if (head_request_) result.response->suppress_body();
    }
    if (result.code != deserialization_status::kMoreBytesNeeded ||
        !body_framer_) {
      request_.reset();
      body_framer_.reset();
      storage_failed_ = false;
      method_ = {};
      target_ = target::kUnknown;
      absolute_path_ = {};
      query_ = {};
      headers_.clear();
      query_parameters_.clear();
      const policies configuration = context_.policies;
      context_ = {};
      context_.policies = configuration;
    }
    if (result.code != deserialization_status::kMoreBytesNeeded) {
      reminder_active_ = false;
      if (result.code == deserialization_status::kInvalidSource) {
        reminder_.clear();
        input = {};
      }
    } else if (body_framer_ && request_->body_is_stored()) {
      reminder_.resize(head_end_);
    }
    return result;
  }
  // +=========================================================================+
  // | [>] configuration                                            ( public ) |
  // +-------------------------------------------------------------------------+
  // | Get the decoder settings.                                               |
  // +-------------------------------------------------------------------------+
  const policies& configuration() const noexcept { return context_.policies; }
  // +=========================================================================+
  // | [>] make_response                                            ( public ) |
  // +-------------------------------------------------------------------------+
  // | Build a response that owns its bytes and uses these limits.             |
  // +-------------------------------------------------------------------------+
  RSty make_response() {
    return RSty::from(context_.policies.max_response_head_size,
                      context_.policies.response_body_memory_threshold);
  }
  RSty make_response(std::string& head, std::string& body) {
    return RSty::from(context_.policies.max_response_head_size,
                      context_.policies.response_body_memory_threshold,
                      head, body);
  }

 private:
  // Find the first head terminator, even across input calls.
  // +-------------------------------------------------------------------------+
  // | [>] head_fragment_size                                       ( public ) |
  // +-------------------------------------------------------------------------+
  std::size_t head_fragment_size(std::string_view input) const noexcept {
    input = input.substr(
        0, context_.policies.max_request_head_size - reminder_.size());
    constexpr std::string_view end = "\r\n\r\n";
    std::size_t take = input.size();
    const std::string_view previous(reminder_.data(), reminder_.size());
    for (std::size_t split = 1; split < end.size() && split <= previous.size();
         ++split) {
      if (previous.ends_with(end.substr(0, split)) &&
          input.starts_with(end.substr(split))) {
        take = std::min(take, end.size() - split);
      }
    }
    const auto at = input.find(end);
    if (at != std::string_view::npos) take = std::min(take, at + end.size());
    return take;
  }
  // +=========================================================================+
  // | [>] parse_core                                               ( public ) |
  // +-------------------------------------------------------------------------+
  deserialization_result<RQty, RSty> parse_core(std::string_view& source) {
    head_request_ = false;
    std::string_view sv =
        source.substr(0, context_.policies.max_request_head_size);
    std::size_t i = 0;
    // +-----------------------------------------------------------------------+
    // | request-line = method SP request-target SP HTTP-version               |
    // +-----------------------------------------------------------------------+
    // +-----------------------------------------------------------------------+
    // | Read the method.                                                      |
    // +-----------------------------------------------------------------------+
    if (sv.empty()) return deserialization_status::kMoreBytesNeeded;
    method_ = helpers::consume_token(sv);
    if (method_.empty()) return deserialization_status::kInvalidSource;
    i += method_.size();
    if (i >= sv.size()) return deserialization_status::kMoreBytesNeeded;
    if (sv[i++] != ' ') return deserialization_status::kInvalidSource;
    head_request_ = method_ == method_names::kHead;
    // +-----------------------------------------------------------------------+
    // | Read the request target.                                              |
    // +-----------------------------------------------------------------------+
    // | request-target = origin-form                                          |
    // | / absolute-form                                                       |
    // | / authority-form                                                      |
    // | / asterisk-form                                                       |
    // +-----------------------------------------------------------------------+
    if (i >= sv.size()) return deserialization_status::kMoreBytesNeeded;
    deserialization_status status;
    std::size_t bytes_used = 0;
    if (method_ == method_names::kConnect) {
      // CONNECT needs authority-form.
      std::string_view authority_host, authority_port;
      helpers::host_type authority_type;
      status = helpers::try_to_deserialize_as_authority_form(
          sv.substr(i), authority_host, authority_port, authority_type,
          bytes_used);
      if (status == deserialization_status::kSucceeded) {
        target_ = target::kAuthorityForm;
        context_.has_target_authority = true;
        context_.target_authority = {
            authority_host, authority_port, authority_type, {}};
      }
    } else if (method_ == method_names::kOptions && sv[i] == '*') {
      // OPTIONS * needs asterisk-form.
      status = helpers::try_to_deserialize_as_asterisk_form(sv.substr(i),
                                                            bytes_used);
      if (status == deserialization_status::kSucceeded) {
        target_ = target::kAsteriskForm;
      }
    } else {
      // Try origin-form for the usual request path.
      status = helpers::try_to_deserialize_as_origin_form(
          sv.substr(i), absolute_path_, query_, bytes_used);
      if (status == deserialization_status::kSucceeded) {
        target_ = target::kOriginForm;
      } else if (status == deserialization_status::kInvalidSource) {
        // Not origin-form? Try absolute-form next.
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
    if (status != deserialization_status::kSucceeded) {
      // Until the target's space arrives, URI prefixes can change meaning.
      if (status == deserialization_status::kInvalidSource &&
          sv.find(' ', i) == std::string_view::npos) {
        return deserialization_status::kMoreBytesNeeded;
      }
      return status;
    }
    i += bytes_used;
    if (i >= sv.size()) return deserialization_status::kMoreBytesNeeded;
    if (sv[i++] != ' ') return deserialization_status::kInvalidSource;
    // Reject escaped NULs only once the target is closed.
    if (!absolute_path_.empty() &&
        !helpers::percent_decode_validate(absolute_path_)) {
      context_.rejection_reason = rejection_reason::kSyntax;
      return deserialization_status::kInvalidSource;
    }
    // +-----------------------------------------------------------------------+
    // | Read the HTTP version.                                                |
    // +-----------------------------------------------------------------------+
    // | HTTP-version   | HTTP-name "/" DIGIT "." DIGIT                        |
    // | HTTP-name      | %s"HTTP"                                             |
    // +-----------------------------------------------------------------------+
    constexpr std::string_view kHttpVersion = "HTTP/0.0";
    constexpr std::size_t kHttpVersionLength = kHttpVersion.size();
    const std::size_t available = std::min(kHttpVersionLength, sv.size() - i);
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
      // We don't speak versions above 1.1, so return 505. HTTP/1.0 and lower
      // are also out of scope, but get 400.
      if (sv[i + 5] > '1' || (sv[i + 5] == '1' && sv[i + 7] > '1')) {
        context_.rejection_reason = rejection_reason::kVersionNotSupported;
      }
      return deserialization_status::kInvalidSource;
    }
    i += 8;
    if (i >= sv.size()) return deserialization_status::kMoreBytesNeeded;
    if (sv[i] != '\r') return deserialization_status::kInvalidSource;
    if (i + 1 >= sv.size()) return deserialization_status::kMoreBytesNeeded;
    if (sv[i + 1] != '\n') {
      return deserialization_status::kInvalidSource;
    }
    i += 2;
    // +-----------------------------------------------------------------------+
    // | Read the headers.                                                     |
    // +-----------------------------------------------------------------------+
    // +-----------------------------------------------------------------------+
    // | Rule            | Definition                                          |
    // +-----------------------------------------------------------------------+
    // | header-field    | field-name ":" OWS field-value OWS                  |
    // | field-name      | token                                               |
    // | token           | 1*tchar                                             |
    // | tchar           | ALPHA / DIGIT / "!" / "#" / "$" / "%" / "&" / "'" / |
    // | | "*" / "+" / "-" / "." / "^" / "_" / "`" / "|" / "~"                 |
    // | field-value     | *( field-content )                                  |
    // | field-content   | field-vchar [ *( SP / HTAB ) field-vchar ]          |
    // | field-vchar     | VCHAR / obs-text                                    |
    // | OWS             | *( SP / HTAB )                                      |
    // | obs-fold        | CRLF 1*( SP / HTAB ) ; obsolete, not supported      |
    // +-----------------------------------------------------------------------+
    bool field_name_decoded = false;
    std::size_t fn_start = i;
    while (i < sv.size()) {
      if (!field_name_decoded) {
        if (sv[i] == '\r') {
          if (i != fn_start) return deserialization_status::kInvalidSource;
          if (i + 1 >= sv.size())
            return deserialization_status::kMoreBytesNeeded;
          if (sv[i + 1] != '\n') return deserialization_status::kInvalidSource;
          i += 2;
          // Headers have already been parsed and interpreted into ctx. Now
          // check the rules that need more than one header; any rejection fails
          // decoding.
          if (headers::rules::framing::apply(context_) == verdict::kReject ||
              headers::rules::routing::apply(context_) == verdict::kReject ||
              headers::rules::directives::apply(context_) == verdict::kReject ||
              headers::rules::policy::apply(context_) == verdict::kReject) {
            // A cross-header rule rejected the request; stop decoding.
            return deserialization_status::kInvalidSource;
          }
          if (!store_head(sv.substr(0, i))) {
            return deserialization_status::kInvalidSource;
          }
          head_end_ = i;
          source.remove_prefix(i);
          // Check for a body and choose its framing.
          bool body_expected =
              context_.connection.chunked ||
              (context_.has_content_length && context_.content_length > 0);
          if (body_expected) {
            if (!request_->begin_body(
                    context_.policies.request_body_memory_threshold,
                    context_.connection.chunked ? 0
                                                : context_.content_length)) {
              storage_failed_ = true;
              return deserialization_status::kInvalidSource;
            }
            if (context_.connection.chunked) {
              body_framer_ = body::framer_chunked(
                  context_.policies.max_request_body_size,
                  context_.policies.max_chunk_extension_size,
                  context_.policies.max_trailer_section_size);
            } else {
              body_framer_ = body::framer_raw(context_.content_length);
            }
          }
          if (!body_framer_) return deserialization_status::kSucceeded;
          auto result = parse_body(source);
          if (result.code == deserialization_status::kMoreBytesNeeded &&
              context_.connection.expects_continue) {
            result.response.emplace(make_response());
            result.response->continue_100();
          }
          return result;
        }
        if (sv[i] == '\n') return deserialization_status::kInvalidSource;
        // Read the field name.
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
      // Read the field value.
      if (i >= sv.size()) return deserialization_status::kMoreBytesNeeded;
      if (sv[i++] != ':') return deserialization_status::kInvalidSource;
      std::string_view field_name = sv.substr(fn_start, i - 1 - fn_start);
      std::size_t fv_start = i;
      while (i < sv.size()) {
        if (sv[i] == '\r') break;
        if (!helpers::is_vchar(sv[i]) && !helpers::is_obs_text(sv[i]) &&
            !helpers::is_ows(sv[i])) {
          // An invalid byte here leaves the field incomplete; reject it.
          return deserialization_status::kInvalidSource;
        }
        i++;
      }
      if (i + 1 >= sv.size()) return deserialization_status::kMoreBytesNeeded;
      if (sv[i + 0] != '\r' || sv[i + 1] != '\n') {
        return deserialization_status::kInvalidSource;
      }
      std::string_view field_value = sv.substr(fv_start, i - fv_start);
      helpers::ows_trim(field_value);
      // Check syntax once, parse modelled headers once, then run their
      // interpreters and record cross-header state in ctx. Reject semantic
      // failures.
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
  // +-------------------------------------------------------------------------+
  deserialization_result<RQty, RSty> parse_body(std::string_view& source) {
    body::framer_state state = std::visit(
        [source](auto& arg) -> body::framer_state {
          std::span<const std::byte> byte_span{
              reinterpret_cast<const std::byte*>(source.data()), source.size()};
          return arg.consume(byte_span);
        },
        *body_framer_);
    if (state.has_error || state.consumed > source.size()) {
      if (state.error == body::framer_error::chunked_size_limit_exceeded) {
        context_.rejection_reason = rejection_reason::kPayloadTooLarge;
      }
      return deserialization_status::kInvalidSource;
    }
    if (state.consumed) {
      auto bytes = source.substr(0, state.consumed);
      const auto threshold = context_.policies.request_body_memory_threshold;
      if (reminder_active_ && !request_->body_is_stored() && threshold &&
          request_->body_wire_size() <= threshold &&
          state.consumed <= threshold - request_->body_wire_size()) {
        const auto previous =
            reinterpret_cast<std::uintptr_t>(reminder_.data());
        const auto offset = reminder_.size();
        reminder_.insert(reminder_.end(), bytes.begin(), bytes.end());
        if (previous != reinterpret_cast<std::uintptr_t>(reminder_.data())) {
          request_->rebase_bytes(previous, reminder_.data());
        }
        bytes = {reminder_.data() + offset, state.consumed};
      }
      if (!request_->append_body(bytes)) {
        storage_failed_ = true;
        return deserialization_status::kInvalidSource;
      }
    }
    source.remove_prefix(state.consumed);
    if (!state.complete) return deserialization_status::kMoreBytesNeeded;
    return deserialization_status::kSucceeded;
  }
  // +=========================================================================+
  // | [>] store_head                                              ( private ) |
  // +-------------------------------------------------------------------------+
  bool store_head(std::string_view buffer_view) {
    if (buffer_view.size() > context_.policies.max_request_head_size) {
      context_.rejection_reason = rejection_reason::kHeaderFieldsTooLarge;
      return false;
    }
    // Split a nonempty query into name/value pairs for the request.
    query_parameters_.clear();
    if (!query_.empty()) {
      constexpr std::size_t kQueryParameterStackCapacity = 129;
      std::array<std::string_view, kQueryParameterStackCapacity> key_stack,
          value_stack;
      std::vector<std::string_view> key_extra, value_extra;
      const std::size_t possible = query_.size() / 2 + 1;
      const std::size_t limit = context_.policies.max_query_parameters;
      const std::size_t slots =
          limit && limit < possible ? limit + 1 : possible;
      std::span<std::string_view> keys = key_stack;
      std::span<std::string_view> values = value_stack;
      if (slots > keys.size()) {
        key_extra.resize(slots);
        value_extra.resize(slots);
        keys = key_extra;
        values = value_extra;
      } else {
        keys = keys.first(slots);
        values = values.first(slots);
      }
      const std::size_t qc =
          helpers::split_query_parameters(query_, keys, values);
      if (limit && qc > limit) return false;
      query_parameters_.reserve(qc);
      for (std::size_t q = 0; q < qc; q++) {
        query_parameters_.emplace_back(keys[q], values[q]);
      }
    }
    request_.emplace(RQty::from(
        method_, absolute_path_, target_, headers_, query_parameters_,
        context_.has_host ? std::optional(context_.host.host) : std::nullopt,
        context_.has_host ? std::optional(context_.host.port) : std::nullopt,
        context_.has_host ? std::optional(context_.host.type) : std::nullopt,
        context_.has_target_authority
            ? std::optional(context_.target_authority.host)
            : std::nullopt,
        context_.has_target_authority
            ? std::optional(context_.target_authority.port)
            : std::nullopt,
        context_.has_target_authority
            ? std::optional(context_.target_authority.type)
            : std::nullopt,
        context_.connection.close_requested, std::nullopt, true));
    return true;
  }
  // +=========================================================================+
  // | HTTP/1.1 SERVER HEADER CHECKLIST                                        |
  // +-------------------------------------------------------------------------+
  // | Header                                                     |  Supported |
  // +-------------------------------------------------------------------------+
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
  // +-------------------------------------------------------------------------+
  // +=========================================================================+
  // | [>] TYPEs                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  using header_dispatch = verdict (*)(std::string_view,
                                      protocol::http::v11::context&);
  // +=========================================================================+
  // | [>] dispatch                                                ( private ) |
  // +-------------------------------------------------------------------------+
  // | For unmodelled headers, check syntax and leave ctx alone. Reject bad    |
  // | values; accept the rest as-is. Each checker gets a direct, inlinable    |
  // | dispatcher.                                                             |
  // +-------------------------------------------------------------------------+
  template <typename CHty>
  static constexpr verdict dispatch(std::string_view sv, context&) {
    return CHty::check(sv) ? verdict::kAccept : verdict::kReject;
  }
  // +=========================================================================+
  // | [>] dispatch_conditional_date                               ( private ) |
  // +-------------------------------------------------------------------------+
  // | For unmodelled conditional dates, leave ctx alone and ignore invalid    |
  // | dates as RFC 9110 S13.1.3/S13.1.4 requires. Keep a direct dispatcher    |
  // | per checker.                                                            |
  // +-------------------------------------------------------------------------+
  static constexpr verdict dispatch_conditional_date(std::string_view,
                                                     context&) {
    return verdict::kAccept;
  }
  // +=========================================================================+
  // | [>] dispatch_host (modelled header)                         ( private ) |
  // +-------------------------------------------------------------------------+
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
  // +-------------------------------------------------------------------------+
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
  // +-------------------------------------------------------------------------+
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
  // +-------------------------------------------------------------------------+
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
  // +-------------------------------------------------------------------------+
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
  // +-------------------------------------------------------------------------+
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
  // +-------------------------------------------------------------------------+
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
  // +-------------------------------------------------------------------------+
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
  // +-------------------------------------------------------------------------+
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
  // +-------------------------------------------------------------------------+
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
  // +-------------------------------------------------------------------------+
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
  // +-------------------------------------------------------------------------+
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
  // +-------------------------------------------------------------------------+
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
  // +-------------------------------------------------------------------------+
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
  // +-------------------------------------------------------------------------+
  static const inline common::hash_map<std::string_view, header_dispatch>
      header_dispatchers_ = {
          {"Host",  // Check syntax and interpret.
           &dispatch_host},
          {"Content-Length",  // Check syntax and interpret.
           &dispatch_content_length},
          {"Transfer-Encoding",  // Check syntax and interpret.
           &dispatch_transfer_encoding},
          {"Connection",  // Check syntax and interpret.
           &dispatch_connection},
          {"TE",  // Check syntax and interpret.
           &dispatch_te},
          {"Trailer",  // Check syntax and interpret.
           &dispatch_trailer},
          {"Expect",  // Check syntax and interpret.
           &dispatch_expect},
          {"Upgrade",  // Check syntax and interpret.
           &dispatch_upgrade},
          {"Content-Type",  // Syntax only.
           &dispatch<http::headers::content_type>},
          {"Content-Encoding",  // Syntax only.
           &dispatch<http::headers::content_encoding>},
          {"Date",  // Syntax only.
           &dispatch<http::headers::date>},
          {"Accept",  // Syntax only.
           &dispatch<http::headers::accept>},
          {"Accept-Encoding",  // Syntax only.
           &dispatch<http::headers::accept_encoding>},
          {"Accept-Language",  // Syntax only.
           &dispatch<http::headers::accept_language>},
          {"Content-Language",  // Syntax only.
           &dispatch<http::headers::content_language>},
          {"Content-Location",  // Syntax only.
           &dispatch<http::headers::content_location>},
          {"Range",  // Syntax only.
           &dispatch<http::headers::range>},
          {"Content-Range",  // Syntax only.
           &dispatch<http::headers::content_range>},
          {"Accept-Ranges",  // Syntax only.
           &dispatch<http::headers::accept_ranges>},
          {"If-Range",  // Syntax only.
           &dispatch<http::headers::if_range>},
          {"ETag",  // Syntax only.
           &dispatch<http::headers::etag>},
          {"Last-Modified",  // Syntax only.
           &dispatch<http::headers::last_modified>},
          {"If-Match",  // Syntax only.
           &dispatch<http::headers::if_match>},
          {"If-None-Match",  // Syntax only.
           &dispatch<http::headers::if_none_match>},
          {"If-Modified-Since", &dispatch_conditional_date},
          {"If-Unmodified-Since", &dispatch_conditional_date},
          {"Cache-Control",  // Syntax only.
           &dispatch<http::headers::cache_control>},
          {"Vary",  // Syntax only.
           &dispatch<http::headers::vary>},
          {"Age",  // Syntax only.
           &dispatch<http::headers::age>},
          {"Expires",  // Syntax only.
           &dispatch<http::headers::expires>},
          {"Pragma",  // Syntax only.
           &dispatch<http::headers::pragma>},
          {"Location",  // Syntax only.
           &dispatch<http::headers::location>},
          {"Allow",  // Syntax only.
           &dispatch<http::headers::allow>},
          {"Retry-After",  // Syntax only.
           &dispatch<http::headers::retry_after>},
          {"Authorization",  // Syntax only.
           &dispatch<http::headers::authorization>},
          {"WWW-Authenticate",  // Syntax only.
           &dispatch<http::headers::www_authenticate>},
          {"Authentication-Info",  // Syntax only.
           &dispatch<http::headers::authentication_info>},
          {"Cookie",  // Syntax only.
           &dispatch<http::headers::cookie>},
          {"Set-Cookie",  // Syntax only.
           &dispatch<http::headers::set_cookie>},
          {"User-Agent",  // Syntax only.
           &dispatch<http::headers::user_agent>},
          {"Server",  // Syntax only.
           &dispatch<http::headers::server>},
          {"Referer",  // Syntax only.
           &dispatch<http::headers::referer>},
          {"Max-Forwards",  // Syntax only.
           &dispatch_max_forwards},
          {"From",  // Syntax only.
           &dispatch<http::headers::from>},
          {"Accept-Charset",  // Syntax only.
           &dispatch<http::headers::accept_charset>},
          {"Origin",  // Syntax only.
           &dispatch<http::headers::origin>},
          {"Access-Control-Request-Method",  // Syntax only.
           &dispatch<http::headers::access_control_request_method>},
          {"Access-Control-Request-Headers",  // Syntax only.
           &dispatch<http::headers::access_control_request_headers>},
          {"Access-Control-Allow-Origin",  // Syntax only.
           &dispatch<http::headers::access_control_allow_origin>},
          {"Access-Control-Allow-Methods",  // Syntax only.
           &dispatch<http::headers::access_control_allow_methods>},
          {"Access-Control-Allow-Headers",  // Syntax only.
           &dispatch<http::headers::access_control_allow_headers>},
          {"Access-Control-Allow-Credentials",  // Syntax only.
           &dispatch<http::headers::access_control_allow_credentials>},
          {"Access-Control-Expose-Headers",  // Syntax only.
           &dispatch<http::headers::access_control_expose_headers>},
          {"Access-Control-Max-Age",  // Syntax only.
           &dispatch<http::headers::access_control_max_age>},
          {"Sec-WebSocket-Key",  // Syntax only.
           &dispatch<http::headers::sec_websocket_key>},
          {"Sec-WebSocket-Accept",  // Syntax only.
           &dispatch<http::headers::sec_websocket_accept>},
          {"Sec-WebSocket-Version",  // Syntax only.
           &dispatch<http::headers::sec_websocket_version>},
          {"Sec-WebSocket-Protocol",  // Syntax only.
           &dispatch<http::headers::sec_websocket_protocol>},
          {"Sec-WebSocket-Extensions",  // Syntax only.
           &dispatch<http::headers::sec_websocket_extensions>},
          {"Via",  // Check syntax and interpret.
           &dispatch_via},
          {"Forwarded",  // Check syntax and interpret.
           &dispatch_forwarded},
          {"X-Forwarded-For",  // Check syntax and interpret.
           &dispatch_x_forwarded_for},
          {"X-Forwarded-Host",  // Check syntax and interpret.
           &dispatch_x_forwarded_host},
          {"X-Forwarded-Proto",  // Check syntax and interpret.
           &dispatch_x_forwarded_proto},
          {"Keep-Alive",  // Syntax only.
           &dispatch<http::headers::x_keep_alive>},
          {"Proxy-Connection",  // Syntax only.
           &dispatch<http::headers::x_proxy_connection>},
  };
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  std::vector<char> reminder_;
  bool reminder_active_{false};
  std::size_t head_end_{0};
  std::optional<RQty> request_;
  bool storage_failed_ = false;
  std::optional<body_framer_t> body_framer_ = std::nullopt;
  context context_;
  std::string_view query_;
  std::string_view method_;
  std::string_view absolute_path_;
  target target_ = target::kUnknown;
  std::vector<header_view> headers_;
  std::vector<query_parameter_view> query_parameters_;
  bool head_request_{false};
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
