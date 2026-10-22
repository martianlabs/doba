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

#ifndef martianlabs_doba_common_filesystem_monitor_linux_h
#define martianlabs_doba_common_filesystem_monitor_linux_h

#include <array>
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <sys/inotify.h>
#include <poll.h>
#include <sys/vfs.h>

#include "common/filesystem.h"

namespace martianlabs::doba::common::detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] filesystem_monitor_native [linux]                           ( class ) |
// +---------------------------------------------------------------------------+
// | Nonblocking inotify backend. No thread or user callback is created.        |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class filesystem_monitor_native {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  filesystem_monitor_native() = default;
  filesystem_monitor_native(const filesystem_monitor_native&) = delete;
  filesystem_monitor_native& operator=(const filesystem_monitor_native&) = delete;
  ~filesystem_monitor_native() { close(); }
  // +=========================================================================+
  // | [>] open                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  bool open(const std::filesystem::path& root, std::size_t limit,
            std::error_code& error) {
    close();
    error.clear();
    limit_ = limit;
    if (!limit) { error = std::make_error_code(std::errc::operation_not_supported); return false; }
    if (!supported(root, error)) return false;
    descriptor_ = ::inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    if (descriptor_ == -1) { error = native_error(); return false; }
    auto current = root.root_path();
    for (const auto& part : root.relative_path()) {
      if (!add(current, part.native(), true, error)) return false;
      current /= part;
    }
    return add(root, {}, true, error);
  }
  // +=========================================================================+
  // | [>] watch                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  bool watch(const std::filesystem::path& root, std::string_view relative,
             std::error_code& error) {
    const auto target = root / std::filesystem::path(relative);
    if (!supported(target, error)) return false;
    auto current = root;
    const auto parts = std::filesystem::path(relative);
    for (auto part = parts.begin(); part != parts.end(); ++part) {
      current /= *part;
      const bool directory = std::next(part) != parts.end();
      if (!add(current, {}, directory, error)) return false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] pending                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  // Readiness query does not dequeue events and supports concurrent callers.
  int pending(std::error_code& error) const {
    error.clear();
    struct pollfd descriptor{descriptor_, POLLIN, 0};
    const auto count = ::poll(&descriptor, 1, 0);
    if (!count) return 0;
    if (count < 0 && errno == EINTR) return 1;
    if (count < 0) { error = native_error(); return 2; }
    if (descriptor.revents & (POLLERR | POLLHUP | POLLNVAL)) return lost(error);
    return descriptor.revents & POLLIN ? 1 : 0;
  }
  // +=========================================================================+
  // | [>] poll                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // 0: unchanged, 1: changed, 2: coverage lost (rebuild required).
  int poll(std::error_code& error) {
    error.clear();
    bool changed = false;
    alignas(inotify_event) std::array<char, 65536> buffer;
    for (unsigned batch = 0; batch < 16; ++batch) {
      const auto size = ::read(descriptor_, buffer.data(), buffer.size());
      if (size < 0) {
        if (errno == EINTR) continue;
        if (errno == EAGAIN) return changed ? 1 : 0;
        error = native_error();
        return 2;
      }
      if (!size) { error = std::make_error_code(std::errc::io_error); return 2; }
      std::size_t position = 0;
      while (position < static_cast<std::size_t>(size)) {
        if (static_cast<std::size_t>(size) - position < sizeof(inotify_event))
          return lost(error);
        inotify_event event{};
        std::memcpy(&event, buffer.data() + position, sizeof(event));
        const auto remaining = static_cast<std::size_t>(size) - position - sizeof(event);
        if (event.len > remaining) return lost(error);
        if (event.mask & (IN_Q_OVERFLOW | IN_IGNORED | IN_UNMOUNT)) return lost(error);
        auto found = records_.find(event.wd);
        if (found == records_.end()) return lost(error);
        const char* name = buffer.data() + position + sizeof(event);
        const auto length = event.len ? ::strnlen(name, event.len) : 0;
        const auto& filter = found->second;
        const bool relevant = !event.len || filter.empty() || filter == std::string_view(name, length);
        if (relevant) {
          changed = true;
          if (event.mask & (IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO |
                            IN_DELETE_SELF | IN_MOVE_SELF)) return lost(error);
        }
        position += sizeof(event) + event.len;
      }
    }
    // A continuous event stream must not monopolize a request worker.
    return lost(error);
  }
  // +=========================================================================+
  // | [>] close                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  void close() noexcept {
    if (descriptor_ != -1) ::close(descriptor_);
    descriptor_ = -1;
    records_.clear();
    paths_.clear();
  }

 private:
  // +=========================================================================+
  // | [>] native_error                                           ( private ) |
  // +-------------------------------------------------------------------------+
  static std::error_code native_error() { return {errno, std::generic_category()}; }
  // +=========================================================================+
  // | [>] lost                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  static int lost(std::error_code& error) {
    error = std::make_error_code(std::errc::state_not_recoverable);
    return 2;
  }
  // +=========================================================================+
  // | [>] supported                                              ( private ) |
  // +-------------------------------------------------------------------------+
  static bool supported(const std::filesystem::path& path, std::error_code& error) {
    struct statfs info{};
    if (::statfs(path.c_str(), &info) != 0) { error = native_error(); return false; }
    // Local ext*, XFS, Btrfs, tmpfs and overlayfs. Remote/virtual filesystems
    // cannot be assumed to deliver all changes made outside this process.
    switch (static_cast<unsigned long>(info.f_type)) {
      case 0xef53UL: case 0x58465342UL: case 0x9123683eUL:
      case 0x01021994UL: case 0x794c7630UL: return true;
      default: error = std::make_error_code(std::errc::operation_not_supported); return false;
    }
  }
  // +=========================================================================+
  // | [>] add                                                    ( private ) |
  // +-------------------------------------------------------------------------+
  bool add(const std::filesystem::path& path, std::string filter, bool directory,
           std::error_code& error) {
    if (paths_.contains(path.native())) return true;
    if (paths_.size() >= limit_ || records_.size() >= limit_) {
      error = std::make_error_code(std::errc::no_space_on_device);
      return false;
    }
    constexpr std::uint32_t changes = IN_MODIFY | IN_ATTRIB | IN_CLOSE_WRITE |
        IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO | IN_DELETE_SELF | IN_MOVE_SELF;
    const int id = ::inotify_add_watch(descriptor_, path.c_str(), changes |
        IN_DONT_FOLLOW | (directory ? IN_ONLYDIR : 0));
    if (id == -1) { error = native_error(); return false; }
    auto [record, added] = records_.try_emplace(id, std::move(filter));
    // Multiple hard-link names can share a watch descriptor. Empty means all.
    if (!added) record->second.clear();
    paths_.insert(path.native());
    return true;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  // Nonblocking descriptor; -1 means closed.
  int descriptor_ = -1;
  // Maximum native watches owned by this instance.
  std::size_t limit_ = 0;
  // Watch descriptors and optional child-name filters for ancestors.
  std::unordered_map<int, std::string> records_;
  // Registered paths, avoiding syscalls when a retained path is observed again.
  std::unordered_set<std::string> paths_;
};
}  // namespace martianlabs::doba::common::detail

#endif
