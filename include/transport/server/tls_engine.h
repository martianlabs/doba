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

#ifndef martianlabs_doba_transport_server_tls_engine_h
#define martianlabs_doba_transport_server_tls_engine_h

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
#include "transport/server/tls_configuration.h"

namespace martianlabs::doba::transport::server::detail {
using martianlabs::doba::protocol::processing_result;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tls_engine                                                  ( class ) |
// +---------------------------------------------------------------------------+
// | TCP sees an ordinary engine and its turn results. Only TLS sees SSL.      |
// | All output, including a terminal source, is encoded before TCP sees it.   |
// | One bounded BIO pair moves records without owning or accessing a socket.  |
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
    if (failed_ || closing_) { close_(); return {}; }
    begin_batch();
    try {
      while (!input.empty() && !closing_ && !failed_) {
        std::size_t written = 0;
        const int result = BIO_write_ex(network_.get(), input.data(),
                                         input.size(), &written);
        if (result != 1 || !written) fail();
        input.remove_prefix(written);
        pump();
      }
      return finish_batch();
    } catch (...) { return abort_batch(); }
  }
  // +=========================================================================+
  // | [>] on_processing_turn                                       ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_processing_turn(bool output_idle) {
    if (failed_ || closing_) { close_(); return {}; }
    // Native scheduling cannot precede application input, but do not expose
    // an unfinished handshake to a protocol even in a custom adapter harness.
    if (!SSL_is_init_finished(session_.get())) return {};
    begin_batch();
    try {
      processing_ = true;
      source_seen_ = false;
      progress_ = engine_.on_processing_turn(output_idle);
      processing_ = false;
      return finish_batch();
    } catch (...) { return abort_batch(); }
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
  // TCP copies encoded output before return. No TLS output remains hidden
  // behind its output-idle snapshot, including synchronously drained readers.
  processing_result finish_batch() {
    if (failed_) fail();
    flush_record();
    if (closing_) shutdown();
    if (!ciphertext_.empty()) output_(ciphertext_, {}, nullptr);
    if (closing_) { progress_ = {}; close_(); }
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
  // Retains only a finite ciphertext batch; OpenSSL owns its internal state.
  std::size_t drain() {
    const auto before = ciphertext_.size();
    std::array<char, 16384> block;
    while (BIO_ctrl_pending(network_.get())) {
      std::size_t size = 0;
      if (BIO_read_ex(network_.get(), block.data(), block.size(), &size) != 1 ||
          !size || size > configuration_->policies.
              max_buffered_ciphertext_bytes - ciphertext_.size()) fail();
      ciphertext_.append(block.data(), size);
    }
    return ciphertext_.size() - before;
  }
  // +=========================================================================+
  // | [>] pump                                                    ( private ) |
  // +-------------------------------------------------------------------------+
  void pump() {
    for (;;) {
      std::size_t received = 0;
      ERR_clear_error();
      const bool handshake = SSL_is_init_finished(session_.get()) == 0;
      const int result = handshake ? SSL_do_handshake(session_.get()) :
          SSL_read_ex(session_.get(), plaintext_.data(), plaintext_.size(),
                      &received);
      const int error = result == 1 ? SSL_ERROR_NONE :
          SSL_get_error(session_.get(), result);
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
      if (error == SSL_ERROR_ZERO_RETURN) { closing_ = true; return; }
      fail();
    }
  }
  // +=========================================================================+
  // | [>] encode                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  void encode(std::string_view input) {
    if (input.size() > configuration_->policies.
        max_buffered_send_bytes - accepted_) fail();
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
  // Group inline responses and source blocks into ordinary TLS records.
  void flush_record() {
    if (plaintext_output_.empty()) return;
    for (;;) {
      std::size_t written = 0;
      ERR_clear_error();
      const int result = SSL_write_ex(session_.get(), plaintext_output_.data(),
                                       plaintext_output_.size(), &written);
      const int error = result == 1 ? SSL_ERROR_NONE :
          SSL_get_error(session_.get(), result);
      const auto produced = drain();
      if (result == 1 && written == plaintext_output_.size()) {
        plaintext_output_.clear();
        return;
      }
      // Application writes never suspend waiting for peer input. TLS 1.2
      // renegotiation, early data and post-handshake auth are disabled.
      if (error != SSL_ERROR_WANT_WRITE || !produced) fail();
    }
  }
  // +=========================================================================+
  // | [>] append                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  void append(std::string_view prefix, std::string_view body,
              std::unique_ptr<common::reader> source) noexcept {
    if (failed_) return;
    if (processing_ && !closing_ && prefix.empty() && body.empty() && !source)
      return;
    if (!processing_ || closing_ || source_seen_) { failed_ = true; return; }
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
    } catch (...) { failed_ = true; }
  }
  // +=========================================================================+
  // | [>] shutdown                                                ( private ) |
  // +-------------------------------------------------------------------------+
  // Send our close_notify and let TCP drain. Do not await the peer's reply.
  void shutdown() {
    for (;;) {
      ERR_clear_error();
      const int result = SSL_shutdown(session_.get());
      const int error = result < 0 ? SSL_get_error(session_.get(), result) :
          SSL_ERROR_NONE;
      const auto produced = drain();
      if (result >= 0 || error == SSL_ERROR_WANT_READ) return;
      if (error != SSL_ERROR_WANT_WRITE || !produced) fail();
    }
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  // Immutable credentials remain alive through session destruction.
  const std::shared_ptr<const tls_configuration> configuration_;
  // Per-connection TLS state, including the internal half of the BIO pair.
  std::unique_ptr<SSL, decltype(&SSL_free)> session_;
  // External BIO half; input is fed here and encoded records are drained here.
  std::unique_ptr<BIO, decltype(&BIO_free)> network_;
  // Reusable decrypted receive block borrowed by the application engine.
  std::vector<char> plaintext_;
  // Protocol state; RX and turns are serialized by the TCP backend.
  ENty engine_;
  // TCP's existing synchronous callbacks, never invoked from source readers.
  protocol::send_delegate output_;
  std::function<void()> close_;
  // At most one 16 KiB plaintext record; combines prefix/body/source writes.
  std::string plaintext_output_;
  // Ciphertext accepted by TCP after a successful RX or processing entry.
  std::string ciphertext_;
  // Plaintext produced by all protocol calls in this RX or turn batch.
  std::size_t accepted_{0};
  // Last complete protocol snapshot; TLS control-only RX must preserve it.
  processing_result progress_{};
  // Application output is permitted only inside an engine RX or turn call.
  bool processing_{false};
  // A source is terminal for the current application RX or turn call.
  bool source_seen_{false};
  // Valid output drains before closure; fatal errors discard the current batch.
  bool closing_{false};
  bool failed_{false};
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tls_factory                                                ( struct ) |
// +---------------------------------------------------------------------------+
// | Adapts the application's factory without changing TCP's factory contract. |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty,
          protocol::contracts::engine_factory<ENty> FAty>
struct tls_factory {
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  tls_engine<ENty> operator()() const {
    return tls_engine<ENty>(configuration, factory);
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // Shared immutable TLS configuration retained by each generated engine.
  std::shared_ptr<const tls_configuration> configuration;
  // Creates the application's independent per-connection engine.
  FAty factory;
};
}  // namespace martianlabs::doba::transport::server::detail

#endif
