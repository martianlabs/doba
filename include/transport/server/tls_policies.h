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

#ifndef martianlabs_doba_transport_server_tls_policies_h
#define martianlabs_doba_transport_server_tls_policies_h

#include <cstddef>
#include <string>

#include "transport/server/tcp_policies.h"

namespace martianlabs::doba::transport::server {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tls_policies                                               ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct tls_policies : tcp_policies {
  // Encrypted socket input capacity per connection, in bytes.
  static constexpr std::size_t kDefaultEncryptedReceiveBufferCapacity =
      17 * 1024;
  // Capacity of each direction of the bounded TLS BIO pair, in bytes.
  static constexpr std::size_t kDefaultNetworkBioBufferCapacity = 17 * 1024;
  // Default encoded batch limit, including handshake and record overhead.
  static constexpr std::size_t kDefaultBufferedCiphertextBytes =
      2 * 1024 * 1024;
  // Path to the server certificate chain in PEM format.
  std::string certificate_file;
  // Path to the matching unencrypted private key in PEM format.
  std::string private_key_file;
  // Encrypted receive capacity per connection (must be positive).
  std::size_t encrypted_receive_buffer_capacity =
      kDefaultEncryptedReceiveBufferCapacity;
  // Inherited RX capacity limits decrypted blocks delivered to the engine.
  // Inherited TX budget includes inline bytes AND a synchronously read source.
  // Network BIO capacity per direction (positive, at most INT_MAX).
  std::size_t network_bio_buffer_capacity = kDefaultNetworkBioBufferCapacity;
  // Maximum encoded bytes accumulated per TCP input call (must be positive).
  // This is also TCP's finite TX budget; it is separate from the plain budget.
  std::size_t max_buffered_ciphertext_bytes = kDefaultBufferedCiphertextBytes;
  // Optional single ALPN token, at most 255 bytes; empty disables negotiation.
  // HTTP/1.1 applications may set this to "http/1.1". No token implies no ALPN.
  std::string alpn_protocol;
};
}  // namespace martianlabs::doba::transport::server

#endif
