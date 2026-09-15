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

#ifndef martianlabs_doba_common_filesystem_windows_h
#define martianlabs_doba_common_filesystem_windows_h

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "common/filesystem.h"

namespace martianlabs::doba::common {
// +---------------------------------------------------------------------------+
// | [>] filesystem_file [windows]                                   ( class ) |
// +---------------------------------------------------------------------------+
class filesystem_file {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+

  filesystem_file() = default;
  filesystem_file(const filesystem_file&) = delete;
  filesystem_file(filesystem_file&& in) noexcept
      : file_(std::exchange(in.file_, INVALID_HANDLE_VALUE)),
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
    file_ = std::exchange(in.file_, INVALID_HANDLE_VALUE);
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

    std::filesystem::path relative;
    try {
      relative = std::filesystem::path(
          std::u8string(path.begin(), path.end()));
    } catch (const std::filesystem::filesystem_error& failure) {
      error = failure.code();
      return false;
    }
    for (const auto& part : relative) {
      auto name = part.native();
      if (name.back() == L'.' || name.back() == L' ' ||
          name.find_first_of(L"*?<>\"|") != name.npos) {
        error = std::make_error_code(std::errc::permission_denied);
        return false;
      }
      const auto dot = name.find(L'.');
      if (dot != name.npos) name.resize(dot);
      for (auto& c : name) {
        if (c >= L'a' && c <= L'z') c -= L'a' - L'A';
      }
      if (name == L"CON" || name == L"PRN" ||
          name == L"AUX" || name == L"NUL" ||
          name == L"CONIN$" || name == L"CONOUT$" ||
          (name.size() == 4 &&
           (name.starts_with(L"COM") || name.starts_with(L"LPT")) &&
           ((name[3] >= L'1' && name[3] <= L'9') ||
            name[3] == 0xb9 || name[3] == 0xb2 || name[3] == 0xb3))) {
        error = std::make_error_code(std::errc::permission_denied);
        return false;
      }
    }
    const auto target = directory / relative;
    auto current_path = target.root_path();
    const auto parts = target.relative_path();
    auto part = parts.begin();
    if (target.root_name().native().starts_with(L"\\\\")) {
      if (target.root_name() == L"\\\\?" ||
          target.root_name() == L"\\\\.") {
        error = std::make_error_code(std::errc::permission_denied);
        return false;
      }
      current_path /= *part++;
    }
    // Deny directory renames until the final file has been opened.
    std::vector<filesystem_file> directories;
    for (;;) {
      const bool last = current_path == target;
      filesystem_file next;
      next.file_ = CreateFileW(
          current_path.c_str(), GENERIC_READ,
          FILE_SHARE_READ | FILE_SHARE_WRITE | (last ? FILE_SHARE_DELETE : 0),
          nullptr, OPEN_EXISTING,
          FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
      if (next.file_ == INVALID_HANDLE_VALUE) {
        error = std::error_code(
            static_cast<int>(GetLastError()), std::system_category());
        return false;
      }
      BY_HANDLE_FILE_INFORMATION info{};
      if (!GetFileInformationByHandle(next.file_, &info)) {
        error = std::error_code(
            static_cast<int>(GetLastError()), std::system_category());
        return false;
      }
      if ((info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) ||
          GetFileType(next.file_) != FILE_TYPE_DISK) {
        error = std::make_error_code(std::errc::permission_denied);
        return false;
      }
      const bool is_directory =
          (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
      if (is_directory == last) {
        error = std::make_error_code(
            last ? std::errc::is_a_directory : std::errc::not_a_directory);
        return false;
      }
      if (last) {
        // Attribute writes can turn pinned directories into reparse points.
        const DWORD capacity =
            GetFinalPathNameByHandleW(next.file_, nullptr, 0, 0);
        if (!capacity) {
          error = std::error_code(
              static_cast<int>(GetLastError()), std::system_category());
          return false;
        }
        std::wstring resolved(capacity, L'\0');
        const DWORD path_length = GetFinalPathNameByHandleW(
            next.file_, resolved.data(), capacity, 0);
        if (!path_length || path_length >= capacity) {
          error = path_length
              ? std::make_error_code(std::errc::filename_too_long)
              : std::error_code(
                    static_cast<int>(GetLastError()), std::system_category());
          return false;
        }
        resolved.resize(path_length);
        auto boundary = directory.native();
        boundary = boundary.starts_with(L"\\\\")
            ? L"\\\\?\\UNC\\" + boundary.substr(2)
            : L"\\\\?\\" + boundary;
        if (boundary.back() != L'\\') boundary += L'\\';
        if (!resolved.starts_with(boundary)) {
          error = std::make_error_code(std::errc::permission_denied);
          return false;
        }
        const std::uint64_t length =
            (static_cast<std::uint64_t>(info.nFileSizeHigh) << 32) |
            info.nFileSizeLow;
        if (length > std::numeric_limits<std::size_t>::max()) {
          error = std::make_error_code(std::errc::value_too_large);
          return false;
        }
        next.size_ = static_cast<std::size_t>(length);
        *this = std::move(next);
        return true;
      }
      directories.push_back(std::move(next));
      current_path /= *part++;
    }
  }
  // +=========================================================================+
  // | [>] read                                                     ( public ) |
  // +=========================================================================+

  std::size_t read(std::span<std::byte> output) {
    if (output.empty() || failed_) return 0;
    if (file_ == INVALID_HANDLE_VALUE) {
      failed_ = true;
      return 0;
    }

    const auto count = std::min(
        {output.size(), size_ - position_,
         static_cast<std::size_t>(std::numeric_limits<DWORD>::max())});
    if (!count) return 0;
    DWORD result = 0;
    if (!ReadFile(file_, output.data(), static_cast<DWORD>(count),
                  &result, nullptr) || !result) {
      failed_ = true;
      return 0;
    }
    position_ += result;
    return result;
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
    if (file_ != INVALID_HANDLE_VALUE) CloseHandle(file_);
    file_ = INVALID_HANDLE_VALUE;
    size_ = 0;
    position_ = 0;
    failed_ = false;
  }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  HANDLE file_{INVALID_HANDLE_VALUE};

  std::size_t size_{0};
  std::size_t position_{0};
  bool failed_{false};
};
}  // namespace martianlabs::doba::common

#endif
