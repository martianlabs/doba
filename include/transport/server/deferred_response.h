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

#ifndef martianlabs_doba_transport_server_deferred_response_h
#define martianlabs_doba_transport_server_deferred_response_h

#include <cstdint>
#include <memory>
#include <utility>

#include "protocol/serialization.h"
#include "transport/server/response_completion.h"

namespace martianlabs::doba::transport::server::detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] response_sender                                            ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename Pty>
class response_sender {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  response_sender() = default;
  response_sender(Pty publisher, uint64_t connection_key, uint64_t position)
      : publisher_{std::move(publisher)},
        connection_key_{connection_key},
        position_{position},
        pending_{true} {}
  response_sender(const response_sender&) = delete;
  response_sender(response_sender&& other) noexcept
      : publisher_{std::move(other.publisher_)},
        connection_key_{other.connection_key_},
        position_{other.position_},
        pending_{std::exchange(other.pending_, false)} {}
  ~response_sender() = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  response_sender& operator=(const response_sender&) = delete;
  response_sender& operator=(response_sender&&) noexcept = delete;
  // +=========================================================================+
  // | [>] complete                                                 ( public ) |
  // +=========================================================================+
  bool complete(
      std::unique_ptr<protocol::serialization_result> response) {
    if (!pending_ || !response) return false;
    pending_ = false;
    return publisher_.publish(response_completion{
        connection_key_, position_, std::move(response)});
  }
  // +=========================================================================+
  // | [>] valid                                                    ( public ) |
  // +=========================================================================+
  bool valid() const { return pending_; }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  Pty publisher_;
  uint64_t connection_key_{0};
  uint64_t position_{0};
  bool pending_{false};
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] deferred_response_context                                  ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename Cty, typename Mty>
class deferred_response_context {
 public:
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +=========================================================================+
  using sender = response_sender<typename Mty::publisher>;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  deferred_response_context(Cty& context, Mty& mailbox)
      : context_{context}, mailbox_{mailbox} {}
  deferred_response_context(const deferred_response_context&) = delete;
  deferred_response_context(deferred_response_context&&) noexcept = delete;
  ~deferred_response_context() = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  deferred_response_context& operator=(
      const deferred_response_context&) = delete;
  deferred_response_context& operator=(
      deferred_response_context&&) noexcept = delete;
  // +=========================================================================+
  // | [>] defer                                                    ( public ) |
  // +=========================================================================+
  sender defer() {
    if (deferred_) return {};
    typename Mty::publisher publisher = mailbox_.get_publisher();
    uint64_t position = context_.reserve_response();
    deferred_ = true;
    return sender{std::move(publisher), context_.get_connection_key(),
                  position};
  }
  // +=========================================================================+
  // | [>] deferred                                                 ( public ) |
  // +=========================================================================+
  bool deferred() const { return deferred_; }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  Cty& context_;
  Mty& mailbox_;
  bool deferred_{false};
};
}  // namespace martianlabs::doba::transport::server::detail

#endif
