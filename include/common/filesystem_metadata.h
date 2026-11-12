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

#ifndef martianlabs_doba_common_filesystem_metadata_h
#define martianlabs_doba_common_filesystem_metadata_h

#include <cstdint>

namespace martianlabs::doba::common {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] filesystem_metadata                                        ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct filesystem_metadata {
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  std::uintmax_t size = 0;
  std::int64_t modified_seconds = 0;
  std::uint32_t modified_nanoseconds = 0;
  std::uint64_t device = 0;
  std::uint64_t identity = 0;
  std::int64_t changed_seconds = 0;
  std::uint32_t changed_nanoseconds = 0;
  // Windows flags, or Linux file type and permissions.
  std::uint32_t attributes = 0;
  // Hard link count.
  std::uint64_t links = 0;
  bool operator==(const filesystem_metadata&) const = default;
};
}  // namespace martianlabs::doba::common

#endif
