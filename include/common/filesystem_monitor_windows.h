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

#ifndef martianlabs_doba_common_filesystem_monitor_windows_h
#define martianlabs_doba_common_filesystem_monitor_windows_h

#include <array>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "common/filesystem.h"

namespace martianlabs::doba::common::detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] filesystem_monitor_native [windows]                         ( class ) |
// +---------------------------------------------------------------------------+
// | Overlapped directory notifications, polled without callbacks or threads.  |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class filesystem_monitor_native {
 private:
  // +=========================================================================+
  // | [>] watch_record                                           ( private ) |
  // +-------------------------------------------------------------------------+
  struct watch_record {
    // Directory handle with one outstanding notification request.
    HANDLE directory = INVALID_HANDLE_VALUE;
    // Absolute directory associated with notification names.
    std::filesystem::path path;
    // Completion event; never associated with a transport completion port.
    HANDLE event = nullptr;
    // Stable storage for the pending operation.
    OVERLAPPED operation{};
    // DWORD-aligned notification buffer (16 KiB).
    std::array<DWORD, 4096> buffer{};
    // Ancestor child-name filter; empty means the whole watched subtree.
    std::wstring filter;
    // True for the configured root, false for its ancestors.
    bool subtree = false;
    // Whether cancellation must complete before releasing storage.
    bool pending = false;
    ~watch_record() {
      if (directory != INVALID_HANDLE_VALUE) {
        if (pending) {
          CancelIoEx(directory, &operation);
          DWORD ignored = 0;
          GetOverlappedResult(directory, &operation, &ignored, TRUE);
        }
        CloseHandle(directory);
      }
      if (event) CloseHandle(event);
    }
  };
 public:
  // +=========================================================================+
  // | [>] open                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  bool open(const std::filesystem::path& root, std::size_t limit,
            std::error_code& error) {
    close();
    error.clear();
    max_directories_ = limit;
    if (!limit || GetDriveTypeW(root.root_path().c_str()) == DRIVE_REMOTE) {
      error = std::make_error_code(std::errc::operation_not_supported);
      return false;
    }
    auto current = root.root_path();
    for (const auto& part : root.relative_path()) {
      if (!add(current, part.native(), false, limit, error)) return false;
      current /= part;
    }
    return add(root, {}, true, limit, error);
  }
  // +=========================================================================+
  // | [>] watch                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  bool watch(const std::filesystem::path& root, std::string_view relative,
             std::error_code& error) {
    auto current = root;
    const auto parts = std::filesystem::path(std::u8string(relative.begin(), relative.end()));
    for (auto part = parts.begin(); part != parts.end(); ++part) {
      if (std::next(part) == parts.end()) break;
      current /= *part;
      if (!directories_.contains(current.native())) {
        if (directories_.size() >= max_directories_) {
          error = std::make_error_code(std::errc::no_space_on_device); return false;
        }
        const auto attributes = directory_attributes(current, error);
        if (!attributes) return false;
        directories_.emplace(current.native(), *attributes);
      }
    }
    return true;
  }
  // +=========================================================================+
  // | [>] pending                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  // Manual-reset events remain signalled until the exclusive consumer rearms.
  int pending(std::error_code& error) const {
    error.clear();
    std::array<HANDLE, MAXIMUM_WAIT_OBJECTS> events{};
    for (std::size_t i = 0; i < records_.size(); ++i) events[i] = records_[i]->event;
    const auto result = WaitForMultipleObjects(static_cast<DWORD>(records_.size()),
                                               events.data(), FALSE, 0);
    if (result == WAIT_TIMEOUT) return 0;
    if (result < records_.size()) return 1;
    error = native_error();
    return 2;
  }
  // +=========================================================================+
  // | [>] poll                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  int poll(std::error_code& error) {
    error.clear();
    bool changed = false;
    std::array<HANDLE, MAXIMUM_WAIT_OBJECTS> events{};
    for (std::size_t i = 0; i < records_.size(); ++i) events[i] = records_[i]->event;
    for (unsigned batch = 0; batch < 64; ++batch) {
      const auto index = WaitForMultipleObjects(static_cast<DWORD>(records_.size()),
                                                events.data(), FALSE, 0);
      if (index == WAIT_TIMEOUT) return changed ? 1 : 0;
      if (index == WAIT_FAILED || index >= records_.size()) {
        error = native_error(); return 2;
      }
      auto& record = *records_[index];
      DWORD size = 0;
      if (!GetOverlappedResult(record.directory, &record.operation, &size, FALSE)) {
        error = native_error(); return 2;
      }
      record.pending = false;
      if (!size || size > sizeof(record.buffer)) return lost(error);
      std::size_t position = 0;
      while (position < size) {
        constexpr auto prefix = offsetof(FILE_NOTIFY_INFORMATION, FileName);
        if (size - position < prefix) return lost(error);
        const auto* item = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(
            reinterpret_cast<const char*>(record.buffer.data()) + position);
        if (item->FileNameLength % sizeof(wchar_t) || item->FileNameLength > size - position - prefix)
          return lost(error);
        const auto length = static_cast<int>(item->FileNameLength / sizeof(wchar_t));
        const bool relevant = record.filter.empty() ||
            CompareStringOrdinal(record.filter.data(), static_cast<int>(record.filter.size()),
                                 item->FileName, length, TRUE) == CSTR_EQUAL;
        if (relevant) {
          if (item->Action != FILE_ACTION_MODIFIED) return lost(error);
          const auto path = record.path / std::wstring(item->FileName, static_cast<std::size_t>(length));
          auto directory = directories_.find(path.native());
          if (directory == directories_.end()) {
            // Notifications preserve filesystem spelling, which may differ
            // from the spelling of a case-insensitive request path.
            directory = std::find_if(directories_.begin(), directories_.end(), [&](const auto& candidate) {
              return CompareStringOrdinal(candidate.first.c_str(), -1, path.c_str(), -1, TRUE) == CSTR_EQUAL;
            });
          }
          if (directory != directories_.end()) {
            const auto attributes = directory_attributes(path, error);
            if (!attributes) return 2;
            // Windows can report directory access-time changes as MODIFIED.
            // Ignore only a confirmed unchanged stable-attribute snapshot;
            // otherwise our own confined read opens could invalidate loads.
            if (*attributes != directory->second) {
              changed = true;
              directory->second = *attributes;
            }
          } else changed = true;
        }
        if (!item->NextEntryOffset) break;
        if (item->NextEntryOffset < prefix + item->FileNameLength ||
            item->NextEntryOffset % alignof(DWORD) || item->NextEntryOffset >= size - position)
          return lost(error);
        position += item->NextEntryOffset;
      }
      if (!arm(record, error)) return 2;
    }
    return lost(error);
  }
  // +=========================================================================+
  // | [>] close                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  void close() noexcept { records_.clear(); directories_.clear(); }
 private:
  // +=========================================================================+
  // | [>] native_error                                           ( private ) |
  // +-------------------------------------------------------------------------+
  static std::error_code native_error() {
    return {static_cast<int>(GetLastError()), std::system_category()};
  }
  // +=========================================================================+
  // | [>] lost                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  static int lost(std::error_code& error) {
    error = std::make_error_code(std::errc::state_not_recoverable); return 2;
  }
  // +=========================================================================+
  // | [>] directory_attributes                                   ( private ) |
  // +-------------------------------------------------------------------------+
  static std::optional<filesystem_metadata> directory_attributes(
      const std::filesystem::path& path, std::error_code& error) {
    const auto handle = CreateFileW(path.c_str(), FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
    if (handle == INVALID_HANDLE_VALUE) { error = native_error(); return {}; }
    BY_HANDLE_FILE_INFORMATION info{};
    FILE_BASIC_INFO basic{};
    const bool success = GetFileInformationByHandle(handle, &info) &&
        GetFileInformationByHandleEx(handle, FileBasicInfo, &basic, sizeof(basic));
    if (!success) error = native_error();
    CloseHandle(handle);
    if (!success) return {};
    if ((info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) ||
        !(info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
      error = std::make_error_code(std::errc::permission_denied); return {};
    }
    constexpr std::int64_t epoch = 116444736000000000LL;
    const auto modified = basic.LastWriteTime.QuadPart - epoch;
    const auto changed = basic.ChangeTime.QuadPart - epoch;
    return filesystem_metadata{0, modified / 10000000,
        static_cast<std::uint32_t>((modified % 10000000) * 100), info.dwVolumeSerialNumber,
        (static_cast<std::uint64_t>(info.nFileIndexHigh) << 32) | info.nFileIndexLow,
        changed / 10000000, static_cast<std::uint32_t>((changed % 10000000) * 100),
        info.dwFileAttributes, info.nNumberOfLinks};
  }
  // +=========================================================================+
  // | [>] arm                                                    ( private ) |
  // +-------------------------------------------------------------------------+
  static bool arm(watch_record& record, std::error_code& error) {
    ResetEvent(record.event);
    record.operation = {};
    record.operation.hEvent = record.event;
    constexpr DWORD changes = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
        FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE |
        FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SECURITY;
    if (!ReadDirectoryChangesW(record.directory, record.buffer.data(),
          static_cast<DWORD>(sizeof(record.buffer)), record.subtree, changes,
          nullptr, &record.operation, nullptr)) { error = native_error(); return false; }
    record.pending = true;
    return true;
  }
  // +=========================================================================+
  // | [>] add                                                    ( private ) |
  // +-------------------------------------------------------------------------+
  bool add(const std::filesystem::path& path, std::wstring filter, bool subtree,
           std::size_t limit, std::error_code& error) {
    if (records_.size() >= limit || records_.size() >= MAXIMUM_WAIT_OBJECTS) {
      error = std::make_error_code(std::errc::no_space_on_device); return false;
    }
    auto record = std::make_unique<watch_record>();
    record->path = path;
    record->filter = std::move(filter);
    record->subtree = subtree;
    record->directory = CreateFileW(path.c_str(), FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_OVERLAPPED, nullptr);
    if (record->directory == INVALID_HANDLE_VALUE) { error = native_error(); return false; }
    BY_HANDLE_FILE_INFORMATION info{};
    if (!GetFileInformationByHandle(record->directory, &info)) { error = native_error(); return false; }
    if ((info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) ||
        !(info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
      error = std::make_error_code(std::errc::permission_denied); return false;
    }
    record->event = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!record->event) { error = native_error(); return false; }
    if (!arm(*record, error)) return false;
    const auto attributes = directory_attributes(path, error);
    if (!attributes) return false;
    directories_.emplace(path.native(), *attributes);
    records_.push_back(std::move(record));
    return true;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  // Stable per-directory operations, released only after cancellation completes.
  std::vector<std::unique_ptr<watch_record>> records_;
  // Stable directory attributes, excluding access time; inspected only on events.
  std::unordered_map<std::wstring, filesystem_metadata> directories_;
  // Bound on lazily registered directory snapshots as well as native resources.
  std::size_t max_directories_ = 0;

};
}  // namespace martianlabs::doba::common::detail

#endif
