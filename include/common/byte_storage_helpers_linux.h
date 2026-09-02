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

#ifndef martianlabs_doba_common_byte_storage_helpers_linux_h
#define martianlabs_doba_common_byte_storage_helpers_linux_h

#include <cerrno>
#include <cstddef>
#include <filesystem>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace martianlabs::doba::common {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] byte_storage_file [linux]                                  ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class byte_storage_file {
 public:
  byte_storage_file() = default;
  byte_storage_file(const byte_storage_file&) = delete;
  byte_storage_file(byte_storage_file&& in) noexcept
      : file_(std::exchange(in.file_, -1)) {}
  ~byte_storage_file() { close(); }

  byte_storage_file& operator=(const byte_storage_file&) = delete;
  byte_storage_file& operator=(byte_storage_file&& in) noexcept {
    if (this == &in) return *this;
    close();
    file_ = std::exchange(in.file_, -1);
    return *this;
  }

  bool open(const std::filesystem::path& directory,
            std::filesystem::path& path) {
    std::string pattern = (directory / "doba_bytes_XXXXXX").string();
    std::vector<char> name(pattern.begin(), pattern.end());
    name.push_back('\0');
    file_ = mkstemp(name.data());
    if (file_ == -1) return false;
    if (fcntl(file_, F_SETFD, FD_CLOEXEC) == -1) {
      close();
      return false;
    }
    path = name.data();
    return true;
  }

  bool write(const char* data, std::size_t size) {
    std::size_t written = 0;
    while (written < size) {
      ssize_t result = ::write(file_, data + written, size - written);
      if (result < 0 && errno == EINTR) continue;
      if (result <= 0) return false;
      written += static_cast<std::size_t>(result);
    }
    return true;
  }

  bool flush() { return fsync(file_) == 0; }

  bool read(std::size_t position, char* data, std::size_t size,
            std::size_t& read) {
    read = 0;
    while (read < size) {
      ssize_t result = pread(file_, data + read, size - read,
                             static_cast<off_t>(position + read));
      if (result < 0 && errno == EINTR) continue;
      if (result < 0) return false;
      if (result == 0) return true;
      read += static_cast<std::size_t>(result);
    }
    return true;
  }

  void close() noexcept {
    if (file_ == -1) return;
    ::close(file_);
    file_ = -1;
  }

 private:
  int file_{-1};
};
}  // namespace martianlabs::doba::common

#endif
