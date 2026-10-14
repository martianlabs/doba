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

#include <cstddef>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

#include "common/signaler.h"
#include "http_server.h"
#include "protocol/http/v11/contracts.h"

namespace {
using namespace martianlabs::doba::protocol::http;
using namespace martianlabs::doba::protocol::http::v11;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] custom_request                                              ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class custom_request : public request {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  custom_request(const custom_request&) = delete;
  custom_request(custom_request&&) noexcept = default;
  ~custom_request() = default;
  // +=========================================================================+
  // | [>] from                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  static custom_request from(
      std::string_view method, std::string_view abs_path, target target_form,
      std::vector<header_view> headers,
      std::vector<query_parameter_view> query_parameters,
      std::optional<std::string_view> host,
      std::optional<std::string_view> port,
      std::optional<helpers::host_type> type,
      std::optional<std::string_view> target_authority_host,
      std::optional<std::string_view> target_authority_port,
      std::optional<helpers::host_type> target_authority_type,
      bool wants_connection_close, std::optional<body::reader> body_reader) {
    return custom_request(request::from(
        method, abs_path, target_form, std::move(headers),
        std::move(query_parameters), host, port, type, target_authority_host,
        target_authority_port, target_authority_type, wants_connection_close,
        std::move(body_reader)));
  }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  custom_request& operator=(const custom_request&) = delete;
  custom_request& operator=(custom_request&&) noexcept = default;
  // +=========================================================================+
  // | [>] has_client_tag                                           ( public ) |
  // +-------------------------------------------------------------------------+
  bool has_client_tag() const noexcept { return tagged_; }

 private:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                ( private ) |
  // +-------------------------------------------------------------------------+
  explicit custom_request(request&& value)
      : request(std::move(value)), tagged_(exist_header("X-Client-Tag")) {}
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  bool tagged_;
};

static_assert(contracts::request<custom_request>);
}  // namespace

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main() {
  martianlabs::doba::http_server<custom_request> server(
      {.ip = "0.0.0.0", .port = "8080"});
  server.add_route("GET", "/hello",
                   [](const custom_request& req, response& res) {
                     res.ok_200().set_body(req.has_client_tag() ? "tagged"
                                                               : "untagged");
                   });
  server.start();
  martianlabs::doba::common::signaler::wait();
  server.stop();
  return 0;
}
