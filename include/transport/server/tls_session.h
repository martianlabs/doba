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

#ifndef martianlabs_doba_transport_server_tls_session_h
#define martianlabs_doba_transport_server_tls_session_h

#include <algorithm>
#include <cstddef>
#include <memory>
#include <mutex>
#include <span>
#include <stdexcept>

#include <openssl/ssl.h>

#include "transport/server/tls_policies.h"

namespace martianlabs::doba::transport::server {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tls_context                                                 ( alias ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
using tls_context = std::shared_ptr<SSL_CTX>;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] make_tls_context                                           ( method ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
inline tls_context make_tls_context(const tls_policies& configuration) {
  if (!configuration.encrypted_receive_buffer_size ||
      !configuration.network_bio_buffer_size) {
    throw std::runtime_error("TLS buffer capacities must be positive!");
  }
  if (configuration.certificate_file.empty() ||
      configuration.private_key_file.empty()) {
    throw std::runtime_error("TLS certificate and key are required!");
  }
  std::unique_ptr<SSL_CTX, decltype(&SSL_CTX_free)> context(
      SSL_CTX_new(TLS_server_method()), SSL_CTX_free);
  if (!context) throw std::runtime_error("TLS context could not be created!");
  if (SSL_CTX_use_certificate_chain_file(
          context.get(), configuration.certificate_file.c_str()) != 1) {
    throw std::runtime_error("TLS certificate could not be loaded!");
  }
  if (SSL_CTX_use_PrivateKey_file(context.get(),
                                  configuration.private_key_file.c_str(),
                                  SSL_FILETYPE_PEM) != 1) {
    throw std::runtime_error("TLS private key could not be loaded!");
  }
  if (SSL_CTX_check_private_key(context.get()) != 1) {
    throw std::runtime_error("TLS certificate and key do not match!");
  }
  return tls_context(std::move(context));
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tls_session                                                 ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class tls_session {
 public:
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  enum class status { ready, need_input, need_output, closed, failed };
  struct result {
    status state;
    std::size_t size;
  };
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  tls_session(tls_context context, std::size_t send_capacity,
              std::size_t network_bio_capacity = tls_policies::kBioBufferSize)
      : context_{std::move(context)},
        ssl_{context_ ? SSL_new(context_.get()) : nullptr, SSL_free},
        network_{nullptr, BIO_free} {
    if (!ssl_ || !send_capacity || !network_bio_capacity) {
      throw std::runtime_error("TLS session could not be created!");
    }
    BIO* internal = nullptr;
    BIO* network = nullptr;
    const std::size_t bio_capacity =
        (std::min)(send_capacity, network_bio_capacity);
    if (BIO_new_bio_pair(&internal, bio_capacity, &network,
                         network_bio_capacity) != 1) {
      throw std::runtime_error("TLS buffers could not be created!");
    }
    SSL_set_bio(ssl_.get(), internal, internal);
    network_.reset(network);
    SSL_set_accept_state(ssl_.get());
    SSL_set_mode(ssl_.get(), SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
  }
  // +=========================================================================+
  // | [>] handshake                                                ( public ) |
  // +-------------------------------------------------------------------------+
  status handshake() {
    std::lock_guard lock(mutex_);
    if (phase_ == phase::failed) return status::failed;
    if (phase_ == phase::active) return status::ready;
    const int result = SSL_do_handshake(ssl_.get());
    if (result == 1) {
      phase_ = phase::active;
      return status::ready;
    }
    return classify(result);
  }
  // +=========================================================================+
  // | [>] receive                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  std::size_t receive(std::span<const char> bytes) {
    std::lock_guard lock(mutex_);
    if (phase_ == phase::failed || bytes.empty()) return 0;
    std::size_t received = 0;
    if (BIO_write_ex(network_.get(), bytes.data(), bytes.size(), &received) !=
            1 &&
        !BIO_should_retry(network_.get())) {
      phase_ = phase::failed;
    }
    return received;
  }
  // +=========================================================================+
  // | [>] read                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  result read(std::span<char> bytes) {
    std::lock_guard lock(mutex_);
    if (phase_ == phase::failed) return {status::failed, 0};
    if (phase_ != phase::active) {
      return {BIO_ctrl_pending(network_.get()) ? status::need_output
                                               : status::need_input,
              0};
    }
    if (bytes.empty()) return {status::ready, 0};
    std::size_t received = 0;
    const int value =
        SSL_read_ex(ssl_.get(), bytes.data(), bytes.size(), &received);
    if (value == 1) return {status::ready, received};
    return {classify(value), 0};
  }
  // +=========================================================================+
  // | [>] write                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  result write(std::span<const char> bytes) {
    std::lock_guard lock(mutex_);
    if (phase_ == phase::failed || shutdown_sent_) {
      return {status::failed, 0};
    }
    if (phase_ != phase::active) {
      return {BIO_ctrl_pending(network_.get()) ? status::need_output
                                               : status::need_input,
              0};
    }
    if (bytes.empty() && !write_retry_size_) return {status::ready, 0};
    if (write_retry_size_ && bytes.size() < write_retry_size_) {
      phase_ = phase::failed;
      return {status::failed, 0};
    }
    std::size_t size = write_retry_size_;
    if (!size) {
      const std::size_t available =
          BIO_ctrl_get_write_guarantee(SSL_get_wbio(ssl_.get()));
      if (!available) return {status::need_output, 0};
      const std::size_t reserve = (std::min)(std::size_t{2048}, available / 2);
      size =
          (std::min)({bytes.size(), available - reserve, std::size_t{16384}});
    }
    std::size_t written = 0;
    const int value = SSL_write_ex(ssl_.get(), bytes.data(), size, &written);
    if (value == 1) {
      write_retry_size_ = 0;
      return {status::ready, written};
    }
    const status state = classify(value);
    if (state != status::failed) write_retry_size_ = size;
    return {state, 0};
  }
  // +=========================================================================+
  // | [>] shutdown                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  status shutdown() {
    std::lock_guard lock(mutex_);
    if (phase_ != phase::active) return status::failed;
    if (!shutdown_sent_) {
      const int result = SSL_shutdown(ssl_.get());
      if (result < 0) {
        const auto state = classify(result);
        return state == status::need_output ? state : status::failed;
      }
      shutdown_sent_ = true;
    }
    return BIO_ctrl_pending(network_.get()) ? status::need_output
                                            : status::ready;
  }
  // +=========================================================================+
  // | [>] drain                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  std::size_t drain(std::span<char> bytes) {
    std::lock_guard lock(mutex_);
    if (phase_ == phase::failed || bytes.empty()) return 0;
    std::size_t drained = 0;
    if (BIO_read_ex(network_.get(), bytes.data(), bytes.size(), &drained) !=
            1 &&
        !BIO_should_retry(network_.get())) {
      phase_ = phase::failed;
    }
    return drained;
  }
  // +=========================================================================+
  // | [>] output_bytes                                             ( public ) |
  // +=========================================================================+
  std::span<char> output_bytes() {
    std::lock_guard lock(mutex_);
    char* bytes = nullptr;
    const int size = BIO_nread0(network_.get(), &bytes);
    return size > 0 ? std::span(bytes, static_cast<std::size_t>(size))
                    : std::span<char>{};
  }
  // +=========================================================================+
  // | [>] output_sent                                              ( public ) |
  // +-------------------------------------------------------------------------+
  void output_sent(std::size_t size) {
    std::lock_guard lock(mutex_);
    char* bytes = nullptr;
    BIO_nread(network_.get(), &bytes, static_cast<int>(size));
  }
  // +=========================================================================+
  // | [>] pending                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  std::size_t pending() const {
    std::lock_guard lock(mutex_);
    return BIO_ctrl_pending(network_.get());
  }
  // +=========================================================================+
  // | [>] established                                              ( public ) |
  // +-------------------------------------------------------------------------+
  bool established() const {
    std::lock_guard lock(mutex_);
    return phase_ == phase::active;
  }
  // +=========================================================================+
  // | [>] failed                                                   ( public ) |
  // +=========================================================================+
  bool failed() const {
    std::lock_guard lock(mutex_);
    return phase_ == phase::failed;
  }

 private:
  // +=========================================================================+
  // | [>] TYPEs                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  enum class phase { handshaking, active, failed };
  // +=========================================================================+
  // | [>] classify                                                ( private ) |
  // +-------------------------------------------------------------------------+
  status classify(int result) {
    const int error = SSL_get_error(ssl_.get(), result);
    if (error == SSL_ERROR_ZERO_RETURN) return status::closed;
    if (error == SSL_ERROR_WANT_WRITE) {
      if (BIO_ctrl_pending(network_.get())) return status::need_output;
      phase_ = phase::failed;
      return status::failed;
    }
    if (error == SSL_ERROR_WANT_READ) {
      return BIO_ctrl_pending(network_.get()) ? status::need_output
                                              : status::need_input;
    }
    phase_ = phase::failed;
    return status::failed;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  tls_context context_;
  std::unique_ptr<SSL, decltype(&SSL_free)> ssl_;
  std::unique_ptr<BIO, decltype(&BIO_free)> network_;
  mutable std::mutex mutex_;
  phase phase_{phase::handshaking};
  // The caller must retry the same bytes after an OpenSSL retry.
  std::size_t write_retry_size_{0};
  bool shutdown_sent_{false};
};
}  // namespace martianlabs::doba::transport::server

#endif
