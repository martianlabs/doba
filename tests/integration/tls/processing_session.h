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

#ifndef martianlabs_doba_tests_tls_processing_session_h
#define martianlabs_doba_tests_tls_processing_session_h

#include <algorithm>
#include <array>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "transport/server/tls_engine.h"

namespace martianlabs::doba::tests::tls {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] processing_session                                          ( class ) |
// +---------------------------------------------------------------------------+
// | Verified memory-BIO client and real TLS adapter; no native scheduling.    |
// | Tests supply turns and can hold ciphertext to model occupied TCP output. |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty>
class processing_session {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  template <typename Factory>
  processing_session(transport::server::tls_policies policies,
                     Factory factory, int version)
      : context_(client_context(policies.certificate_file, version)),
        client_(SSL_new(context_.get()), SSL_free),
        server_(std::make_shared<const configuration>(std::move(policies)),
                factory) {
    check(client_ != nullptr);
    BIO* input = BIO_new(BIO_s_mem());
    BIO* output = BIO_new(BIO_s_mem());
    if (!input || !output) {
      BIO_free(input);
      BIO_free(output);
      check(false);
    }
    SSL_set_bio(client_.get(), input, output);
    SSL_set_connect_state(client_.get());
    check(SSL_set1_host(client_.get(), "localhost") == 1);
    server_.set_on_send([this](auto prefix, auto body, auto source) {
      check(body.empty() && !source);
      ciphertext_.append(prefix);
    });
    server_.set_on_close([this] { closed_ = true; });
    for (unsigned round = 0; round < 1000; ++round) {
      ERR_clear_error();
      const int result = SSL_do_handshake(client_.get());
      const int error = result == 1 ? SSL_ERROR_NONE :
          SSL_get_error(client_.get(), result);
      check(result == 1 || error == SSL_ERROR_WANT_READ);
      transfer();
      deliver();
      check(!closed_);
      if (result == 1) {
        check(SSL_get_verify_result(client_.get()) == X509_V_OK);
        return;
      }
    }
    check(false);
  }
  // +=========================================================================+
  // | [>] write                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // Decrypted input is borrowed; overwrite every delivered encrypted fragment.
  void write(std::string_view plaintext, std::size_t fragment = 8192) {
    std::size_t size = 0;
    ERR_clear_error();
    check(SSL_write_ex(client_.get(), plaintext.data(), plaintext.size(), &size)
          == 1 && size == plaintext.size());
    transfer(fragment);
  }
  // +=========================================================================+
  // | [>] turn                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  void turn(bool idle) { progress_ = server_.on_processing_turn(idle); }
  // +=========================================================================+
  // | [>] deliver                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  void deliver() {
    if (ciphertext_.empty()) return;
    std::size_t size = 0;
    check(BIO_write_ex(SSL_get_rbio(client_.get()), ciphertext_.data(),
                        ciphertext_.size(), &size) == 1);
    check(size == ciphertext_.size());
    ciphertext_.clear();
  }
  // +=========================================================================+
  // | [>] read                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  std::string read() {
    deliver();
    std::string result;
    std::array<char, 8192> block;
    for (;;) {
      std::size_t size = 0;
      ERR_clear_error();
      const int status = SSL_read_ex(client_.get(), block.data(),
                                      block.size(), &size);
      const int error = status == 1 ? SSL_ERROR_NONE :
          SSL_get_error(client_.get(), status);
      if (status == 1) { result.append(block.data(), size); continue; }
      check(error == SSL_ERROR_WANT_READ || error == SSL_ERROR_ZERO_RETURN);
      return result;
    }
  }
  // +=========================================================================+
  // | [>] update_key                                               ( public ) |
  // +-------------------------------------------------------------------------+
  void update_key() {
    check(SSL_key_update(client_.get(), SSL_KEY_UPDATE_REQUESTED) == 1);
    ERR_clear_error();
    check(SSL_do_handshake(client_.get()) == 1);
    transfer(1);
  }
  // +=========================================================================+
  // | [>] shutdown                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  void shutdown() {
    ERR_clear_error();
    check(SSL_shutdown(client_.get()) >= 0);
    transfer(1);
  }
  // +=========================================================================+
  // | [>] OBSERVERs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  const protocol::processing_result& progress() const { return progress_; }
  bool closed() const { return closed_; }
  std::size_t pending() const { return ciphertext_.size(); }

 private:
  // +=========================================================================+
  // | [>] USINGs                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  using configuration = transport::server::detail::tls_configuration;
  using context_owner = std::unique_ptr<SSL_CTX, decltype(&SSL_CTX_free)>;
  // +=========================================================================+
  // | [>] check                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  static void check(bool condition) {
    if (!condition) throw std::runtime_error("TLS processing peer failed");
  }
  // +=========================================================================+
  // | [>] client_context                                          ( private ) |
  // +-------------------------------------------------------------------------+
  static context_owner client_context(const std::string& cert, int version) {
    context_owner result(SSL_CTX_new(TLS_client_method()), SSL_CTX_free);
    check(result != nullptr);
    check(SSL_CTX_load_verify_locations(result.get(), cert.c_str(), nullptr)
          == 1);
    SSL_CTX_set_verify(result.get(), SSL_VERIFY_PEER, nullptr);
    check(SSL_CTX_set_min_proto_version(result.get(), version) == 1);
    check(SSL_CTX_set_max_proto_version(result.get(), version) == 1);
    return result;
  }
  // +=========================================================================+
  // | [>] transfer                                                ( private ) |
  // +-------------------------------------------------------------------------+
  void transfer(std::size_t fragment = 8192) {
    check(fragment > 0);
    std::array<char, 8192> bytes;
    BIO* outgoing = SSL_get_wbio(client_.get());
    while (BIO_ctrl_pending(outgoing)) {
      std::size_t size = 0;
      check(BIO_read_ex(outgoing, bytes.data(),
                         std::min(fragment, bytes.size()), &size) == 1);
      check(size > 0);
      progress_ = server_.on_bytes_received({bytes.data(), size});
      std::fill(bytes.begin(), bytes.end(), 'x');
    }
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  // Verified client context and session own both memory BIOs.
  context_owner context_;
  std::unique_ptr<SSL, decltype(&SSL_free)> client_;
  // Same TLS adapter used by both real native transports.
  transport::server::detail::tls_engine<ENty> server_;
  // Encoded output waiting for simulated TCP completion/client delivery.
  std::string ciphertext_;
  // Complete last return, observed even for partial/control-only input.
  protocol::processing_result progress_{};
  // Captures the adapter's existing synchronous close callback.
  bool closed_{false};
};
}  // namespace martianlabs::doba::tests::tls

#endif
