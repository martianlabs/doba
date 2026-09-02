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

#ifndef martianlabs_doba_common_byte_storage_helpers_windows_h
#define martianlabs_doba_common_byte_storage_helpers_windows_h

#include <aclapi.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <random>
#include <string>
#include <utility>
#include <vector>

namespace martianlabs::doba::common {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] byte_storage_file [windowsTM]                            ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class byte_storage_file {
 public:
  byte_storage_file() = default;
  byte_storage_file(const byte_storage_file&) = delete;
  byte_storage_file(byte_storage_file&& in) noexcept
      : file_(std::exchange(in.file_, INVALID_HANDLE_VALUE)) {}
  ~byte_storage_file() { close(); }

  byte_storage_file& operator=(const byte_storage_file&) = delete;
  byte_storage_file& operator=(byte_storage_file&& in) noexcept {
    if (this == &in) return *this;
    close();
    file_ = std::exchange(in.file_, INVALID_HANDLE_VALUE);
    return *this;
  }

  bool open(const std::filesystem::path& directory,
            std::filesystem::path& path) {
    private_security_attributes security;
    if (!security.create()) return false;
    std::random_device random;
    for (std::size_t index = 0; index < 128; ++index) {
      const std::uint64_t value =
          (static_cast<std::uint64_t>(random()) << 32) | random();
      path = directory / ("doba_bytes_" + std::to_string(value) + ".tmp");
      file_ = CreateFileW(
          path.c_str(), GENERIC_READ | GENERIC_WRITE,
          FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
          security.attributes(), CREATE_NEW, FILE_ATTRIBUTE_TEMPORARY, nullptr);
      if (file_ != INVALID_HANDLE_VALUE) return true;
      DWORD error = GetLastError();
      if (error != ERROR_FILE_EXISTS && error != ERROR_ALREADY_EXISTS) break;
    }
    return false;
  }

  bool write(const char* data, std::size_t size) {
    std::size_t written = 0;
    while (written < size) {
      DWORD count = static_cast<DWORD>(std::min(
          size - written, static_cast<std::size_t>(std::numeric_limits<DWORD>::max())));
      DWORD result = 0;
      if (!WriteFile(file_, data + written, count, &result, nullptr)) return false;
      if (result != count) return false;
      written += result;
    }
    return true;
  }

  bool flush() { return FlushFileBuffers(file_) != 0; }

  bool read(std::size_t position, char* data, std::size_t size,
            std::size_t& read) {
    read = 0;
    LARGE_INTEGER offset;
    offset.QuadPart = static_cast<LONGLONG>(position);
    if (!SetFilePointerEx(file_, offset, nullptr, FILE_BEGIN)) return false;
    while (read < size) {
      DWORD count = static_cast<DWORD>(std::min(
          size - read, static_cast<std::size_t>(std::numeric_limits<DWORD>::max())));
      DWORD result = 0;
      if (!ReadFile(file_, data + read, count, &result, nullptr)) return false;
      read += result;
      if (result != count) return true;
    }
    return true;
  }

  void close() noexcept {
    if (file_ == INVALID_HANDLE_VALUE) return;
    CloseHandle(file_);
    file_ = INVALID_HANDLE_VALUE;
  }

 private:
  class private_security_attributes {
   public:
    ~private_security_attributes() {
      if (acl_) LocalFree(acl_);
    }

    bool create() {
      HANDLE token = nullptr;
      if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) return false;
      DWORD size = 0;
      GetTokenInformation(token, TokenUser, nullptr, 0, &size);
      std::vector<std::byte> storage(size);
      bool got_user = GetTokenInformation(token, TokenUser, storage.data(), size,
                                          &size) != 0;
      CloseHandle(token);
      if (!got_user) return false;
      auto* user = reinterpret_cast<TOKEN_USER*>(storage.data());
      EXPLICIT_ACCESSW access{};
      access.grfAccessPermissions = GENERIC_ALL;
      access.grfAccessMode = SET_ACCESS;
      access.grfInheritance = NO_INHERITANCE;
      access.Trustee.TrusteeForm = TRUSTEE_IS_SID;
      access.Trustee.ptstrName = static_cast<LPWSTR>(user->User.Sid);
      if (SetEntriesInAclW(1, &access, nullptr, &acl_) != ERROR_SUCCESS) {
        return false;
      }
      if (!InitializeSecurityDescriptor(&descriptor_, SECURITY_DESCRIPTOR_REVISION) ||
          !SetSecurityDescriptorDacl(&descriptor_, TRUE, acl_, FALSE)) {
        return false;
      }
      attributes_.nLength = sizeof(attributes_);
      attributes_.lpSecurityDescriptor = &descriptor_;
      attributes_.bInheritHandle = FALSE;
      return true;
    }

    SECURITY_ATTRIBUTES* attributes() { return &attributes_; }

   private:
    SECURITY_ATTRIBUTES attributes_{};
    SECURITY_DESCRIPTOR descriptor_{};
    PACL acl_{nullptr};
  };

  HANDLE file_{INVALID_HANDLE_VALUE};
};
}  // namespace martianlabs::doba::common

#endif
