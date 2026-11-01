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

#include <array>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "transport/server/tls_engine.h"

namespace {
namespace detail = martianlabs::doba::transport::server::detail;
// The SSL_CTX callback retains its configuration's address for its lifetime.
static_assert(!std::is_copy_constructible_v<detail::tls_configuration>);
static_assert(!std::is_move_constructible_v<detail::tls_configuration>);
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] require                                                  ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
void require(bool condition) {
  if (!condition) throw std::runtime_error("TLS core assertion failed");
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] echo                                                      ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct echo {
  using policies_type = int;
  // +=========================================================================+
  // | [>] set_on_send                                              ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_send(martianlabs::doba::protocol::send_delegate callback) {
    output = std::move(callback);
  }
  // +=========================================================================+
  // | [>] set_on_close                                             ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_close(std::function<void()>) {}
  // +=========================================================================+
  // | [>] on_bytes_received                                        ( public ) |
  // +-------------------------------------------------------------------------+
  void on_bytes_received(std::string_view input) {
    require(input.size() <= 7);
    output({}, input, {});
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // Echoes plaintext exclusively during its input call.
  martianlabs::doba::protocol::send_delegate output;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] exercise                                                 ( function ) |
// +---------------------------------------------------------------------------+
// | Uses the same adapter as both native backends, without a network socket.  |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
void exercise(const char* certificate, const char* key, int version,
              std::size_t capacity) {
  martianlabs::doba::transport::server::tls_policies policies;
  policies.certificate_file = certificate;
  policies.private_key_file = key;
  policies.network_bio_buffer_capacity = capacity;
  policies.receive_buffer_capacity = 7;
  auto settings = std::make_shared<const detail::tls_configuration>(policies);
  auto factory = [] { return echo{}; };
  detail::tls_engine<echo> server(settings, factory);
  std::unique_ptr<SSL_CTX, decltype(&SSL_CTX_free)> context(
      SSL_CTX_new(TLS_client_method()), SSL_CTX_free);
  require(context != nullptr);
  require(SSL_CTX_load_verify_locations(context.get(), certificate, nullptr)
          == 1);
  SSL_CTX_set_verify(context.get(), SSL_VERIFY_PEER, nullptr);
  require(SSL_CTX_set_min_proto_version(context.get(), version) == 1);
  require(SSL_CTX_set_max_proto_version(context.get(), version) == 1);
  std::unique_ptr<SSL, decltype(&SSL_free)> client(SSL_new(context.get()),
                                                  SSL_free);
  require(client != nullptr);
  BIO* input = BIO_new(BIO_s_mem());
  BIO* output = BIO_new(BIO_s_mem());
  require(input && output);
  SSL_set_bio(client.get(), input, output);
  SSL_set_connect_state(client.get());
  require(SSL_set1_host(client.get(), "localhost") == 1);
  bool closed = false;
  server.set_on_send([&](std::string_view bytes, std::string_view,
                         std::unique_ptr<martianlabs::doba::common::reader>) {
    require(BIO_write(input, bytes.data(), static_cast<int>(bytes.size())) ==
            static_cast<int>(bytes.size()));
  });
  server.set_on_close([&] { closed = true; });
  auto transfer = [&] {
    std::array<char, 8192> block;
    while (BIO_ctrl_pending(output)) {
      const int count = BIO_read(
          output, block.data(), static_cast<int>(block.size()));
      require(count > 0);
      server.on_bytes_received({block.data(), static_cast<std::size_t>(count)});
      require(!closed);
    }
  };
  bool established = false;
  for (int turn = 0; turn < 100 && !established; ++turn) {
    ERR_clear_error();
    const int result = SSL_do_handshake(client.get());
    const int error = result == 1 ? SSL_ERROR_NONE :
        SSL_get_error(client.get(), result);
    require(result == 1 || error == SSL_ERROR_WANT_READ);
    transfer();
    established = result == 1;
  }
  require(established && SSL_get_verify_result(client.get()) == X509_V_OK);
  auto exchange = [&] {
    constexpr std::string_view message = "verified plaintext";
    std::size_t size = 0;
    ERR_clear_error();
    require(SSL_write_ex(client.get(), message.data(), message.size(), &size)
            == 1 && size == message.size());
    transfer();
    std::array<char, 128> block;
    ERR_clear_error();
    require(SSL_read_ex(client.get(), block.data(), block.size(), &size) == 1);
    require(std::string_view(block.data(), size) == message);
    transfer();
  };
  exchange();
  if (version == TLS1_3_VERSION) {
    require(SSL_key_update(client.get(), SSL_KEY_UPDATE_REQUESTED) == 1);
    ERR_clear_error();
    require(SSL_do_handshake(client.get()) == 1);
    transfer();
    exchange();
    exchange();
  }
}
}  // namespace

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main(int count, char** arguments) {
  if (count != 3) return 1;
  try {
    for (int version : {TLS1_2_VERSION, TLS1_3_VERSION}) {
      for (std::size_t capacity : {1u, 127u, 17408u}) {
        exercise(arguments[1], arguments[2], version, capacity);
      }
    }
    std::cout << "Bounded BIO progress and TLS 1.3 KeyUpdate: PASS\n";
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
