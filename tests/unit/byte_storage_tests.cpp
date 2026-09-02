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
#include <atomic>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>

#include "common/byte_storage.h"
#include "common/reader.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::byte_storage;
using martianlabs::doba::common::byte_storage_options;
using martianlabs::doba::common::reader;

class spill_directory {
 public:
  spill_directory() {
    namespace fs = std::filesystem;
    static std::atomic<std::size_t> sequence{0};
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    path_ = fs::temp_directory_path() /
            ("doba_byte_storage_" + std::to_string(stamp) + "_" +
             std::to_string(sequence.fetch_add(1)));
    fs::create_directory(path_);
  }
  ~spill_directory() {
    std::error_code error;
    std::filesystem::remove_all(path_, error);
  }

  const std::filesystem::path& path() const { return path_; }

 private:
  std::filesystem::path path_;
};

std::filesystem::path only_spill_file(const std::filesystem::path& directory) {
  std::filesystem::path result;
  for (const auto& entry : std::filesystem::directory_iterator(directory)) {
    if (entry.path().filename() != "existing.tmp") result = entry.path();
  }
  return result;
}
}  // namespace

// +===========================================================================+
// | [>] spilling preserves existing files                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("spilling preserves existing files") {
  spill_directory directory;
  const auto existing = directory.path() / "existing.tmp";
  {
    std::ofstream stream(existing, std::ios::binary);
    stream << "preserved";
  }
  {
    byte_storage storage(byte_storage_options{.spill_threshold = 1,
                                              .spill_dir = directory.path().string()});
    DOBA_EXPECT(storage.write("body", 4));
    storage.finish(4);
    DOBA_EXPECT(storage.ok());
  }
  std::ifstream stream(existing, std::ios::binary);
  std::string content((std::istreambuf_iterator<char>(stream)), {});
  DOBA_EXPECT_EQUAL(content, "preserved");
}
// +===========================================================================+
// | [>] truncated spill files fail the reader                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("truncated spill files fail the reader") {
  spill_directory directory;
  byte_storage storage(byte_storage_options{.spill_threshold = 1,
                                            .spill_dir = directory.path().string()});
  DOBA_EXPECT(storage.write("abcdef", 6));
  storage.finish(6);
  const auto spill_file = only_spill_file(directory.path());
  DOBA_EXPECT(!spill_file.empty());
  std::filesystem::resize_file(spill_file, 2);
  reader source(std::move(storage));
  std::array<std::byte, 6> output{};
  DOBA_EXPECT_EQUAL(source.read(output), 2);
  DOBA_EXPECT(source.failed());
  DOBA_EXPECT(!source.eof());
}
