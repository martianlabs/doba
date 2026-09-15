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

#ifdef _WIN32

#include <array>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <span>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>

#include "common/filesystem.h"
#include "common/reader.h"
#include "test_helper.h"

#include <winioctl.h>

namespace {
namespace fs = std::filesystem;
using martianlabs::doba::common::filesystem_file;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] file_directory                                              ( class ) |
// +---------------------------------------------------------------------------+
// | Internal implementation detail.                                           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class file_directory {
 public:
  // +=========================================================================+
  // | [>] METHODs                                                  ( public ) |
  // +=========================================================================+
  file_directory() {
    static std::atomic<unsigned int> counter{0};
    const auto stamp =
        std::chrono::steady_clock::now().time_since_epoch().count();
    do {
      path_ = fs::temp_directory_path() /
          ("doba_files_" + std::to_string(stamp) + "_" +
           std::to_string(counter.fetch_add(1)));
    } while (!fs::create_directory(path_));
  }
  ~file_directory() {
    std::error_code error;
    fs::remove_all(path_, error);
  }
  const fs::path& path() const { return path_; }
  void write(std::string_view name, std::string_view contents) {
    const fs::path relative(std::u8string(name.begin(), name.end()));
    std::ofstream output(path_ / relative, std::ios::binary);
    output.write(contents.data(),
                 static_cast<std::streamsize>(contents.size()));
    if (!output) throw std::runtime_error("Unable to write fixture");
  }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  fs::path path_;
};
void create_junction(const fs::path& path, const fs::path& target) {
  fs::create_directory(path);
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
  const bool created = DeviceIoControl(
      handle, FSCTL_SET_REPARSE_POINT, &data, 8 + data.length,
      nullptr, 0, &returned, nullptr) != 0;
  const DWORD error = GetLastError();
  CloseHandle(handle);
  if (!created) {
    throw std::system_error(static_cast<int>(error), std::system_category());
  }
}
}  // namespace

// +===========================================================================+
// | [>] filesystem Windows path rejection                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("filesystem rejects Windows device stream and alias paths") {
  file_directory directory;
  directory.write("file", "data");
  filesystem_file file;
  std::error_code error;
  for (std::string_view path : {"NUL", "con.txt", "COM1", "LPT1.txt",
                                "file:stream", "file.", "file ", "*.txt"}) {
    DOBA_EXPECT(!file.open(directory.path(), path, error));
    DOBA_EXPECT(error == std::errc::permission_denied);
  }
}

// +===========================================================================+
// | [>] filesystem Windows releases its owned handle            ( test-case ) |
// +===========================================================================+
DOBA_TEST("filesystem Windows releases its owned handle") {
  file_directory directory;
  directory.write("file", "data");
  const auto path = directory.path() / "file";
  {
    filesystem_file file;
    std::error_code error;
    DOBA_EXPECT(file.open(directory.path(), "file", error));
    HANDLE exclusive = CreateFileW(path.c_str(), GENERIC_READ, 0, nullptr,
                                    OPEN_EXISTING, 0, nullptr);
    DOBA_EXPECT(exclusive == INVALID_HANDLE_VALUE);
    if (exclusive != INVALID_HANDLE_VALUE) CloseHandle(exclusive);
  }
  HANDLE exclusive = CreateFileW(path.c_str(), GENERIC_READ, 0, nullptr,
                                  OPEN_EXISTING, 0, nullptr);
  DOBA_EXPECT(exclusive != INVALID_HANDLE_VALUE);
  if (exclusive != INVALID_HANDLE_VALUE) CloseHandle(exclusive);
}


// +===========================================================================+
// | [>] filesystem Windows rejects reparse points               ( test-case ) |
// +===========================================================================+
DOBA_TEST("filesystem Windows rejects reparse points") {
  file_directory directory;
  file_directory outside;
  outside.write("data", "outside");
  create_junction(directory.path() / "linked", outside.path());
  filesystem_file file;
  std::error_code error;
  for (std::string_view path : {"linked", "linked/data"}) {
    DOBA_EXPECT(!file.open(directory.path(), path, error));
    DOBA_EXPECT(error == std::errc::permission_denied);
  }
  DOBA_EXPECT(fs::remove(directory.path() / "linked"));
}

// +===========================================================================+
// | [>] filesystem Windows keeps opened files after replacement ( test-case ) |
// +===========================================================================+
DOBA_TEST("filesystem Windows keeps opened files after replacement") {
  file_directory directory;
  file_directory outside;
  fs::create_directory(directory.path() / "root");
  directory.write("root/data", "inside");
  outside.write("data", "outside");
  std::error_code error;
  const auto root = martianlabs::doba::common::filesystem_root(
      directory.path() / "root", error);
  DOBA_EXPECT(!error);
  filesystem_file file;
  DOBA_EXPECT(file.open(root, "data", error));
  fs::rename(root / "data", root / "original");
  directory.write("root/data", "replacement");
  std::array<std::byte, 8> bytes{};
  DOBA_EXPECT_EQUAL(file.read(bytes), 6);
  DOBA_EXPECT_EQUAL(std::string_view(
      reinterpret_cast<char*>(bytes.data()), 6), "inside");
  file.close();
  fs::rename(root, directory.path() / "old");
  create_junction(root, outside.path());
  DOBA_EXPECT(!file.open(root, "data", error));
  DOBA_EXPECT(error == std::errc::permission_denied);
  DOBA_EXPECT(fs::remove(root));
}


// +===========================================================================+
// | [>] filesystem Windows reparse mutation race ( test-case )                |
// +===========================================================================+
DOBA_TEST("filesystem Windows confines concurrent reparse mutations") {
  file_directory directory;
  file_directory outside;
  const auto root = directory.path() / "root";
  fs::create_directory(root);
  outside.write("secret", "outside");
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
  const std::wstring name = L"\\??\\" + outside.path().native();
  DOBA_EXPECT(name.size() + 2 <= std::size(data.path));
  data.tag = IO_REPARSE_TAG_MOUNT_POINT;
  data.substitute_length = static_cast<WORD>(name.size() * sizeof(wchar_t));
  data.print_offset = data.substitute_length + sizeof(wchar_t);
  data.length = 8 + data.substitute_length + 2 * sizeof(wchar_t);
  std::copy(name.begin(), name.end(), data.path);
  struct {
    DWORD tag;
    WORD length;
    WORD reserved;
  } clear{IO_REPARSE_TAG_MOUNT_POINT, 0, 0};
  HANDLE mutation = CreateFileW(
      root.c_str(), FILE_WRITE_ATTRIBUTES,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
      nullptr, OPEN_EXISTING,
      FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
  DOBA_EXPECT(mutation != INVALID_HANDLE_VALUE);
  std::atomic<unsigned int> changes{0};
  std::atomic<bool> failed{false};
  unsigned int escaped = 0;
  {
    std::jthread mutator([&](std::stop_token token) {
      DWORD returned = 0;
      while (!token.stop_requested()) {
        if (!DeviceIoControl(mutation, FSCTL_SET_REPARSE_POINT,
                             &data, 8 + data.length, nullptr, 0,
                             &returned, nullptr)) {
          failed = true;
          break;
        }
        changes++;
        std::this_thread::yield();
        if (!DeviceIoControl(
                mutation, FSCTL_DELETE_REPARSE_POINT,
                &clear, sizeof(clear), nullptr, 0, &returned, nullptr)) {
          failed = true;
          break;
        }
      }
    });
    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(2);
    while (!changes && !failed &&
           std::chrono::steady_clock::now() < deadline) {
      std::this_thread::yield();
    }
    for (unsigned int i = 0; i < 2000; i++) {
      filesystem_file file;
      std::error_code error;
      if (file.open(root, "secret", error)) escaped++;
    }
  }
  CloseHandle(mutation);
  DOBA_EXPECT(!failed);
  DOBA_EXPECT(changes.load() > 0);
  DOBA_EXPECT_EQUAL(escaped, 0);
}

#endif
