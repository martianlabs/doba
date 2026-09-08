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
#include <barrier>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <optional>
#include <thread>
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
    const auto stamp =
        std::chrono::steady_clock::now().time_since_epoch().count();
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
    byte_storage storage(
        byte_storage_options{.spill_threshold = 1,
                             .spill_dir = directory.path().string()});
    DOBA_EXPECT(storage.write("body", 4));
    storage.finish(4);
    DOBA_EXPECT(storage.ok());
  }
  DOBA_EXPECT(only_spill_file(directory.path()).empty());
  std::ifstream stream(existing, std::ios::binary);
  std::string content((std::istreambuf_iterator<char>(stream)), {});
  DOBA_EXPECT_EQUAL(content, "preserved");
}
// +===========================================================================+
// | [>] truncated spill files fail the reader                  ( test-case )  |
// +===========================================================================+
DOBA_TEST("truncated spill files fail the reader") {
  spill_directory directory;
  {
    byte_storage storage(
        byte_storage_options{.spill_threshold = 1,
                             .spill_dir = directory.path().string()});
    DOBA_EXPECT(storage.write("abcdef", 6));
    storage.finish(6);
    DOBA_EXPECT_EQUAL(
        std::distance(std::filesystem::directory_iterator(directory.path()),
                      std::filesystem::directory_iterator()), 1);
    const auto spill_file = only_spill_file(directory.path());
    DOBA_EXPECT(!spill_file.empty());
    std::filesystem::resize_file(spill_file, 2);
    reader source(std::move(storage));
    std::array<std::byte, 6> output{};
    DOBA_EXPECT_EQUAL(source.read(output), 2);
    DOBA_EXPECT(source.failed());
    DOBA_EXPECT(!source.eof());
    DOBA_EXPECT_EQUAL(source.read(output), 0);
    std::byte byte{};
    DOBA_EXPECT(!source.fetch(byte));
    DOBA_EXPECT(source.failed());
  }
  DOBA_EXPECT(std::filesystem::is_empty(directory.path()));
}
// +===========================================================================+
// | [>] finishing seals memory and spilled storage              ( test-case ) |
// +===========================================================================+
DOBA_TEST("finishing seals memory and spilled storage") {
  spill_directory directory;
  byte_storage_options cases[] = {
      {},
      {.spill_threshold = 1, .spill_dir = directory.path().string()},
  };
  for (auto& options : cases) {
    byte_storage storage(std::move(options));
    DOBA_EXPECT(storage.write("body", 4));
    storage.finish(4);
    DOBA_EXPECT(!storage.write("x", 1));
    DOBA_EXPECT(!storage.write(nullptr, 0));
    storage.finish(1);
    DOBA_EXPECT_EQUAL(storage.total_size().value(), 4);
    std::array<char, 4> output{};
    DOBA_EXPECT_EQUAL(storage.read(output.data(), output.size()), 4);
    DOBA_EXPECT_EQUAL(std::string_view(output.data(), output.size()), "body");
  }
}

// +===========================================================================+
// | [>] memory threshold 6                                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("storage below the threshold remains in memory") {
  spill_directory directory;
  byte_storage storage(
      byte_storage_options{.spill_threshold = 8,
                           .spill_dir = directory.path().string()});
  DOBA_EXPECT(storage.write("abcdef", 6));
  storage.finish(6);
  DOBA_EXPECT_EQUAL(storage.total_size().value(), 6);
  DOBA_EXPECT(std::filesystem::is_empty(directory.path()));
  std::array<char, 6> output{};
  DOBA_EXPECT_EQUAL(storage.read(output.data(), output.size()), 6);
  DOBA_EXPECT_EQUAL(std::string_view(output.data(), output.size()), "abcdef");
  DOBA_EXPECT(storage.exhausted());
  DOBA_EXPECT(storage.ok());
}

// +===========================================================================+
// | [>] memory threshold 8                                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("storage at the threshold remains in memory") {
  spill_directory directory;
  byte_storage storage(
      byte_storage_options{.spill_threshold = 8,
                           .spill_dir = directory.path().string()});
  DOBA_EXPECT(storage.write("abcdefgh", 8));
  storage.finish(8);
  DOBA_EXPECT_EQUAL(storage.total_size().value(), 8);
  DOBA_EXPECT(std::filesystem::is_empty(directory.path()));
  std::array<char, 8> output{};
  DOBA_EXPECT_EQUAL(storage.read(output.data(), output.size()), 8);
  DOBA_EXPECT_EQUAL(std::string_view(output.data(), output.size()), "abcdefgh");
  DOBA_EXPECT(storage.exhausted());
  DOBA_EXPECT(storage.ok());
}

// +===========================================================================+
// | [>] spill preserves memory prefix                           ( test-case ) |
// +===========================================================================+
DOBA_TEST("crossing the threshold preserves the memory prefix") {
  spill_directory directory;
  {
    byte_storage storage(
        byte_storage_options{.spill_threshold = 8,
                             .spill_dir = directory.path().string()});
    DOBA_EXPECT(storage.write("abcdef", 6));
    DOBA_EXPECT(std::filesystem::is_empty(directory.path()));
    DOBA_EXPECT(storage.write("GHIJ", 4));
    storage.finish(10);
    DOBA_EXPECT_EQUAL(
        std::distance(std::filesystem::directory_iterator(directory.path()),
                      std::filesystem::directory_iterator()), 1);
    std::array<char, 10> output{};
    DOBA_EXPECT_EQUAL(storage.read(output.data(), output.size()), 10);
    DOBA_EXPECT_EQUAL(std::string_view(output.data(), output.size()),
                      "abcdefGHIJ");
    DOBA_EXPECT(storage.exhausted());
    DOBA_EXPECT(storage.ok());
  }
  DOBA_EXPECT(std::filesystem::is_empty(directory.path()));
}

// +===========================================================================+
// | [>] zero spill threshold                                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("zero threshold disables spilling") {
  spill_directory directory;
  byte_storage storage(
      byte_storage_options{.spill_threshold = 0,
                           .spill_dir = directory.path().string()});
  for (std::size_t i = 0; i < 4; i++) DOBA_EXPECT(storage.write("abcdefgh", 8));
  storage.finish(32);
  DOBA_EXPECT(std::filesystem::is_empty(directory.path()));
  std::array<char, 32> output{};
  DOBA_EXPECT_EQUAL(storage.read(output.data(), output.size()), 32);
  DOBA_EXPECT_EQUAL(std::string_view(output.data(), output.size()),
                    "abcdefghabcdefghabcdefghabcdefgh");
  DOBA_EXPECT(storage.exhausted());
  DOBA_EXPECT(storage.ok());
}

// +===========================================================================+
// | [>] successive spill writes                                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("successive spill writes append without duplication") {
  spill_directory directory;
  {
    byte_storage storage(
        byte_storage_options{.spill_threshold = 8,
                             .spill_dir = directory.path().string()});
    DOBA_EXPECT(storage.write("abcdef", 6));
    DOBA_EXPECT(storage.write("GHIJ", 4));
    const auto spill = only_spill_file(directory.path());
    DOBA_EXPECT(!spill.empty());
    DOBA_EXPECT(storage.write("klmno", 5));
    storage.finish(15);
    DOBA_EXPECT_EQUAL(only_spill_file(directory.path()), spill);
    DOBA_EXPECT_EQUAL(
        std::distance(std::filesystem::directory_iterator(directory.path()),
                      std::filesystem::directory_iterator()), 1);
    std::array<char, 15> output{};
    DOBA_EXPECT_EQUAL(storage.read(output.data(), output.size()), 15);
    DOBA_EXPECT_EQUAL(std::string_view(output.data(), output.size()),
                      "abcdefGHIJklmno");
    DOBA_EXPECT(storage.exhausted());
    DOBA_EXPECT(storage.ok());
  }
  DOBA_EXPECT(std::filesystem::is_empty(directory.path()));
}

// +===========================================================================+
// | [>] binary storage round trip                               ( test-case ) |
// +===========================================================================+
DOBA_TEST("storage preserves every byte value") {
  spill_directory directory;
  std::string input(256, '\0');
  for (std::size_t i = 0; i < input.size(); i++) {
    input[i] = static_cast<char>(i);
  }
  for (std::size_t threshold : {0, 64}) {
    {
      byte_storage storage(
          byte_storage_options{.spill_threshold = threshold,
                               .spill_dir = directory.path().string()});
      DOBA_EXPECT(storage.write(input.data(), 63));
      DOBA_EXPECT(storage.write(input.data() + 63, input.size() - 63));
      storage.finish(input.size());
      std::string output(input.size(), '\0');
      DOBA_EXPECT_EQUAL(storage.read(output.data(), output.size()),
                        input.size());
      DOBA_EXPECT_EQUAL(output, input);
      DOBA_EXPECT(storage.exhausted());
      DOBA_EXPECT(storage.ok());
    }
    DOBA_EXPECT(std::filesystem::is_empty(directory.path()));
  }
}

// +===========================================================================+
// | [>] memory move construction                                ( test-case ) |
// +===========================================================================+
DOBA_TEST("memory move construction preserves the cursor") {
  spill_directory directory;
  {
    std::optional<byte_storage> destination;
    {
      byte_storage storage(
          byte_storage_options{.spill_threshold = 0,
                               .spill_dir = directory.path().string()});
      DOBA_EXPECT(storage.write("abcdef", 6));
      storage.finish(6);
      std::array<char, 2> prefix{};
      DOBA_EXPECT_EQUAL(storage.read(prefix.data(), prefix.size()), 2);
      DOBA_EXPECT_EQUAL(std::string_view(prefix.data(), prefix.size()), "ab");
      destination.emplace(std::move(storage));
    }
    DOBA_EXPECT_EQUAL(std::filesystem::is_empty(directory.path()), true);
    DOBA_EXPECT_EQUAL(destination->total_size().value(), 6);
    std::array<char, 4> output{};
    DOBA_EXPECT_EQUAL((*destination).read(output.data(), output.size()), 4);
    DOBA_EXPECT_EQUAL(std::string_view(output.data(), output.size()), "cdef");
    DOBA_EXPECT((*destination).exhausted());
    DOBA_EXPECT((*destination).ok());
  }
  DOBA_EXPECT(std::filesystem::is_empty(directory.path()));
}

// +===========================================================================+
// | [>] spill move construction                                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("spill move construction transfers cursor and ownership") {
  spill_directory directory;
  {
    std::optional<byte_storage> destination;
    {
      byte_storage storage(
          byte_storage_options{.spill_threshold = 1,
                               .spill_dir = directory.path().string()});
      DOBA_EXPECT(storage.write("abcdef", 6));
      storage.finish(6);
      std::array<char, 2> prefix{};
      DOBA_EXPECT_EQUAL(storage.read(prefix.data(), prefix.size()), 2);
      DOBA_EXPECT_EQUAL(std::string_view(prefix.data(), prefix.size()), "ab");
      destination.emplace(std::move(storage));
    }
    DOBA_EXPECT_EQUAL(std::filesystem::is_empty(directory.path()), false);
    DOBA_EXPECT_EQUAL(destination->total_size().value(), 6);
    std::array<char, 4> output{};
    DOBA_EXPECT_EQUAL((*destination).read(output.data(), output.size()), 4);
    DOBA_EXPECT_EQUAL(std::string_view(output.data(), output.size()), "cdef");
    DOBA_EXPECT((*destination).exhausted());
    DOBA_EXPECT((*destination).ok());
  }
  DOBA_EXPECT(std::filesystem::is_empty(directory.path()));
}

// +===========================================================================+
// | [>] move assignment spill to spill                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("spill move assignment releases the previous file") {
  spill_directory directory;
  {
    byte_storage destination(
        byte_storage_options{.spill_threshold = 1,
                             .spill_dir = directory.path().string()});
    DOBA_EXPECT(destination.write("old", 3));
    destination.finish(3);
    const auto previous_file = only_spill_file(directory.path());
    {
      byte_storage source(
          byte_storage_options{.spill_threshold = 1,
                               .spill_dir = directory.path().string()});
      DOBA_EXPECT(source.write("abcdef", 6));
      source.finish(6);
      std::array<char, 2> prefix{};
      DOBA_EXPECT_EQUAL(source.read(prefix.data(), prefix.size()), 2);
      DOBA_EXPECT_EQUAL(std::string_view(prefix.data(), prefix.size()), "ab");
      destination = std::move(source);
      if (!previous_file.empty()) {
        DOBA_EXPECT(!std::filesystem::exists(previous_file));
      }
    }
    DOBA_EXPECT_EQUAL(
        std::distance(std::filesystem::directory_iterator(directory.path()),
                      std::filesystem::directory_iterator()), 1);
    DOBA_EXPECT_EQUAL(destination.total_size().value(), 6);
    std::array<char, 4> output{};
    DOBA_EXPECT_EQUAL(destination.read(output.data(), output.size()), 4);
    DOBA_EXPECT_EQUAL(std::string_view(output.data(), output.size()), "cdef");
    DOBA_EXPECT(destination.exhausted());
    DOBA_EXPECT(destination.ok());
  }
  DOBA_EXPECT(std::filesystem::is_empty(directory.path()));
}

// +===========================================================================+
// | [>] move assignment memory to spill                         ( test-case ) |
// +===========================================================================+
DOBA_TEST("memory move assignment releases a spilled destination") {
  spill_directory directory;
  {
    byte_storage destination(
        byte_storage_options{.spill_threshold = 1,
                             .spill_dir = directory.path().string()});
    DOBA_EXPECT(destination.write("old", 3));
    destination.finish(3);
    const auto previous_file = only_spill_file(directory.path());
    {
      byte_storage source(
          byte_storage_options{.spill_threshold = 0,
                               .spill_dir = directory.path().string()});
      DOBA_EXPECT(source.write("abcdef", 6));
      source.finish(6);
      std::array<char, 2> prefix{};
      DOBA_EXPECT_EQUAL(source.read(prefix.data(), prefix.size()), 2);
      DOBA_EXPECT_EQUAL(std::string_view(prefix.data(), prefix.size()), "ab");
      destination = std::move(source);
      if (!previous_file.empty()) {
        DOBA_EXPECT(!std::filesystem::exists(previous_file));
      }
    }
    DOBA_EXPECT_EQUAL(
        std::distance(std::filesystem::directory_iterator(directory.path()),
                      std::filesystem::directory_iterator()), 0);
    DOBA_EXPECT_EQUAL(destination.total_size().value(), 6);
    std::array<char, 4> output{};
    DOBA_EXPECT_EQUAL(destination.read(output.data(), output.size()), 4);
    DOBA_EXPECT_EQUAL(std::string_view(output.data(), output.size()), "cdef");
    DOBA_EXPECT(destination.exhausted());
    DOBA_EXPECT(destination.ok());
  }
  DOBA_EXPECT(std::filesystem::is_empty(directory.path()));
}

// +===========================================================================+
// | [>] move assignment spill to memory                         ( test-case ) |
// +===========================================================================+
DOBA_TEST("spill move assignment replaces memory and preserves cursor") {
  spill_directory directory;
  {
    byte_storage destination(
        byte_storage_options{.spill_threshold = 0,
                             .spill_dir = directory.path().string()});
    DOBA_EXPECT(destination.write("old", 3));
    destination.finish(3);
    const auto previous_file = only_spill_file(directory.path());
    {
      byte_storage source(
          byte_storage_options{.spill_threshold = 1,
                               .spill_dir = directory.path().string()});
      DOBA_EXPECT(source.write("abcdef", 6));
      source.finish(6);
      std::array<char, 2> prefix{};
      DOBA_EXPECT_EQUAL(source.read(prefix.data(), prefix.size()), 2);
      DOBA_EXPECT_EQUAL(std::string_view(prefix.data(), prefix.size()), "ab");
      destination = std::move(source);
      if (!previous_file.empty()) {
        DOBA_EXPECT(!std::filesystem::exists(previous_file));
      }
    }
    DOBA_EXPECT_EQUAL(
        std::distance(std::filesystem::directory_iterator(directory.path()),
                      std::filesystem::directory_iterator()), 1);
    DOBA_EXPECT_EQUAL(destination.total_size().value(), 6);
    std::array<char, 4> output{};
    DOBA_EXPECT_EQUAL(destination.read(output.data(), output.size()), 4);
    DOBA_EXPECT_EQUAL(std::string_view(output.data(), output.size()), "cdef");
    DOBA_EXPECT(destination.exhausted());
    DOBA_EXPECT(destination.ok());
  }
  DOBA_EXPECT(std::filesystem::is_empty(directory.path()));
}

// +===========================================================================+
// | [>] persistent spill creation error                         ( test-case ) |
// +===========================================================================+
DOBA_TEST("spill creation failure remains observable") {
  spill_directory directory;
  const auto blocked = directory.path() / "existing.tmp";
  {
    std::ofstream file(blocked);
    file << "preserved";
  }
  byte_storage storage(
      byte_storage_options{.spill_threshold = 1,
                           .spill_dir = blocked.string()});
  DOBA_EXPECT(!storage.write("abcdef", 6));
  DOBA_EXPECT(!storage.ok());
  DOBA_EXPECT(!storage.write("x", 1));
  DOBA_EXPECT(!storage.write(nullptr, 0));
  storage.finish(6);
  std::array<char, 6> output{};
  DOBA_EXPECT_EQUAL(storage.read(output.data(), output.size()), 0);
  std::byte byte{};
  DOBA_EXPECT(!storage.fetch(byte));
  DOBA_EXPECT(!storage.ok());
  DOBA_EXPECT(only_spill_file(directory.path()).empty());
  std::ifstream file(blocked);
  std::string content((std::istreambuf_iterator<char>(file)), {});
  DOBA_EXPECT_EQUAL(content, "preserved");
}

// +===========================================================================+
// | [>] shared read and fetch cursor                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("read and fetch share one storage cursor") {
  spill_directory directory;
  for (std::size_t threshold : {0, 1}) {
    {
      byte_storage storage(
          byte_storage_options{.spill_threshold = threshold,
                               .spill_dir = directory.path().string()});
      DOBA_EXPECT(storage.write("abcdef", 6));
      storage.finish(6);
      std::array<char, 2> prefix{};
      DOBA_EXPECT_EQUAL(storage.read(prefix.data(), prefix.size()), 2);
      DOBA_EXPECT_EQUAL(std::string_view(prefix.data(), prefix.size()), "ab");
      std::byte byte{};
      DOBA_EXPECT(storage.fetch(byte));
      DOBA_EXPECT_EQUAL(byte, std::byte{'c'});
      std::array<char, 4> tail{};
      DOBA_EXPECT_EQUAL(storage.read(tail.data(), tail.size()), 3);
      DOBA_EXPECT_EQUAL(std::string_view(tail.data(), 3), "def");
      DOBA_EXPECT(storage.exhausted());
      DOBA_EXPECT(!storage.fetch(byte));
      DOBA_EXPECT(storage.ok());
    }
    DOBA_EXPECT(std::filesystem::is_empty(directory.path()));
  }
}

// +===========================================================================+
// | [>] reader owns moved storage                               ( test-case ) |
// +===========================================================================+
DOBA_TEST("reader owns storage after the moved source is destroyed") {
  spill_directory directory;
  for (std::size_t threshold : {0, 1}) {
    {
      std::optional<reader> destination;
      {
        byte_storage storage(
            byte_storage_options{.spill_threshold = threshold,
                                 .spill_dir = directory.path().string()});
        DOBA_EXPECT(storage.write("abcdef", 6));
        storage.finish(6);
        destination.emplace(std::move(storage));
      }
      DOBA_EXPECT_EQUAL(std::filesystem::is_empty(directory.path()),
                        threshold == 0);
      std::string output;
      DOBA_EXPECT_EQUAL(destination->read_all(output), 6);
      DOBA_EXPECT_EQUAL(output, "abcdef");
      DOBA_EXPECT(destination->eof());
      DOBA_EXPECT(!destination->failed());
    }
    DOBA_EXPECT(std::filesystem::is_empty(directory.path()));
  }
}

// +===========================================================================+
// | [>] concurrent spill ownership                              ( test-case ) |
// +===========================================================================+
DOBA_TEST("concurrent spill owners keep distinct files and contents") {
  spill_directory directory;
  {
    constexpr std::size_t count = 8;
    std::array<byte_storage, count> storage;
    std::array<bool, count> written{};
    std::barrier start(static_cast<std::ptrdiff_t>(count));
    std::array<std::jthread, count> workers;
    for (std::size_t i = 0; i < count; i++) {
      workers[i] = std::jthread([&, i]() {
        start.arrive_and_wait();
        storage[i] = byte_storage(
            byte_storage_options{.spill_threshold = 1,
                                 .spill_dir = directory.path().string()});
        const std::string value = "owner-" + std::to_string(i);
        written[i] = storage[i].write(value.data(), value.size());
        storage[i].finish(value.size());
      });
    }
    for (auto& worker : workers) worker.join();
    DOBA_EXPECT_EQUAL(
        std::distance(std::filesystem::directory_iterator(directory.path()),
                      std::filesystem::directory_iterator()), count);
    for (std::size_t i = 0; i < count; i++) {
      DOBA_EXPECT(written[i]);
      std::array<char, 7> output{};
      DOBA_EXPECT_EQUAL(storage[i].read(output.data(), output.size()), 7);
      DOBA_EXPECT_EQUAL(std::string_view(output.data(), output.size()),
                        "owner-" + std::to_string(i));
      DOBA_EXPECT(storage[i].ok());
    }
  }
  DOBA_EXPECT(std::filesystem::is_empty(directory.path()));
}

// +===========================================================================+
// | [>] empty storage lifecycle                                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("empty storage finishes without an error") {
  byte_storage storage;
  DOBA_EXPECT(storage.write(nullptr, 0));
  storage.finish(0);
  DOBA_EXPECT_EQUAL(storage.total_size().value(), 0);
  DOBA_EXPECT(storage.exhausted());
  DOBA_EXPECT_EQUAL(storage.read(nullptr, 0), 0);
  std::byte byte{};
  DOBA_EXPECT(!storage.fetch(byte));
  DOBA_EXPECT(storage.ok());
}

// +===========================================================================+
// | [>] borrowed reader move semantics                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("moving a borrowed reader preserves the view and cursor") {
  std::string backing = "abcdef";
  auto source = reader::borrowed(std::as_bytes(std::span(backing)));
  std::array<std::byte, 2> prefix{};
  DOBA_EXPECT_EQUAL(source.read(prefix), 2);
  DOBA_EXPECT_EQUAL(
      std::string_view(reinterpret_cast<const char*>(prefix.data()), 2), "ab");
  reader destination(std::move(source));
  backing[2] = 'C';
  std::string output;
  DOBA_EXPECT_EQUAL(destination.read_all(output), 4);
  DOBA_EXPECT_EQUAL(output, "Cdef");
  DOBA_EXPECT(destination.eof());
  DOBA_EXPECT(!destination.failed());
}
