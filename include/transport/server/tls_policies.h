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
  // Default encrypted RX capacity per connection, in bytes.
  static constexpr std::size_t kDefaultEncryptedReceiveBufferCapacity =
      17 * 1024;
  // Default byte capacity in each direction of the bounded BIO pair.
  static constexpr std::size_t kDefaultNetworkBioBufferCapacity = 17 * 1024;
  // Default encrypted-batch cap, handshake and record overhead included.
  static constexpr std::size_t kDefaultBufferedCiphertextBytes =
      2 * 1024 * 1024;
  // Server certificate chain path, in PEM format.
  std::string certificate_file;
  // Matching unencrypted private-key path, in PEM format.
  std::string private_key_file;
  // Encrypted RX byte capacity per connection; must be positive.
  std::size_t encrypted_receive_buffer_capacity =
      kDefaultEncryptedReceiveBufferCapacity;
  // Inherited RX caps decrypted blocks sent to the engine. Inherited TX counts
  // inline bytes and synchronously read sources. This BIO byte capacity applies
  // per direction; positive and at most INT_MAX.
  std::size_t network_bio_buffer_capacity = kDefaultNetworkBioBufferCapacity;
  // Cap encrypted bytes per TCP input/turn call; must be positive. Also TCP's
  // finite TX budget, separate from the plaintext budget.
  std::size_t max_buffered_ciphertext_bytes = kDefaultBufferedCiphertextBytes;
  // One optional ALPN token, at most 255 bytes. Empty means no negotiation;
  // HTTP/1.1 can use http/1.1.
  std::string alpn_protocol;
};
}  // namespace martianlabs::doba::transport::server

#endif
