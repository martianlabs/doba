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

#ifndef martianlabs_doba_transport_tls_engine_h
#define martianlabs_doba_transport_tls_engine_h

#include <algorithm>
#include <array>
#include <functional>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "protocol/processing_result.h"
#include "protocol/contracts.h"
#include "transport/tls_configuration.h"

namespace martianlabs::doba::transport::detail {
using martianlabs::doba::protocol::processing_result;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tls_engine                                                  ( class ) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   ENty - protocol engine type                                             |
// +---------------------------------------------------------------------------+
// | Adds TLS to a protocol engine without accessing the socket.               |
// | A bounded BIO pair passes encrypted bytes between TLS and TCP.            |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty>
class tls_engine {
 public:
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using policies_type = typename ENty::policies_type;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  // | Template parameters:                                                    |
  // |   FAty - engine factory type                                            |
  // +-------------------------------------------------------------------------+
  template <protocol::contracts::engine_factory<ENty> FAty>
  tls_engine(std::shared_ptr<const tls_configuration> configuration,
             const FAty& factory)
      : configuration_(std::move(configuration)),
        session_(SSL_new(configuration_->context.get()), SSL_free),
        network_(nullptr, BIO_free),
        plaintext_(configuration_->policies.receive_buffer_capacity),
        engine_(factory()) {
    BIO* internal = nullptr;
    BIO* external = nullptr;
    const auto capacity = configuration_->policies.network_bio_buffer_capacity;
    if (!session_ ||
        BIO_new_bio_pair(&internal, capacity, &external, capacity) != 1) {
      throw std::runtime_error("Cannot create TLS session");
    }
    network_.reset(external);
    SSL_set_bio(session_.get(), internal, internal);
    SSL_set_accept_state(session_.get());
    engine_.set_on_send([this](std::string_view prefix, std::string_view body,
                               std::unique_ptr<common::reader> source) {
      append(prefix, body, std::move(source));
    });
    engine_.set_on_close([this] {
      if (!processing_) failed_ = true;
      closing_ = true;
    });
  }
  tls_engine(const tls_engine&) = delete;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  tls_engine& operator=(const tls_engine&) = delete;
  // +=========================================================================+
  // | [>] set_on_send                                              ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_send(protocol::send_delegate output) {
    output_ = std::move(output);
  }
  // +=========================================================================+
  // | [>] set_on_close                                             ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_close(std::function<void()> close) { close_ = std::move(close); }
  // +=========================================================================+
  // | [>] on_bytes_received                                        ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_bytes_received(std::string_view input) {
    if (failed_ || closing_) {
      close_();
      return {};
    }
    begin_batch();
    try {
      while (!input.empty() && !closing_ && !failed_) {
        std::size_t written = 0;
        const int result =
            BIO_write_ex(network_.get(), input.data(), input.size(), &written);
        if (result != 1 || !written) fail();
        input.remove_prefix(written);
        pump();
      }
      return finish_batch();
    } catch (...) {
      return abort_batch();
    }
  }
  // +=========================================================================+
  // | [>] on_processing_turn                                       ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_processing_turn(bool output_idle) {
    if (failed_ || closing_) {
      close_();
      return {};
    }
    // Do not run the protocol engine before the TLS handshake completes.
    if (!SSL_is_init_finished(session_.get())) return {};
    begin_batch();
    try {
      processing_ = true;
      source_seen_ = false;
      progress_ = engine_.on_processing_turn(output_idle);
      processing_ = false;
      return finish_batch();
    } catch (...) {
      return abort_batch();
    }
  }

 private:
  // +=========================================================================+
  // | [>] begin_batch                                             ( private ) |
  // +-------------------------------------------------------------------------+
  void begin_batch() {
    ciphertext_.clear();
    plaintext_output_.clear();
    accepted_ = 0;
  }
  // +=========================================================================+
  // | [>] finish_batch                                            ( private ) |
  // +-------------------------------------------------------------------------+
  // | Sends all encoded bytes to TCP before returning the engine result.      |
  // +-------------------------------------------------------------------------+
  processing_result finish_batch() {
    if (failed_) fail();
    flush_record();
    if (closing_) shutdown();
    if (!ciphertext_.empty()) output_(ciphertext_, {}, nullptr);
    if (closing_) {
      progress_ = {};
      close_();
    }
    return progress_;
  }
  // +=========================================================================+
  // | [>] abort_batch                                             ( private ) |
  // +-------------------------------------------------------------------------+
  processing_result abort_batch() {
    processing_ = false;
    failed_ = true;
    progress_ = {};
    ciphertext_.clear();
    plaintext_output_.clear();
    close_();
    return {};
  }
  // +=========================================================================+
  // | [>] fail                                                    ( private ) |
  // +-------------------------------------------------------------------------+
  [[noreturn]] static void fail() {
    throw std::runtime_error("TLS session failed");
  }
  // +=========================================================================+
  // | [>] drain                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  // | Collects encrypted output up to max_buffered_ciphertext_bytes.          |
  // +-------------------------------------------------------------------------+
  std::size_t drain() {
    const auto before = ciphertext_.size();
    std::array<char, 16384> block;
    while (BIO_ctrl_pending(network_.get())) {
      std::size_t size = 0;
      if (BIO_read_ex(network_.get(), block.data(), block.size(), &size) != 1 ||
          !size ||
          size > configuration_->policies.max_buffered_ciphertext_bytes -
                     ciphertext_.size())
        fail();
      ciphertext_.append(block.data(), size);
    }
    return ciphertext_.size() - before;
  }
  // +=========================================================================+
  // | [>] pump                                                    ( private ) |
  // +-------------------------------------------------------------------------+
  // | Advances the handshake or delivers decrypted input to the engine.       |
  // +-------------------------------------------------------------------------+
  void pump() {
    for (;;) {
      std::size_t received = 0;
      ERR_clear_error();
      const bool handshake = SSL_is_init_finished(session_.get()) == 0;
      const int result = handshake
                             ? SSL_do_handshake(session_.get())
                             : SSL_read_ex(session_.get(), plaintext_.data(),
                                           plaintext_.size(), &received);
      const int error =
          result == 1 ? SSL_ERROR_NONE : SSL_get_error(session_.get(), result);
      const auto produced = drain();
      if (result == 1) {
        if (handshake) continue;
        if (!received) fail();
        processing_ = true;
        source_seen_ = false;
        progress_ = engine_.on_bytes_received({plaintext_.data(), received});
        processing_ = false;
        if (failed_) fail();
        if (closing_) return;
        continue;
      }
      if (error == SSL_ERROR_WANT_READ) return;
      if (error == SSL_ERROR_WANT_WRITE && produced) continue;
      if (error == SSL_ERROR_ZERO_RETURN) {
        closing_ = true;
        return;
      }
      fail();
    }
  }
  // +=========================================================================+
  // | [>] encode                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  void encode(std::string_view input) {
    if (input.size() >
        configuration_->policies.max_buffered_send_bytes - accepted_)
      fail();
    accepted_ += input.size();
    while (!input.empty()) {
      const auto size =
          std::min(input.size(), 16384 - plaintext_output_.size());
      plaintext_output_.append(input.substr(0, size));
      input.remove_prefix(size);
      if (plaintext_output_.size() == 16384) flush_record();
    }
  }
  // +=========================================================================+
  // | [>] flush_record                                            ( private ) |
  // +-------------------------------------------------------------------------+
  // | Encrypts pending payload as a TLS record.                               |
  // +-------------------------------------------------------------------------+
  void flush_record() {
    if (plaintext_output_.empty()) return;
    for (;;) {
      std::size_t written = 0;
      ERR_clear_error();
      const int result = SSL_write_ex(session_.get(), plaintext_output_.data(),
                                      plaintext_output_.size(), &written);
      const int error =
          result == 1 ? SSL_ERROR_NONE : SSL_get_error(session_.get(), result);
      const auto produced = drain();
      if (result == 1 && written == plaintext_output_.size()) {
        plaintext_output_.clear();
        return;
      }
      // A write needing peer input fails; this path never suspends.
      if (error != SSL_ERROR_WANT_WRITE || !produced) fail();
    }
  }
  // +=========================================================================+
  // | [>] append                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  // | Encrypts inline bytes and drains the optional source during this call.  |
  // | A source ends nonempty output for the current engine call.              |
  // +-------------------------------------------------------------------------+
  void append(std::string_view prefix, std::string_view body,
              std::unique_ptr<common::reader> source) noexcept {
    if (failed_) return;
    if (processing_ && !closing_ && prefix.empty() && body.empty() && !source)
      return;
    if (!processing_ || closing_ || source_seen_) {
      failed_ = true;
      return;
    }
    try {
      encode(prefix);
      encode(body);
      if (!source) return;
      source_seen_ = true;
      if (source->failed() || !source->ok()) fail();
      std::array<std::byte, 8192> block;
      while (!source->eof() && !source->failed()) {
        const auto size = source->read(block);
        if (!size && !source->eof()) fail();
        encode({reinterpret_cast<const char*>(block.data()), size});
      }
      if (source->failed() || !source->ok()) fail();
    } catch (...) {
      failed_ = true;
    }
  }
  // +=========================================================================+
  // | [>] shutdown                                                ( private ) |
  // +-------------------------------------------------------------------------+
  // | Sends close_notify, then lets TCP drain without waiting for the peer.   |
  // +-------------------------------------------------------------------------+
  void shutdown() {
    for (;;) {
      ERR_clear_error();
      const int result = SSL_shutdown(session_.get());
      const int error =
          result < 0 ? SSL_get_error(session_.get(), result) : SSL_ERROR_NONE;
      const auto produced = drain();
      if (result >= 0 || error == SSL_ERROR_WANT_READ) return;
      if (error != SSL_ERROR_WANT_WRITE || !produced) fail();
    }
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  const std::shared_ptr<const tls_configuration> configuration_;
  // OpenSSL session; owns the internal BIO half.
  std::unique_ptr<SSL, decltype(&SSL_free)> session_;
  // Feeds encrypted input and receives encrypted output.
  std::unique_ptr<BIO, decltype(&BIO_free)> network_;
  // Decrypted bytes borrowed by the engine during its input call.
  std::vector<char> plaintext_;
  // TCP serializes input and processing calls.
  ENty engine_;
  // TCP callbacks, called only during an input or processing turn.
  protocol::send_delegate output_;
  std::function<void()> close_;
  // Up to 16 KiB of payload waiting to become one TLS record.
  std::string plaintext_output_;
  // Encrypted bytes handed to TCP only after the batch succeeds.
  std::string ciphertext_;
  // Payload bytes accepted so far in this input or processing turn.
  std::size_t accepted_{0};
  // TLS control messages must not erase the last engine result.
  processing_result progress_{};
  // Allows output only during the wrapped engine's input or turn call.
  bool processing_{false};
  // The current engine call has already supplied a source.
  bool source_seen_{false};
  // Drain valid output before closing.
  bool closing_{false};
  // Discard this batch after an error.
  bool failed_{false};
};

}  // namespace martianlabs::doba::transport::detail

#endif
