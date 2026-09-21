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

#ifndef martianlabs_doba_protocol_http_v11_engine_h
#define martianlabs_doba_protocol_http_v11_engine_h

#include <cstddef>
#include <functional>
#include <string_view>
#include <utility>

#include "common/output.h"
#include "protocol/deserialization.h"
#include "protocol/http/common/header_names.h"
#include "protocol/http/common/helpers.h"
#include "protocol/http/common/method_names.h"
#include "protocol/http/common/router.h"
#include "protocol/http/v11/decoder.h"
#include "protocol/http/v11/policies.h"

namespace martianlabs::doba::protocol::http::v11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] engine                                                      ( class ) |
// +---------------------------------------------------------------------------+
// | HTTP/1.1 protocol engine.                                                 |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty, typename ROty = router<RQty, RSty>>
class engine {
 public:
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +=========================================================================+
  using policies_type = v11::policies;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  engine(policies_type configuration, const ROty& routes)
      : decoder_{configuration}, router_{routes} {}
  // +=========================================================================+
  // | [>] set_on_send                                              ( public ) |
  // +=========================================================================+
  void set_on_send(common::send_delegate output) {
    on_send_ = std::move(output);
  }
  // +=========================================================================+
  // | [>] set_on_close                                             ( public ) |
  // +=========================================================================+
  void set_on_close(std::function<void()> close) {
    on_close_ = std::move(close);
  }
  // +=========================================================================+
  // | [>] on_send_completed                                        ( public ) |
  // +=========================================================================+
  void on_send_completed(bool) {}
  // +=========================================================================+
  // | [>] on_bytes_received                                        ( public ) |
  // +=========================================================================+
  std::size_t on_bytes_received(const char* buffer, const std::size_t size,
                                const std::size_t capacity) {
    if (stopped_) return size;
    std::size_t processed = 0;
    while (processed < size) {
      std::size_t consumed = 0;
      deserialization_result<RQty> result = decoder_.deserialize(
          buffer + processed, size - processed, capacity, consumed);
      processed += consumed;
      switch (result.code) {
        case deserialization_status::kSucceeded: {
          try {
            RSty response = process_request(*result.request);
            send_response(*result.request, response);
          } catch (...) {
            stopped_ = true;
            on_close_();
          }
          if (stopped_) return processed;
          break;
        }
        case deserialization_status::kInvalidSource:
          stopped_ = true;
          on_close_();
          return processed;
        case deserialization_status::kMoreBytesNeeded:
          return processed;
      }
    }
    return processed;
  }

 private:
  // +=========================================================================+
  // | [>] process_request                                         ( private ) |
  // +=========================================================================+
  RSty process_request(const RQty& request) {
    try {
      switch (request.get_target()) {
        case target::kOriginForm:
        case target::kAbsoluteForm: {
          const std::string_view path = request.get_absolute_path();
          const auto match = router_.match(request.get_method(), path);
          if (match.handler) {
            if (match.handler->is_async()) return RSty::not_implemented_501();
            return match.handler->callback(request);
          }
          if (match.parametrized_handler) {
            if (match.parametrized_handler->is_async()) {
              return RSty::not_implemented_501();
            }
            return match.parametrized_handler->invoke(request, path);
          }
          const auto allowed = router_.allowed_methods(path);
          if (allowed.empty()) return RSty::not_found_404();
          // RFC 9110 S15.5.6: a 405 response must advertise allowed methods.
          RSty response = RSty::method_not_allowed_405();
          response.set_header(header_names::kAllow, allowed);
          return response;
        }
        case target::kAuthorityForm:
          return RSty::not_implemented_501();
        case target::kAsteriskForm:
          return RSty::ok_200();
        default:
          return RSty::bad_request_400();
      }
    } catch (...) {
      return RSty::internal_server_error_500();
    }
  }
  // +=========================================================================+
  // | [>] send_response                                           ( private ) |
  // +=========================================================================+
  void send_response(const RQty& request, RSty& response) {
    bool close = request.wants_connection_close();
    if (close) {
      response.set_header(header_names::kConnection, "close");
    } else if (response.has_header(header_names::kConnection)) {
      const std::size_t count = response.get_headers_length();
      for (std::size_t i = 0; i < count && !close; i++) {
        const auto header = response.get_header(i);
        if (!helpers::iequals(header.first, header_names::kConnection)) {
          continue;
        }
        helpers::for_each_list_element(
            header.second, [&close](std::string_view value) {
              helpers::ows_ltrim(value);
              helpers::ows_rtrim(value);
              if (helpers::iequals(value, "close")) close = true;
              return true;
            });
      }
    }
    // RFC 9110 S9.3.2: HEAD preserves framing but never sends body bytes.
    if (request.get_method() == method_names::kHead) response.suppress_body();
    auto serialized = response.serialize();
    on_send_(std::move(serialized->prefix), serialized->prefix_size,
             std::move(serialized->source));
    if (close && !stopped_) {
      stopped_ = true;
      on_close_();
    }
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  decoder<RQty, RSty> decoder_;
  const ROty& router_;
  common::send_delegate on_send_;
  std::function<void()> on_close_;
  bool stopped_{false};
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
