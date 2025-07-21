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

#ifndef martianlabs_doba_http11_server_h
#define martianlabs_doba_http11_server_h

#include <ranges>
#include <string_view>

#include "server.h"
#include "transport/server/tcpip.h"
#include "protocol/http11/constants.h"
#include "protocol/http11/helpers.h"
#include "protocol/http11/request.h"
#include "protocol/http11/response.h"

namespace martianlabs::doba {
// =============================================================================
// http11_server                                                       ( class )
// -----------------------------------------------------------------------------
// This class holds for the http 1.1 server implementation.
// -----------------------------------------------------------------------------
// =============================================================================
class http11_server
    : public server<transport::server::tcpip, protocol::http11::request,
                    protocol::http11::response> {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  http11_server(const char port[]) {
    set_port(port);
    set_buffer_size(kDefaultBufferSize);
    set_number_of_workers(kDefaultNumberOfWorkers);
    set_on_connection([](socket_type id) {});
    set_on_disconnection([](socket_type id) {});
    set_on_bytes_received([](socket_type id, unsigned long bytes) {});
    set_on_bytes_sent([](socket_type id, unsigned long bytes) {});
    set_on_request_ok(
        [this](const protocol::http11::request& req,
               protocol::http11::response& res) -> transport::process_result {
          if (!process_headers(req, res)) {
            return transport::process_result::kError;
          }
          return transport::process_result::kCompleted;
        });
    set_on_request_error([this](protocol::http11::response& res) {
      // here we have to generate a BAD REQUEST response!
    });
    setup_headers_functions();
  }
  http11_server(const http11_server&) = delete;
  http11_server(http11_server&&) noexcept = delete;
  ~http11_server() = default;
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  http11_server& operator=(const http11_server&) = delete;
  http11_server& operator=(http11_server&&) noexcept = delete;

 private:
  // ___________________________________________________________________________
  // USINGs                                                          ( private )
  //
  using on_header_fn =
      std::function<bool(const std::string_view&, protocol::http11::response&)>;
  // ___________________________________________________________________________
  // CONSTANTs                                                       ( private )
  //
  static constexpr uint32_t kDefaultBufferSize = 2048;
  static constexpr uint8_t kDefaultNumberOfWorkers = 8;
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  void setup_headers_functions() {
    static std::string_view kConnection =
        (const char*)protocol::http11::constants::header::kConnection;
    // +---------------------+------------------------+
    // | Field               | Definition             |
    // +---------------------+------------------------+
    // | Connection          | 1#connection-option    |
    // | connection-option   | token                  |
    // +---------------------+------------------------+
    headers_fns_[kConnection] = [this](
                                    const std::string_view& v,
                                    protocol::http11::response& res) -> bool {
      std::vector<std::string_view> values;
      for (auto token :
           v | std::views::split(
                   protocol::http11::constants::character::kComma)) {
        std::string_view value(&*token.begin(), std::ranges::distance(token));
        value = protocol::http11::helpers::ows_ltrim(
            protocol::http11::helpers::ows_rtrim(value));
        if (!protocol::http11::helpers::is_token(value)) return false;
        values.emplace_back(value);
      }
      if (!values.size()) return false;
      for (auto& value : values) res.set_header(kConnection, value);
      return true;
    };
  }
  bool process_headers(const protocol::http11::request& req,
                       protocol::http11::response& res) const {
    for (auto const& hdr : req.get_headers()) {
      auto itr = headers_fns_.find(hdr.first);
      if (itr != headers_fns_.end())
        if (!itr->second(hdr.second, res)) return false;
    }
    return true;
  }
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  std::unordered_map<std::string_view, on_header_fn> headers_fns_;
};
}  // namespace martianlabs::doba

#endif
