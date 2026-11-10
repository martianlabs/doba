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

#ifndef martianlabs_doba_transport_tls_configuration_h
#define martianlabs_doba_transport_tls_configuration_h

#include <climits>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include <openssl/err.h>
#include <openssl/ssl.h>

#include "transport/tls_policies.h"

namespace martianlabs::doba::transport::detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tls_configuration                                          ( struct ) |
// +---------------------------------------------------------------------------+
// | Shared TLS settings and server credentials.                               |
// | Sessions keep it alive until their TCP contexts are released.             |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct tls_configuration {
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  explicit tls_configuration(tls_policies settings)
      : policies(std::move(settings)), context(nullptr, SSL_CTX_free) {
    if (!policies.receive_buffer_capacity ||
        policies.receive_buffer_capacity > INT_MAX ||
        !policies.encrypted_receive_buffer_capacity ||
        !policies.network_bio_buffer_capacity ||
        policies.network_bio_buffer_capacity > INT_MAX ||
        !policies.max_buffered_send_bytes ||
        !policies.max_buffered_ciphertext_bytes ||
        policies.alpn_protocol.size() > 255 ||
        policies.certificate_file.empty() ||
        policies.private_key_file.empty() ||
        policies.certificate_file.find('\0') != std::string::npos ||
        policies.private_key_file.find('\0') != std::string::npos) {
      throw std::invalid_argument("Invalid TLS policies");
    }
    ERR_clear_error();
    context.reset(SSL_CTX_new(TLS_server_method()));
    if (context) {
      // Reject encrypted keys instead of prompting for a password.
      SSL_CTX_set_default_passwd_cb(context.get(),
                                    [](char*, int, int, void*) { return 0; });
    }
    if (!context ||
        SSL_CTX_set_min_proto_version(context.get(), TLS1_2_VERSION) != 1 ||
        SSL_CTX_set_max_proto_version(context.get(), TLS1_3_VERSION) != 1 ||
        SSL_CTX_use_certificate_chain_file(
            context.get(), policies.certificate_file.c_str()) != 1 ||
        SSL_CTX_use_PrivateKey_file(context.get(),
                                    policies.private_key_file.c_str(),
                                    SSL_FILETYPE_PEM) != 1 ||
        SSL_CTX_check_private_key(context.get()) != 1) {
      throw std::runtime_error("Cannot configure TLS certificate or key");
    }
    SSL_CTX_set_options(context.get(),
                        SSL_OP_NO_RENEGOTIATION | SSL_OP_NO_TICKET);
    SSL_CTX_set_session_cache_mode(context.get(), SSL_SESS_CACHE_OFF);
    SSL_CTX_set_num_tickets(context.get(), 0);
    SSL_CTX_set_max_early_data(context.get(), 0);
    SSL_CTX_set_verify(context.get(), SSL_VERIFY_NONE, nullptr);
    if (!policies.alpn_protocol.empty()) {
      SSL_CTX_set_alpn_select_cb(context.get(), select_alpn, this);
    }
  }
  // The ALPN callback keeps this address; do not move the configuration.
  tls_configuration(const tls_configuration&) = delete;
  // +=========================================================================+
  // | [>] select_alpn                                              ( public ) |
  // +-------------------------------------------------------------------------+
  // | Selects the configured protocol from the client's ALPN list.            |
  // | No match aborts the handshake; clients without ALPN are still allowed.  |
  // +-------------------------------------------------------------------------+
  static int select_alpn(SSL*, const unsigned char** selected,
                         unsigned char* size, const unsigned char* input,
                         unsigned int length, void* owner) noexcept {
    const auto& token =
        static_cast<tls_configuration*>(owner)->policies.alpn_protocol;
    unsigned int offset = 0;
    while (offset < length) {
      const auto count = input[offset++];
      if (!count || count > length - offset) return SSL_TLSEXT_ERR_ALERT_FATAL;
      if (count == token.size() &&
          token.compare(0, token.size(),
                        reinterpret_cast<const char*>(input + offset),
                        count) == 0) {
        *selected = input + offset;
        *size = count;
        return SSL_TLSEXT_ERR_OK;
      }
      offset += count;
    }
    return SSL_TLSEXT_ERR_ALERT_FATAL;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // Fixed limits, key files and optional ALPN protocol.
  const tls_policies policies;
  // Server context, configured once and shared by sessions.
  std::unique_ptr<SSL_CTX, decltype(&SSL_CTX_free)> context;
};
}  // namespace martianlabs::doba::transport::detail

#endif
