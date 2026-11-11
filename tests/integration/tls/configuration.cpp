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

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>

#include <openssl/pem.h>

#include "transport/tls_configuration.h"

namespace {
namespace tr = martianlabs::doba::transport;
namespace detail = tr::detail;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] require                                                  ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
void require(bool condition) {
  if (!condition) throw std::runtime_error("TLS configuration check failed");
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] temporary_files                                            ( struct ) |
// +---------------------------------------------------------------------------+
// | Remove only files created here; never remove a directory tree.            |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct temporary_files {
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  temporary_files() {
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch();
    directory = std::filesystem::temp_directory_path() /
        ("doba-tls-configuration-" + std::to_string(stamp.count()));
    require(std::filesystem::create_directory(directory));
  }
  ~temporary_files() {
    std::error_code error;
    for (const auto* name : {"mismatch.pem", "encrypted.pem", "invalid.pem"}) {
      std::filesystem::remove(directory / name, error);
    }
    std::filesystem::remove(directory, error);
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // A private directory under the system temp directory.
  std::filesystem::path directory;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] invalid                                                  ( function ) |
// +---------------------------------------------------------------------------+
// | Check the expected policy error before any listener opens.                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename FNty>
void invalid(const tr::tls_policies& good, FNty change) {
  auto policies = good;
  change(policies);
  bool rejected = false;
  try { detail::tls_configuration configuration(std::move(policies)); }
  catch (const std::invalid_argument&) { rejected = true; }
  require(rejected);
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] bad_credentials                                          ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
void bad_credentials(tr::tls_policies policies) {
  bool rejected = false;
  try { detail::tls_configuration configuration(std::move(policies)); }
  catch (const std::runtime_error&) { rejected = true; }
  require(rejected);
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] write_key                                                ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
void write_key(const std::filesystem::path& path, EVP_PKEY* key,
               bool encrypted) {
  std::unique_ptr<BIO, decltype(&BIO_free)> output(
      BIO_new_file(path.string().c_str(), "w"), BIO_free);
  require(output != nullptr);
  const std::string password = "doba-test-only";
  const auto* bytes = reinterpret_cast<const unsigned char*>(password.data());
  require(PEM_write_bio_PrivateKey(output.get(), key,
      encrypted ? EVP_aes_256_cbc() : nullptr,
      encrypted ? bytes : nullptr,
      encrypted ? static_cast<int>(password.size()) : 0,
      nullptr, nullptr) == 1);
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] exercise                                                 ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
void exercise(const char* certificate, const char* key_file) {
  tr::tls_policies good;
  good.certificate_file = certificate;
  good.private_key_file = key_file;
  invalid(good, [](auto& p) { p.receive_buffer_capacity = 0; });
  invalid(good, [](auto& p) {
    p.receive_buffer_capacity = static_cast<std::size_t>(INT_MAX) + 1;
  });
  invalid(good, [](auto& p) { p.encrypted_receive_buffer_capacity = 0; });
  invalid(good, [](auto& p) { p.network_bio_buffer_capacity = 0; });
  invalid(good, [](auto& p) {
    p.network_bio_buffer_capacity = static_cast<std::size_t>(INT_MAX) + 1;
  });
  invalid(good, [](auto& p) { p.max_buffered_send_bytes = 0; });
  invalid(good, [](auto& p) { p.max_buffered_ciphertext_bytes = 0; });
  invalid(good, [](auto& p) { p.certificate_file.clear(); });
  invalid(good, [](auto& p) { p.private_key_file.clear(); });
  invalid(good, [](auto& p) { p.certificate_file.push_back('\0'); });
  invalid(good, [](auto& p) { p.private_key_file.push_back('\0'); });
  invalid(good, [](auto& p) { p.alpn_protocol.assign(256, 'a'); });
  auto policies = good;
  policies.alpn_protocol.assign(255, 'a');
  detail::tls_configuration maximum_alpn(policies);
  policies.alpn_protocol.clear();
  detail::tls_configuration no_alpn(policies);
  require(SSL_CTX_get_min_proto_version(no_alpn.context.get()) ==
          TLS1_2_VERSION);
  require(SSL_CTX_get_max_proto_version(no_alpn.context.get()) ==
          TLS1_3_VERSION);
  const auto password = SSL_CTX_get_default_passwd_cb(no_alpn.context.get());
  require(password != nullptr && password != PEM_def_callback);
  char buffer[32];
  require(password(buffer, 32, 0, nullptr) == 0);
  require(SSL_CTX_get_verify_mode(no_alpn.context.get()) == SSL_VERIFY_NONE);
  require((SSL_CTX_get_options(no_alpn.context.get()) &
           SSL_OP_NO_RENEGOTIATION) != 0);
  require(SSL_CTX_get_max_early_data(no_alpn.context.get()) == 0);
  require(SSL_CTX_get_num_tickets(no_alpn.context.get()) == 0);
  require(SSL_CTX_get_session_cache_mode(no_alpn.context.get()) ==
          SSL_SESS_CACHE_OFF);

  temporary_files files;
  std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> other(
      EVP_PKEY_Q_keygen(nullptr, nullptr, "EC", "prime256v1"), EVP_PKEY_free);
  require(other != nullptr);
  write_key(files.directory / "mismatch.pem", other.get(), false);
  policies.private_key_file = (files.directory / "mismatch.pem").string();
  bad_credentials(policies);
  std::unique_ptr<BIO, decltype(&BIO_free)> input(
      BIO_new_file(key_file, "r"), BIO_free);
  require(input != nullptr);
  std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> original(
      PEM_read_bio_PrivateKey(input.get(), nullptr, nullptr, nullptr),
      EVP_PKEY_free);
  require(original != nullptr);
  write_key(files.directory / "encrypted.pem", original.get(), true);
  policies.private_key_file = (files.directory / "encrypted.pem").string();
  bad_credentials(policies);
  { std::ofstream invalid_file(files.directory / "invalid.pem");
    invalid_file << "not a PEM certificate or key\n"; }
  policies = good;
  policies.certificate_file = (files.directory / "invalid.pem").string();
  bad_credentials(policies);
  policies = good;
  policies.private_key_file = (files.directory / "invalid.pem").string();
  bad_credentials(policies);
  policies = good;
  policies.private_key_file += ".missing";
  bad_credentials(policies);
  policies = good;
  policies.certificate_file += ".missing";
  bad_credentials(policies);
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
    exercise(arguments[1], arguments[2]);
    std::cout << "TLS policies, credentials and excluded options: PASS\n";
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
