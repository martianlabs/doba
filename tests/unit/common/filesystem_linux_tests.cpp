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

#ifdef __linux__

#include <array>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <sys/fsuid.h>

#include "common/filesystem.h"
#include "common/reader.h"
#include "test_helper.h"

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
}  // namespace

// +===========================================================================+
// | [>] filesystem rejects Linux links and special files        ( test-case ) |
// +===========================================================================+
DOBA_TEST("filesystem rejects Linux links and special files") {
  file_directory directory;
  file_directory outside;
  outside.write("secret", "secret");
  fs::create_symlink(outside.path() / "secret", directory.path() / "link");
  fs::create_directory_symlink(outside.path(), directory.path() / "linked");
  fs::create_symlink("missing", directory.path() / "broken");
  DOBA_EXPECT_EQUAL(::mkfifo((directory.path() / "pipe").c_str(), 0600), 0);
  filesystem_file file;
  std::error_code error;
  for (std::string_view path : {"link", "linked/secret", "broken", "pipe"}) {
    DOBA_EXPECT(!file.open(directory.path(), path, error));
    DOBA_EXPECT(error == std::errc::permission_denied);
  }
}

// +===========================================================================+
// | [>] filesystem Linux replacement                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("filesystem Linux descriptor remains bound after replacement") {
  file_directory directory;
  file_directory outside;
  fs::create_directory(directory.path() / "sub");
  directory.write("sub/data", "inside");
  outside.write("data", "outside");
  filesystem_file file;
  std::error_code error;
  DOBA_EXPECT(file.open(directory.path(), "sub/data", error));
  fs::rename(directory.path() / "sub", directory.path() / "old");
  fs::create_directory_symlink(outside.path(), directory.path() / "sub");
  std::array<std::byte, 8> bytes{};
  DOBA_EXPECT_EQUAL(file.read(bytes), 6);
  DOBA_EXPECT_EQUAL(std::string_view(
      reinterpret_cast<char*>(bytes.data()), 6), "inside");
  DOBA_EXPECT(!file.open(directory.path(), "sub/data", error));
  DOBA_EXPECT(error == std::errc::permission_denied);
}


// +===========================================================================+
// | [>] filesystem Linux rejects a replaced root                ( test-case ) |
// +===========================================================================+
DOBA_TEST("filesystem Linux rejects a replaced root") {
  file_directory directory;
  file_directory outside;
  fs::create_directory(directory.path() / "root");
  directory.write("root/data", "inside");
  outside.write("data", "outside");
  std::error_code error;
  const auto root = martianlabs::doba::common::filesystem_root(
      directory.path() / "root", error);
  DOBA_EXPECT(!error);
  fs::rename(root, directory.path() / "old");
  fs::create_directory_symlink(outside.path(), root);
  filesystem_file file;
  DOBA_EXPECT(!file.open(root, "data", error));
  DOBA_EXPECT(error == std::errc::permission_denied);
}

// +===========================================================================+
// | [>] filesystem Linux reports access denial                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("filesystem Linux reports access denial") {
  file_directory directory;
  directory.write("private", "secret");
  const auto path = directory.path() / "private";
  fs::permissions(path, fs::perms::none);
  filesystem_file file;
  std::error_code error;
  const int previous = ::setfsuid(static_cast<uid_t>(-1));
  if (previous == 0) ::setfsuid(65534);
  const bool opened = file.open(directory.path(), "private", error);
  ::setfsuid(static_cast<uid_t>(previous));
  fs::permissions(path, fs::perms::owner_all);
  DOBA_EXPECT(!opened);
  DOBA_EXPECT(error == std::errc::permission_denied);
}

#endif
