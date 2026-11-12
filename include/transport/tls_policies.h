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

#ifndef martianlabs_doba_transport_tls_policies_h
#define martianlabs_doba_transport_tls_policies_h

#include <cstddef>
#include <string>

#include "transport/tcp_policies.h"

namespace martianlabs::doba::transport {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tls_policies                                               ( struct ) |
// +---------------------------------------------------------------------------+
// | TLS credentials and buffer limits, in addition to TCP settings.           |
// | TCP's receive_buffer_capacity limits decrypted input blocks.              |
// | TCP's max_buffered_send_bytes includes inline output and source bytes.    |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct tls_policies : tcp_policies {
  // +=========================================================================+
  // | [>] CONSTANTs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  static constexpr std::size_t kDefaultEncryptedReceiveBufferCapacity =
      17 * 1024;
  static constexpr std::size_t kDefaultNetworkBioBufferCapacity = 17 * 1024;
  static constexpr std::size_t kDefaultBufferedCiphertextBytes =
      2 * 1024 * 1024;
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // PEM file containing the server certificate chain.
  std::string certificate_file;
  // Matching private key in PEM format; encrypted keys are rejected.
  std::string private_key_file;
  // Encrypted socket bytes per read; must be positive.
  std::size_t encrypted_receive_buffer_capacity =
      kDefaultEncryptedReceiveBufferCapacity;
  // Bytes per direction of the TLS BIO pair; positive, at most INT_MAX.
  std::size_t network_bio_buffer_capacity = kDefaultNetworkBioBufferCapacity;
  // Encrypted bytes per input or processing turn, including TLS overhead.
  // Also sets TCP's send buffer limit; must be positive.
  std::size_t max_buffered_ciphertext_bytes = kDefaultBufferedCiphertextBytes;
  // One protocol token, up to 255 bytes. Empty disables ALPN.
  std::string alpn_protocol;
};
}  // namespace martianlabs::doba::transport

#endif
