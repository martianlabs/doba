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

#ifndef martianlabs_doba_common_filesystem_h
#define martianlabs_doba_common_filesystem_h

#include <filesystem>
#include <cstdint>
#include <optional>
#include <string_view>
#include <system_error>

#include "platform.h"

namespace martianlabs::doba::common {
// Read metadata from the open handle; don't resolve the path again.
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] filesystem_metadata                                        ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct filesystem_metadata {
  std::uintmax_t size = 0;
  std::int64_t modified_seconds = 0;
  std::uint32_t modified_nanoseconds = 0;
  std::uint64_t device = 0;
  std::uint64_t identity = 0;
  std::int64_t changed_seconds = 0;
  std::uint32_t changed_nanoseconds = 0;
  // Windows file flags, or Linux file type and permission bits.
  std::uint32_t attributes = 0;
  // Hard links to this open file.
  std::uint64_t links = 0;
  bool operator==(const filesystem_metadata&) const = default;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] filesystem_root                                          ( function ) |
// +---------------------------------------------------------------------------+
// | Resolve a directory to its canonical path. Return an empty path and set   |
// | error on failure.                                                         |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
inline std::filesystem::path filesystem_root(const std::filesystem::path& path,
                                             std::error_code& error) {
  error.clear();
  auto root = std::filesystem::canonical(path, error);
  if (error) return {};
  if (!std::filesystem::is_directory(root, error)) {
    if (!error) error = std::make_error_code(std::errc::not_a_directory);
    return {};
  }
  return root;
}

namespace detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] filesystem_relative_path                                 ( function ) |
// +---------------------------------------------------------------------------+
// | Check that a path is relative and safe to use. Set error if it isn't.     |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
inline bool filesystem_relative_path(std::string_view path,
                                     std::error_code& error) {
  if (path.empty()) {
    error = std::make_error_code(std::errc::is_a_directory);
    return false;
  }
  for (const unsigned char c : path) {
    if (c < 0x20 || c == 0x7f || c == '\\' || c == ':') {
      error = std::make_error_code(std::errc::permission_denied);
      return false;
    }
  }
  std::size_t position = 0;
  for (;;) {
    const auto end = path.find('/', position);
    const auto part = path.substr(
        position, end == path.npos ? path.size() - position : end - position);
    if (part.empty() || part == "." || part == "..") {
      error = std::make_error_code(std::errc::permission_denied);
      return false;
    }
    if (end == path.npos) break;
    position = end + 1;
  }
  return true;
}
}  // namespace detail
}  // namespace martianlabs::doba::common

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] PLATFORM-DEPENDENT-INCLUDEs                               ( section ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#include "common/filesystem_windows.h"
#elif __linux__
#include "common/filesystem_linux.h"
#endif

#endif
