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

#ifndef martianlabs_doba_transport_server_tcp_windows_transport_h
#define martianlabs_doba_transport_server_tcp_windows_transport_h

#include <utility>

#include "transport/server/tcp_windows_worker.h"

namespace martianlabs::doba::transport::server {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] basic_transport [windowsTM]                                 ( class ) |
// +---------------------------------------------------------------------------+
// | This specification holds for the WindowsTM server transport.              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty,
          protocol::contracts::engine_factory<ENty> FAty, typename CNty,
          typename PTy>
class basic_transport {
 public:
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  using policies_type = PTy;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  explicit basic_transport(policies_type configuration, FAty create_engine,
                           typename CNty::shared_state shared_state)
      : worker_{std::move(configuration), std::move(create_engine),
                std::move(shared_state)} {}
  basic_transport(const basic_transport&) = delete;
  basic_transport(basic_transport&&) noexcept = delete;
  ~basic_transport() { stop(); }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  basic_transport& operator=(const basic_transport&) = delete;
  basic_transport& operator=(basic_transport&&) noexcept = delete;
  // +=========================================================================+
  // | [>] start                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  void start() { worker_.start(); }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  void stop() { worker_.stop(); }
  // +=========================================================================+
  // | [>] quiesce                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  void quiesce() { worker_.quiesce(); }
  // +=========================================================================+
  // | [>] set_on_connection                                        ( public ) |
  // +-------------------------------------------------------------------------+
  template <typename FNty>
  void set_on_connection(FNty&& fn) {
    worker_.set_on_connection(std::forward<FNty>(fn));
  }
  // +=========================================================================+
  // | [>] set_on_disconnection                                     ( public ) |
  // +-------------------------------------------------------------------------+
  template <typename FNty>
  void set_on_disconnection(FNty&& fn) {
    worker_.set_on_disconnection(std::forward<FNty>(fn));
  }

 private:
  // +=========================================================================+
  // | ATTRIBUTEs                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  worker<ENty, FAty, CNty, PTy> worker_;
};

}  // namespace martianlabs::doba::transport::server

#endif
