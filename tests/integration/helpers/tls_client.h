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

#ifndef martianlabs_doba_tests_integration_tls_client_h
#define martianlabs_doba_tests_integration_tls_client_h

#include <algorithm>
#include <array>
#include <chrono>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#include <openssl/ssl.h>

#include "tcpip_client.h"

namespace martianlabs::doba::tests::integration {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tls_client                                                 ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct tls_client {
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  tcpip_client socket;
  std::unique_ptr<SSL_CTX, decltype(&SSL_CTX_free)> context{
      SSL_CTX_new(TLS_client_method()), SSL_CTX_free};
  std::unique_ptr<SSL, decltype(&SSL_free)> ssl{
      context ? SSL_new(context.get()) : nullptr, SSL_free};
  std::unique_ptr<BIO, decltype(&BIO_free)> network{nullptr, BIO_free};
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  tls_client() {
    BIO* internal = nullptr;
    BIO* external = nullptr;
    if (!ssl || BIO_new_bio_pair(&internal, 0, &external, 0) != 1) {
      throw std::runtime_error("TLS client could not be created!");
    }
    SSL_set_bio(ssl.get(), internal, internal);
    network.reset(external);
    SSL_set_connect_state(ssl.get());
  }
  // +=========================================================================+
  // | [>] exchange                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  bool exchange(std::size_t fragment = 4096) {
    std::array<char, 4096> bytes;
    while (BIO_ctrl_pending(network.get())) {
      std::size_t size = 0;
      if (BIO_read_ex(network.get(), bytes.data(),
                      (std::min)(fragment, bytes.size()), &size) != 1 ||
          !socket.send_all(std::string_view(bytes.data(), size)))
        return false;
    }
    if (!socket.has_data(std::chrono::milliseconds(10))) return true;
    auto received = socket.receive_some(bytes.size());
    if (!received) return false;
    std::size_t written = 0;
    return BIO_write_ex(network.get(), received->data(), received->size(),
                        &written) == 1 &&
           written == received->size();
  }
  // +=========================================================================+
  // | [>] negotiate                                                ( public ) |
  // +-------------------------------------------------------------------------+
  bool negotiate(std::size_t fragment = 4096) {
    for (int step = 0; step < 100; ++step) {
      const int result = SSL_do_handshake(ssl.get());
      if (result != 1) {
        const int error = SSL_get_error(ssl.get(), result);
        if (error != SSL_ERROR_WANT_READ && error != SSL_ERROR_WANT_WRITE)
          return false;
      }
      if (!exchange(fragment)) return false;
      if (SSL_is_init_finished(ssl.get())) return true;
    }
    return false;
  }
  // +=========================================================================+
  // | [>] send                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  bool send(std::string_view value, std::size_t fragment = 4096) {
    for (int step = 0; step < 100; ++step) {
      std::size_t written = 0;
      const int result =
          SSL_write_ex(ssl.get(), value.data(), value.size(), &written);
      const int error = result == 1 ? 0 : SSL_get_error(ssl.get(), result);
      if (!exchange(fragment)) return false;
      if (result == 1) return written == value.size();
      if (error != SSL_ERROR_WANT_READ && error != SSL_ERROR_WANT_WRITE)
        return false;
    }
    return false;
  }
  // +=========================================================================+
  // | [>] receive                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  std::optional<std::string> receive(std::size_t size) {
    std::string result(size, '\0');
    std::size_t total = 0;
    for (int step = 0; step < 400 && total < size; ++step) {
      std::size_t received = 0;
      const int value = SSL_read_ex(ssl.get(), result.data() + total,
                                    size - total, &received);
      total += received;
      if (value == 1) {
        if (total == size) return result;
        continue;
      }
      const int error = SSL_get_error(ssl.get(), value);
      if (error != SSL_ERROR_WANT_READ && error != SSL_ERROR_WANT_WRITE)
        return std::nullopt;
      if (!exchange()) return std::nullopt;
    }
    return std::nullopt;
  }
  // +=========================================================================+
  // | [>] receive_close                                            ( public ) |
  // +-------------------------------------------------------------------------+
  bool receive_close() {
    std::array<char, 32> bytes;
    for (int step = 0; step < 100; ++step) {
      std::size_t received = 0;
      const int result =
          SSL_read_ex(ssl.get(), bytes.data(), bytes.size(), &received);
      if (result == 1) return false;
      const int error = SSL_get_error(ssl.get(), result);
      if (error == SSL_ERROR_ZERO_RETURN) return true;
      if (error != SSL_ERROR_WANT_READ && error != SSL_ERROR_WANT_WRITE)
        return false;
      if (!exchange()) return false;
    }
    return false;
  }
  // +=========================================================================+
  // | [>] send                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  bool send_outgoing(std::string_view value) {
    for (int step = 0; step < 100; ++step) {
      std::size_t written = 0;
      const int result =
          SSL_write_ex(ssl.get(), value.data(), value.size(), &written);
      const int error = result == 1 ? 0 : SSL_get_error(ssl.get(), result);
      std::array<char, 4096> bytes;
      while (BIO_ctrl_pending(network.get())) {
        std::size_t size = 0;
        if (BIO_read_ex(network.get(), bytes.data(), bytes.size(), &size) !=
                1 ||
            !socket.send_all(std::string_view(bytes.data(), size))) {
          return false;
        }
      }
      if (result == 1) return written == value.size();
      if (error != SSL_ERROR_WANT_READ && error != SSL_ERROR_WANT_WRITE)
        return false;
    }
    return false;
  }
};
}  // namespace martianlabs::doba::tests::integration

#endif
