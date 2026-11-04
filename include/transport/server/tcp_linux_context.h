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

#ifndef martianlabs_doba_transport_server_tcp_linux_context_h
#define martianlabs_doba_transport_server_tcp_linux_context_h

#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <utility>

#include "common/reader.h"
#include "protocol/contracts.h"
#include "protocol/processing_result.h"

namespace martianlabs::doba::transport::server::detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] linux_context                                              ( class ) |
// +---------------------------------------------------------------------------+
// | One worker owns the engine, RX, TX and optional output reader.            |
// | Callbacks run only inside serialized input/processing turns.              |
// | The worker owns the socket and destroys this context outside callbacks.  |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty>
class linux_context {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  template <protocol::contracts::engine_factory<ENty> FAty>
  linux_context(int socket, std::size_t receive_capacity,
                std::size_t send_limit, const FAty& factory)
      : socket_(socket), receive_capacity_(receive_capacity),
        send_limit_(send_limit), receive_(new char[receive_capacity]),
        engine_(factory()) {
    engine_.set_on_send([this](std::string_view prefix, std::string_view body,
                               std::unique_ptr<common::reader> source) {
      append(prefix, body, std::move(source));
    });
    engine_.set_on_close([this]() {
      if (!processing_) failed_ = true;
      closing_ = true;
    });
  }
  linux_context(const linux_context&) = delete;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  linux_context& operator=(const linux_context&) = delete;
  // +=========================================================================+
  // | [>] receive                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  // Fresh bytes are borrowed by the engine only until this call returns.
  bool receive() {
    ssize_t size;
    do {
      size = ::recv(socket_, receive_.get(), receive_capacity_, 0);
    } while (size < 0 && errno == EINTR);
    if (size < 0) return errno == EAGAIN || errno == EWOULDBLOCK;
    if (!size) return false;
    processing_ = true;
    try {
      progress_ = engine_.on_bytes_received(
          {receive_.get(), static_cast<std::size_t>(size)});
    } catch (...) {
      failed_ = true;
    }
    processing_ = false;
    if (closing_) progress_ = {};
    return !failed_;
  }
  // +=========================================================================+
  // | [>] process                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  bool process() {
    if (closing_ || failed_) return !failed_;
    processing_ = true;
    try {
      progress_ = engine_.on_processing_turn(!pending());
    } catch (...) {
      failed_ = true;
    }
    processing_ = false;
    if (closing_) progress_ = {};
    return !failed_;
  }
  // +=========================================================================+
  // | [>] progress                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  const protocol::processing_result& progress() const noexcept {
    return progress_;
  }
  // +=========================================================================+
  // | [>] socket                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  int socket() const noexcept { return socket_; }
  // +=========================================================================+
  // | [>] flush                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // At most one send per turn; a reader fills TX after its prefix drains.
  bool flush() {
    if (failed_) return false;
    if (output_.empty() && source_) {
      if (source_->failed()) return false;
      if (source_->eof()) {
        source_.reset();
      } else {
        output_.resize(std::min<std::size_t>(8192, send_limit_));
        const auto size = source_->read(std::span<std::byte>(
            reinterpret_cast<std::byte*>(output_.data()), output_.size()));
        if (source_->failed() || (!size && !source_->eof())) return false;
        output_.resize(size);
        if (source_->eof()) source_.reset();
      }
    }
    if (!output_.empty()) {
      ssize_t sent;
      do {
        sent = ::send(socket_, output_.data() + offset_,
                      output_.size() - offset_, MSG_NOSIGNAL);
      } while (sent < 0 && errno == EINTR);
      if (sent < 0) return errno == EAGAIN || errno == EWOULDBLOCK;
      if (!sent) return false;
      offset_ += static_cast<std::size_t>(sent);
      if (offset_ == output_.size()) {
        output_.clear();
        offset_ = 0;
      }
    }
    return !closing_ || pending();
  }
  // +=========================================================================+
  // | [>] pending                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  bool pending() const noexcept { return !output_.empty() || source_; }

 private:
  // +=========================================================================+
  // | [>] append                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  // A reader is terminal for nonempty output; empty calls are no-ops.
  void append(std::string_view prefix, std::string_view body,
              std::unique_ptr<common::reader> source) noexcept {
    if (failed_) return;
    if (processing_ && !closing_ &&
        prefix.empty() && body.empty() && !source) return;
    if (!processing_ || closing_ || source_ ||
        prefix.size() > send_limit_ - output_.size() ||
        body.size() > send_limit_ - output_.size() - prefix.size()) {
      failed_ = true;
      return;
    }
    try {
      output_.append(prefix);
      output_.append(body);
      source_ = std::move(source);
    } catch (...) {
      failed_ = true;
    }
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  // Socket borrowed from the owning worker.
  const int socket_;
  // Maximum bytes read from the socket in one turn.
  const std::size_t receive_capacity_;
  // Maximum bytes held in the output buffer.
  const std::size_t send_limit_;
  // Reusable storage for fresh incoming bytes.
  std::unique_ptr<char[]> receive_;
  // Protocol state, accessed exclusively by this connection's worker.
  ENty engine_;
  // Ordered output bytes; capacity is reused after complete sends.
  std::string output_;
  // First unsent byte in output_.
  std::size_t offset_{0};
  // Single reader following all bytes currently in output_.
  std::unique_ptr<common::reader> source_;
  // Latest complete scheduling snapshot, meaningful only to the owner worker.
  protocol::processing_result progress_;
  // True only during a serialized engine input or processing entry.
  bool processing_{false};
  // The engine requested closure after accepted output is sent.
  bool closing_{false};
  // Output contract violation or allocation failure requires closure.
  bool failed_{false};
};
}  // namespace martianlabs::doba::transport::server::detail

#endif
