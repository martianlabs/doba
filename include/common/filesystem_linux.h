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

#ifndef martianlabs_doba_common_filesystem_linux_h
#define martianlabs_doba_common_filesystem_linux_h

#include <algorithm>
#include <cstddef>
#include <limits>
#include <span>
#include <string>
#include <utility>
#include <cerrno>
#include <sys/stat.h>

#include "common/filesystem.h"

namespace martianlabs::doba::common {
// +---------------------------------------------------------------------------+
// | [>] filesystem_file [linux]                                     ( class ) |
// +---------------------------------------------------------------------------+
class filesystem_file {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+

  filesystem_file() = default;
  filesystem_file(const filesystem_file&) = delete;
  filesystem_file(filesystem_file&& in) noexcept
      : file_(std::exchange(in.file_, -1)),
        size_(std::exchange(in.size_, 0)),
        position_(std::exchange(in.position_, 0)),
        failed_(std::exchange(in.failed_, false)) {}
  ~filesystem_file() { close(); }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+

  filesystem_file& operator=(const filesystem_file&) = delete;
  filesystem_file& operator=(filesystem_file&& in) noexcept {
    if (this == &in) return *this;
    close();
    file_ = std::exchange(in.file_, -1);
    size_ = std::exchange(in.size_, 0);
    position_ = std::exchange(in.position_, 0);
    failed_ = std::exchange(in.failed_, false);
    return *this;
  }
  // +=========================================================================+
  // | [>] open                                                     ( public ) |
  // +=========================================================================+

  bool open(const std::filesystem::path& root, std::string_view path,
            std::error_code& error) {
    close();
    error.clear();
    if (!detail::filesystem_relative_path(path, error)) return false;
    if (!root.is_absolute()) {
      error = std::make_error_code(std::errc::invalid_argument);
      return false;
    }
    const auto directory = root.lexically_normal();

    const auto target = directory / std::filesystem::path(path);
    filesystem_file current;
    current.file_ = ::open("/", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (current.file_ == -1) {
      error = std::error_code(errno, std::generic_category());
      return false;
    }
    const auto relative = target.relative_path();
    for (auto part = relative.begin(); part != relative.end(); ++part) {
      const bool last = std::next(part) == relative.end();
      filesystem_file next;
      // Pin each directory and never follow a request-path symlink.
      next.file_ = ::openat(
          current.file_, part->c_str(),
          O_RDONLY | O_CLOEXEC | O_NOFOLLOW | O_NONBLOCK |
              (last ? 0 : O_DIRECTORY));
      if (next.file_ == -1) {
        const int code = errno;
        struct stat link {};
        if (code == ELOOP ||
            (::fstatat(current.file_, part->c_str(), &link,
                       AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(link.st_mode))) {
          error = std::make_error_code(std::errc::permission_denied);
        } else {
          error = std::error_code(code, std::generic_category());
        }
        return false;
      }
      if (last) {
        struct stat info {};
        if (::fstat(next.file_, &info) != 0) {
          error = std::error_code(errno, std::generic_category());
          return false;
        }
        if (!S_ISREG(info.st_mode)) {
          error = std::make_error_code(
              S_ISDIR(info.st_mode) ? std::errc::is_a_directory
                                   : std::errc::permission_denied);
          return false;
        }
        if (info.st_size < 0 ||
            static_cast<std::uintmax_t>(info.st_size) >
                std::numeric_limits<std::size_t>::max()) {
          error = std::make_error_code(std::errc::value_too_large);
          return false;
        }
        next.size_ = static_cast<std::size_t>(info.st_size);
      }
      current = std::move(next);
    }
    *this = std::move(current);
    return true;
  }
  // +=========================================================================+
  // | [>] read                                                     ( public ) |
  // +=========================================================================+

  std::size_t read(std::span<std::byte> output) {
    if (output.empty() || failed_) return 0;
    if (file_ == -1) {
      failed_ = true;
      return 0;
    }

    const auto count = std::min(
        {output.size(), size_ - position_,
         static_cast<std::size_t>(std::numeric_limits<ssize_t>::max())});
    if (!count) return 0;
    ssize_t result;
    do {
      result = ::read(file_, output.data(), count);
    } while (result < 0 && errno == EINTR);
    if (result <= 0) {
      failed_ = true;
      return 0;
    }
    position_ += static_cast<std::size_t>(result);
    return static_cast<std::size_t>(result);
  }
  // +=========================================================================+
  // | [>] size/eof/failed                                          ( public ) |
  // +=========================================================================+

  [[nodiscard]] std::size_t size() const noexcept { return size_; }
  [[nodiscard]] bool eof() const noexcept { return position_ == size_; }
  [[nodiscard]] bool failed() const noexcept { return failed_; }
  // +=========================================================================+
  // | [>] close                                                    ( public ) |
  // +=========================================================================+

  void close() noexcept {
    if (file_ != -1) ::close(file_);
    file_ = -1;
    size_ = 0;
    position_ = 0;
    failed_ = false;
  }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  int file_{-1};

  std::size_t size_{0};
  std::size_t position_{0};
  bool failed_{false};
};
}  // namespace martianlabs::doba::common

#endif
