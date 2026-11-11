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

#ifndef martianlabs_doba_tests_unit_engine_probe_h
#define martianlabs_doba_tests_unit_engine_probe_h

#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "protocol/http/v11/engine.h"
#include "protocol/http/v11/request.h"
#include "protocol/http/v11/response.h"

namespace martianlabs::doba::tests::unit {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] engine_probe                                               ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct engine_probe {
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using request = protocol::http::v11::request;
  using response = protocol::http::v11::response;
  using policies = protocol::http::v11::policies;
  using router = protocol::http::router<request, response>;
  using clock = protocol::processing_result::clock;

  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  explicit engine_probe(const router& routes, policies limits = {})
      : value(limits, routes) {
    value.set_on_send(
        [this](auto head, auto body, std::unique_ptr<common::reader> source) {
          outputs.emplace_back(std::string(head) + std::string(body));
          wire.append(head).append(body);
          if (source) {
            ++sources;
            if (hold_source) {
              held = std::move(source);
            } else {
              source->read_all(wire);
            }
          }
        });
    value.set_on_close([this] { ++closes; });
  }
  // +=========================================================================+
  // | [>] receive                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  void receive(std::string_view input) {
    needs = value.on_bytes_received(input);
  }
  // +=========================================================================+
  // | [>] turn                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  void turn(bool output_idle = true) {
    needs = value.on_processing_turn(output_idle);
  }
  // +=========================================================================+
  // | [>] drain                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  void drain() {
    if (held) {
      held->read_all(wire);
      held.reset();
    }
    turn(true);
  }
  // +=========================================================================+
  // | [>] get                                                      ( public ) |
  // +-------------------------------------------------------------------------+
  static std::string get(std::string_view path) {
    return "GET " + std::string(path) + " HTTP/1.1\r\nHost: local\r\n\r\n";
  }

  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  protocol::processing_result needs;
  std::string wire;
  std::vector<std::string> outputs;
  std::unique_ptr<common::reader> held;
  bool hold_source{false};
  int sources{0};
  int closes{0};
  protocol::http::v11::engine<request, response> value;
};
}  // namespace martianlabs::doba::tests::unit

#endif
