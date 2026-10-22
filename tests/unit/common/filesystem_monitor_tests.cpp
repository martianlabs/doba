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
// Copyright 2026 martianLabs
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
#include <filesystem>
#include <iterator>
#include <stdexcept>
#include <thread>
#include <vector>

#include "common/filesystem.h"
#include "directory_link.h"
#include "file_directory.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::filesystem_monitor;
using martianlabs::doba::tests::file_directory;
namespace fs = std::filesystem;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] changed                                                 ( function ) |
// +---------------------------------------------------------------------------+
// | Poll the native completion, including asynchronous Windows delivery.      |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
bool changed(filesystem_monitor& monitor, std::uint64_t version) {
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
  do {
    std::error_code error;
    if (!monitor.unchanged(version, error)) return true;
    std::this_thread::yield();
  } while (std::chrono::steady_clock::now() < deadline);
  return false;
}
}  // namespace

// +===========================================================================+
// | [>] monitor observations and writes                         ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("filesystem monitor retains generations until a native change") {
  file_directory directory;
  directory.write("file", "first");
  filesystem_monitor monitor(directory.path());
  std::error_code error;
  const auto version = monitor.observe("file", error);
  DOBA_EXPECT(version != 0);
  DOBA_EXPECT(!error);
  for (unsigned i = 0; i < 100; ++i) DOBA_EXPECT(monitor.unchanged(version, error));
  directory.write("file", "other");
  DOBA_EXPECT(changed(monitor, version));
  const auto next = monitor.observe("file", error);
  DOBA_EXPECT(next > version);
  DOBA_EXPECT(monitor.unchanged(next, error));
}

// +===========================================================================+
// | [>] monitor attributes and replacement                      ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("filesystem monitor detects attributes replacements and deletion") {
  file_directory directory;
  directory.write("file", "same");
  filesystem_monitor monitor(directory.path());
  std::error_code error;
  auto version = monitor.observe("file", error);
  DOBA_EXPECT(version != 0);
  const auto timestamp = fs::last_write_time(directory.path() / "file");
  fs::last_write_time(directory.path() / "file", timestamp + std::chrono::seconds(2));
  DOBA_EXPECT(changed(monitor, version));
  version = monitor.observe("file", error);
  DOBA_EXPECT(version != 0);
  fs::rename(directory.path() / "file", directory.path() / "previous");
  directory.write("file", "same");
  fs::last_write_time(directory.path() / "file", timestamp);
  DOBA_EXPECT(changed(monitor, version));
  version = monitor.observe("file", error);
  DOBA_EXPECT(version != 0);
  fs::remove(directory.path() / "file");
  DOBA_EXPECT(changed(monitor, version));
}

// +===========================================================================+
// | [>] monitor directory coverage                              ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("filesystem monitor rebuilds coverage after directory changes") {
  file_directory directory;
  fs::create_directories(directory.path() / "sub" / "nested");
  directory.write("sub/nested/file", "first");
  filesystem_monitor monitor(directory.path());
  std::error_code error;
  auto version = monitor.observe("sub/nested/file", error);
  DOBA_EXPECT(version != 0);
  fs::rename(directory.path() / "sub", directory.path() / "previous");
  DOBA_EXPECT(changed(monitor, version));
  fs::create_directories(directory.path() / "sub" / "nested");
  directory.write("sub/nested/file", "other");
  version = monitor.observe("sub/nested/file", error);
  // Registration may encounter the queued rename and request a retry.
  if (!version) version = monitor.observe("sub/nested/file", error);
  DOBA_EXPECT(version != 0);
  directory.write("sub/nested/file", "again");
  DOBA_EXPECT(changed(monitor, version));
}

// +===========================================================================+
// | [>] monitor reset and resources                             ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("filesystem monitor reset and resource limits never preserve old tokens") {
  file_directory directory;
  directory.write("file", "same");
  filesystem_monitor monitor(directory.path());
  std::error_code error;
  const auto old = monitor.observe("file", error);
  DOBA_EXPECT(old != 0);
  monitor.reset();
  DOBA_EXPECT(!monitor.unchanged(old, error));
  const auto next = monitor.observe("file", error);
  DOBA_EXPECT(next > old);
  DOBA_EXPECT(!monitor.unchanged(old, error));
  filesystem_monitor disabled(directory.path(), 0);
  DOBA_EXPECT_EQUAL(disabled.observe("file", error), 0);
  DOBA_EXPECT(static_cast<bool>(error));
  filesystem_monitor limited(directory.path(), 1);
  DOBA_EXPECT_EQUAL(limited.observe("file", error), 0);
  DOBA_EXPECT(static_cast<bool>(error));
  DOBA_EXPECT_EQUAL(limited.poll(error), 0);
}

// +===========================================================================+
// | [>] monitor invalid paths                                   ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("filesystem monitor rejects invalid observation paths") {
  file_directory directory;
  filesystem_monitor monitor(directory.path());
  std::error_code error;
  for (std::string_view path : {"", "../file", "/file", "a:b", "sub/../file"}) {
    DOBA_EXPECT_EQUAL(monitor.observe(path, error), 0);
    DOBA_EXPECT(static_cast<bool>(error));
  }
  filesystem_monitor absent(directory.path() / "missing");
  DOBA_EXPECT_EQUAL(absent.observe("file", error), 0);
  DOBA_EXPECT(static_cast<bool>(error));
}

// +===========================================================================+
// | [>] monitor concurrent callers                              ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("filesystem monitor serializes callers without a consumer thread") {
  file_directory directory;
  directory.write("file", "same");
  filesystem_monitor monitor(directory.path());
  std::error_code error;
  const auto version = monitor.observe("file", error);
  DOBA_EXPECT(version != 0);
  std::atomic<unsigned> correct{0};
  {
    std::vector<std::jthread> callers;
    for (unsigned i = 0; i < 8; ++i) callers.emplace_back([&] {
      for (unsigned n = 0; n < 100; ++n) {
        std::error_code local;
        if (monitor.unchanged(version, local)) ++correct;
      }
    });
  }
  DOBA_EXPECT_EQUAL(correct.load(), 800);
  directory.write("file", "next");
  DOBA_EXPECT(changed(monitor, version));
}

// +===========================================================================+
// | [>] monitor replaced root                                   ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("filesystem monitor invalidates a replaced root before reuse") {
  file_directory directory;
  file_directory outside;
  fs::create_directory(directory.path() / "root");
  directory.write("root/file", "first");
  outside.write("file", "other");
  const auto root = directory.path() / "root";
  filesystem_monitor monitor(root);
  std::error_code error;
  const auto version = monitor.observe("file", error);
  DOBA_EXPECT(version != 0);
  fs::rename(root, directory.path() / "previous");
  martianlabs::doba::tests::unit::create_directory_link(root, outside.path());
  DOBA_EXPECT(changed(monitor, version));
  DOBA_EXPECT(!monitor.unchanged(version, error));
  fs::remove(root);
}

#ifdef __linux__
// +===========================================================================+
// | [>] monitor hard-link writes                                ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("filesystem monitor observes inode writes through an external hard link") {
  file_directory directory;
  file_directory outside;
  directory.write("file", "first");
  fs::create_hard_link(directory.path() / "file", outside.path() / "alias");
  filesystem_monitor monitor(directory.path());
  std::error_code error;
  const auto version = monitor.observe("file", error);
  DOBA_EXPECT(version != 0);
  outside.write("alias", "other");
  DOBA_EXPECT(changed(monitor, version));
}

// +===========================================================================+
// | [>] monitor native queue overflow                           ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("filesystem monitor loses coverage safely on inotify queue overflow") {
  file_directory directory;
  directory.write("file", "first");
  filesystem_monitor monitor(directory.path());
  std::error_code error;
  const auto version = monitor.observe("file", error);
  DOBA_EXPECT(version != 0);
  const auto descriptor = ::open((directory.path() / "file").c_str(), O_WRONLY | O_CLOEXEC);
  DOBA_EXPECT(descriptor >= 0);
  for (unsigned i = 0; i < 20000; ++i) {
    const char byte = char(i & 0x7f);
    DOBA_EXPECT_EQUAL(::pwrite(descriptor, &byte, 1, 0), 1);
    DOBA_EXPECT_EQUAL(::fchmod(descriptor, i % 2 ? 0600 : 0640), 0);
  }
  ::close(descriptor);
  DOBA_EXPECT(!monitor.unchanged(version, error));
  const auto next = monitor.observe("file", error);
  DOBA_EXPECT(next > version);
  DOBA_EXPECT(monitor.unchanged(next, error));
}
#endif

// +===========================================================================+
// | [>] monitor own reads and failed extensions                  ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("filesystem monitor ignores own reads and preserves covered observations") {
  file_directory directory;
  fs::create_directory(directory.path() / "sub");
  directory.write("sub/file", "first");
  filesystem_monitor monitor(directory.path());
  std::error_code error;
  const auto version = monitor.observe("sub/file", error);
  DOBA_EXPECT(version != 0);
  {
    martianlabs::doba::common::filesystem_file file;
    DOBA_EXPECT(file.open(directory.path(), "sub/file", error));
    std::array<std::byte, 5> bytes{};
    DOBA_EXPECT_EQUAL(file.read(bytes), 5);
  }
  DOBA_EXPECT(monitor.unchanged(version, error));
  (void)monitor.observe("missing", error);
  DOBA_EXPECT(monitor.unchanged(version, error));
}

// +===========================================================================+
// | [>] monitor native resource ownership                       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("filesystem monitor releases native resources after reset and destruction") {
  file_directory directory;
  directory.write("file", "first");
  const auto resources = []() -> std::uint64_t {
#ifdef _WIN32
    DWORD count = 0;
    if (!GetProcessHandleCount(GetCurrentProcess(), &count))
      throw std::runtime_error("Unable to count process handles");
    return count;
#else
    return static_cast<std::uint64_t>(std::distance(
        fs::directory_iterator("/proc/self/fd"), fs::directory_iterator{}));
#endif
  };
  const auto before = resources();
  for (unsigned i = 0; i < 10; ++i) {
    filesystem_monitor monitor(directory.path());
    std::error_code error;
    DOBA_EXPECT(monitor.observe("file", error) != 0);
    DOBA_EXPECT(resources() > before);
    monitor.reset();
    DOBA_EXPECT_EQUAL(resources(), before);
    DOBA_EXPECT(monitor.observe("file", error) != 0);
  }
  DOBA_EXPECT_EQUAL(resources(), before);
}

// +===========================================================================+
// | [>] monitor reset races with readiness checks                ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("filesystem monitor keeps native handles alive during concurrent reset") {
  file_directory directory;
  directory.write("file", "first");
  filesystem_monitor monitor(directory.path());
  std::error_code error;
  const auto original = monitor.observe("file", error);
  DOBA_EXPECT(original != 0);
  std::atomic<bool> start{false};
  std::atomic<unsigned> calls{0};
  {
    std::vector<std::jthread> readers;
    for (unsigned i = 0; i < 8; ++i) readers.emplace_back([&] {
      while (!start.load()) std::this_thread::yield();
      for (unsigned n = 0; n < 500; ++n) {
        std::error_code local;
        (void)monitor.unchanged(original, local);
        ++calls;
      }
    });
    start = true;
    for (unsigned i = 0; i < 50; ++i) {
      monitor.reset();
      DOBA_EXPECT(monitor.observe("file", error) > original);
    }
  }
  DOBA_EXPECT_EQUAL(calls.load(), 4000);
  DOBA_EXPECT(!monitor.unchanged(original, error));
  const auto current = monitor.observe("file", error);
  DOBA_EXPECT(current > original);
  DOBA_EXPECT(monitor.unchanged(current, error));
}
