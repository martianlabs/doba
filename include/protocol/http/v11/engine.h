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
#include <memory>
#include <optional>
#include <span>
#include <string_view>
#include <utility>

#include "protocol/send_delegate.h"
#include "protocol/serialization.h"
#include "protocol/deserialization.h"
#include "protocol/http/common/header_names.h"
#include "protocol/http/common/method_names.h"
#include "protocol/http/common/router.h"
#include "protocol/http/v11/decoder.h"
#include "protocol/http/v11/engine_async.h"
#include "protocol/http/v11/engine_sync.h"
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
  // +-------------------------------------------------------------------------+
  using policies_type = v11::policies;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  engine(policies_type configuration, const ROty& router)
      : router_{router}, decoder_{configuration} {}
  // +=========================================================================+
  // | [>] set_on_send                                              ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_send(protocol::send_delegate output) {
    on_send_ = std::move(output);
    if (async_) async_->set_on_send(on_send_);
  }
  // +=========================================================================+
  // | [>] set_on_close                                             ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_close(std::function<void()> close) {
    on_close_ = std::move(close);
    if (async_) async_->set_on_close(on_close_);
  }
  // +=========================================================================+
  // | [>] on_bytes_received                                        ( public ) |
  // +-------------------------------------------------------------------------+
  std::size_t on_bytes_received(const char* buffer, const std::size_t size,
                                const std::size_t capacity) {
    if (closed_) return size;
    if (!size) return 0;
    std::size_t total = 0;
    while (total < size && !closed_) {
      if (!async_) {
        while (total < size && !closed_) {
          std::size_t consumed = 0;
          try {
            deserialization_result result = decoder_.deserialize(
                buffer + total, size - total, capacity, consumed);
            if (result.code == deserialization_status::kSucceeded) {
              const RQty& request = *result.request;
              bool close = request.wants_connection_close();
              typename ROty::route_match match{};
              std::string_view path;
              if (request.get_target() == target::kOriginForm ||
                  request.get_target() == target::kAbsoluteForm) {
                path = request.get_absolute_path();
                match = router_.match(request.get_method(), path);
              }
              if constexpr (requires { match.async_handler; }) {
                if (match.async_handler || match.async_parametrized_handler) {
                  async_ = std::make_shared<engine_async>(
                      decoder_.configuration().max_pending_requests);
                  async_->set_on_send(on_send_);
                  async_->set_on_close(on_close_);
                  const auto position = request_position();
                  if (!position) return size;
                  dispatch_async(std::move(*result.request), match, path,
                                 *position, close);
                  if (close) closed_ = true;
                  total += consumed;
                  break;
                }
              }
              RSty response = decoder_.make_response();
              sync_type::execute(router_, request, response, close, &match);
              auto serialized = sync_type::serialize(request, response, close);
              on_send_(serialized.head, serialized.body,
                       std::move(serialized.source));
              if (close) query_for_close();
              total += consumed;
              continue;
            }
            if (result.code == deserialization_status::kInvalidSource) {
              if (result.response) {
                auto serialized = sync_type::serialize(*result.response, true);
                on_send_(serialized.head, serialized.body,
                         std::move(serialized.source));
                query_for_close();
              }
              return size;
            }
            if (result.response) {
              auto serialized = sync_type::serialize(*result.response, false);
              on_send_(serialized.head, serialized.body,
                       std::move(serialized.source));
            }
            return total + consumed;
          } catch (...) {
            query_for_close();
            return total + consumed;
          }
        }
        continue;
      }
      if (!async_->accepting()) return size;
      if (async_->idle()) {
        async_.reset();
        continue;
      }
      while (total < size && !closed_ && async_->accepting()) {
        if (async_->idle()) {
          async_.reset();
          break;
        }
        std::size_t consumed = 0;
        try {
          deserialization_result result = decoder_.deserialize(
              buffer + total, size - total, capacity, consumed);
          switch (result.code) {
            case deserialization_status::kSucceeded: {
              const RQty& request = *result.request;
              bool close = request.wants_connection_close();
              typename ROty::route_match match{};
              std::string_view path;
              if (request.get_target() == target::kOriginForm ||
                  request.get_target() == target::kAbsoluteForm) {
                path = request.get_absolute_path();
                match = router_.match(request.get_method(), path);
              }
              const auto position = request_position();
              if (!position) return size;
              if constexpr (requires { match.async_handler; }) {
                if (match.async_handler || match.async_parametrized_handler) {
                  dispatch_async(std::move(*result.request), match, path,
                                 *position, close);
                  if (close) closed_ = true;
                  break;
                }
              }
              RSty response = decoder_.make_response();
              sync_type::execute(router_, request, response, close, &match);
              enqueue_response(request, response, close, *position);
              break;
            }
            case deserialization_status::kInvalidSource:
              if (result.response) {
                const auto position = request_position();
                if (!position) return size;
                enqueue_response(*result.response, true, *position);
              }
              return size;
            case deserialization_status::kMoreBytesNeeded:
              if (result.response) {
                if (!interim_position_) {
                  interim_position_ = async_->reserve();
                  if (!interim_position_) return size;
                }
                enqueue_response(*result.response, false,
                                 *interim_position_, false);
              }
              return total + consumed;
          }
          total += consumed;
        } catch (...) {
          query_for_close();
          return total + consumed;
        }
      }
    }
    return total;
  }

 private:
  using sync_type = engine_sync<RQty, RSty, ROty>;

  std::optional<std::size_t> request_position() {
    if (interim_position_) {
      const std::size_t position = *interim_position_;
      interim_position_.reset();
      return position;
    }
    return async_->reserve();
  }

  struct async_request {
    std::unique_ptr<char[]> request_storage;
    RQty request;
    std::unique_ptr<char[]> response_storage;
    std::shared_ptr<void> parameters;
    RSty response;

    async_request(decoder<RQty, RSty>& owner,
                  std::unique_ptr<char[]> storage, RQty value)
        : request_storage(std::move(storage)),
          request(std::move(value)),
          response_storage(std::make_unique_for_overwrite<char[]>(
              owner.configuration().max_response_head_size +
              owner.configuration().response_body_inline_capacity)),
          response(owner.make_response(std::span<char>(
              response_storage.get(),
              owner.configuration().max_response_head_size +
                  owner.configuration().response_body_inline_capacity))) {}
  };

  void dispatch_async(RQty request, const typename ROty::route_match& match,
                      std::string_view path, std::size_t position, bool close) {
    auto entry = std::make_shared<async_request>(
        decoder_, decoder_.take_request_storage(), std::move(request));
    auto state = async_;
    try {
      common::task<void> work =
          match.async_handler
              ? (*match.async_handler)(entry->request, entry->response)
              : match.async_parametrized_handler->invoke(
                    entry->request, entry->response, path,
                    entry->parameters);
      router_.begin_async();
      state->begin();
      try {
        work.start([entry, state, routes = &router_, position, close](
                       std::exception_ptr error) mutable {
          try {
            if (error) {
              close = true;
              sync_type::build_error_response(entry->response);
            }
            auto serialized = sync_type::serialize(
                entry->request, entry->response, close);
            state->submit(position, std::move(serialized), true, close);
          } catch (...) {
            try {
              state->close_now();
            } catch (...) {
            }
          }
          state->end();
          routes->end_async();
        });
      } catch (...) {
        state->end();
        router_.end_async();
        throw;
      }
    } catch (...) {
      close = true;
      sync_type::build_error_response(entry->response);
      auto serialized = sync_type::serialize(entry->request, entry->response,
                                             close);
      state->submit(position, std::move(serialized), true, close);
    }
  }
  // +=========================================================================+
  // | [>] query_for_close                                         ( private ) |
  // +-------------------------------------------------------------------------+
  void query_for_close() {
    if (closed_) return;
    closed_ = true;
    on_close_();
  }
  // +=========================================================================+
  // | [>] enqueue_response                                        ( private ) |
  // +-------------------------------------------------------------------------+
  void enqueue_response(RSty& response, bool close, std::size_t position,
                        bool final = true) {
    auto serialized = sync_type::serialize(response, close);
    async_->submit(position, std::move(serialized), final, close);
    if (close) closed_ = true;
  }
  // +=========================================================================+
  // | [>] enqueue_response                                        ( private ) |
  // +-------------------------------------------------------------------------+
  void enqueue_response(const RQty& request, RSty& response, bool close,
                        std::size_t position) {
    auto serialized = sync_type::serialize(request, response, close);
    async_->submit(position, std::move(serialized), true, close);
    if (close) closed_ = true;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  const ROty& router_;  // Reference to the router for handling requests.
  decoder<RQty, RSty> decoder_;  // Decoder for processing incoming requests.
  std::shared_ptr<engine_async> async_;
  std::optional<std::size_t> interim_position_;
  protocol::send_delegate on_send_;
  std::function<void()> on_close_;
  bool closed_{false};
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
