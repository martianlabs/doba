//      _       _
//   __| | ___ | |__   __ _
//  / _` |/ _ \| '_ \ / _` |
// | (_| | (_) | |_) | (_| |
//  \__,_|\___/|_.__/ \__,_|
//
// Copyright 2025 martianLabs
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef martianlabs_doba_protocol_http11_target_h
#define martianlabs_doba_protocol_http11_target_h

#include <string_view>

#include "target_type.h"

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// target                                                              ( class )
// -----------------------------------------------------------------------------
// This class holds for the http 1.1 request-target implementation.
// -----------------------------------------------------------------------------
// =============================================================================
class target {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  target(const target&) = default;
  target(target&&) noexcept = default;
  ~target() = default;
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  target& operator=(const target&) = default;
  target& operator=(target&&) noexcept = default;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  inline auto get_type() const { return type_.value(); }
  inline auto get_path() const { return path_; }
  inline auto get_host() const { return host_; }
  inline auto get_port() const { return port_; }
  inline auto get_asterisk() const { return constants::character::kAsterisk; }
  inline void clear() {
    type_.reset();
    path_.reset();
    host_.reset();
    port_.reset();
  }
  // ___________________________________________________________________________
  // STATIC-METHODs                                                   ( public )
  //
  inline static auto create_as_asterisk_form() { return target(); }
  inline static auto create_as_origin_form(const std::string_view& path) {
    return target(path);
  }
  inline static auto create_as_authority_form(const std::string_view& host,
                                              const std::string_view& port) {
    return target(host, port);
  }

 private:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  target() { type_ = target_type::kAsteriskForm; }
  target(const std::string_view& path) {
    type_ = target_type::kOriginForm;
    path_ = path;
  }
  target(const std::string_view& host, const std::string_view& port) {
    type_ = target_type::kAuthorityForm;
    host_ = host;
    port_ = port;
  }
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  std::optional<target_type> type_;
  std::optional<std::string> path_;
  std::optional<std::string> host_;
  std::optional<std::string> port_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
