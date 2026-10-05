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

#ifndef martianlabs_doba_tests_unit_directory_link_windows_h
#define martianlabs_doba_tests_unit_directory_link_windows_h

#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <system_error>

#include "platform.h"

#include <winioctl.h>

namespace martianlabs::doba::tests::unit {
inline void create_directory_link(const std::filesystem::path& path,
                                  const std::filesystem::path& target) {
  std::filesystem::create_directory(path);
  struct junction_data {
    DWORD tag;
    WORD length;
    WORD reserved;
    WORD substitute_offset;
    WORD substitute_length;
    WORD print_offset;
    WORD print_length;
    wchar_t path[4096];
  } data{};
  const std::wstring name = L"\\??\\" + target.native();
  if (name.size() + 2 > std::size(data.path)) {
    throw std::runtime_error("Junction path too long");
  }
  data.tag = IO_REPARSE_TAG_MOUNT_POINT;
  data.substitute_length = static_cast<WORD>(name.size() * sizeof(wchar_t));
  data.print_offset = data.substitute_length + sizeof(wchar_t);
  data.length = 8 + data.substitute_length + 2 * sizeof(wchar_t);
  std::copy(name.begin(), name.end(), data.path);
  HANDLE handle = CreateFileW(
      path.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING,
      FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
  if (handle == INVALID_HANDLE_VALUE) {
    throw std::runtime_error("Unable to open junction");
  }
  DWORD returned = 0;
  const bool created =
      DeviceIoControl(handle, FSCTL_SET_REPARSE_POINT, &data, 8 + data.length,
                      nullptr, 0, &returned, nullptr) != 0;
  const DWORD error = GetLastError();
  CloseHandle(handle);
  if (!created) {
    throw std::system_error(static_cast<int>(error), std::system_category());
  }
}
}  // namespace martianlabs::doba::tests::unit

#endif
