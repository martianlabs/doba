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

#ifndef martianlabs_doba_protocol_http11_server_h
#define martianlabs_doba_protocol_http11_server_h

#include <ranges>
#include <string_view>

#include "server_base.h"
#include "transport/server/tcpip.h"
#include "protocol/http11/constants.h"
#include "protocol/http11/helpers.h"
#include "protocol/http11/request.h"
#include "protocol/http11/response.h"
#include "protocol/http11/router.h"
#include "protocol/http11/headers.h"

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// server                                                              ( class )
// -----------------------------------------------------------------------------
// This class holds for the http 1.1 server implementation.
// -----------------------------------------------------------------------------
// Template parameters:
//    RQty - request being used.
//    RSty - response being used.
//    ROty - router being used.
//    TRty - transport being used.
// =============================================================================
template <typename RQty = request, typename RSty = response,
          template <typename, typename> class ROty = router,
          template <typename, typename> class TRty = transport::server::tcpip>
class server : public server_base<TRty, RQty, RSty> {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  server(const char port[]) {
    TRty<RQty, RSty>::set_port(port);
    TRty<RQty, RSty>::set_number_of_workers(kDefaultNumberOfWorkers);
    TRty<RQty, RSty>::set_on_connection([](socket_type id) {});
    TRty<RQty, RSty>::set_on_disconnection([](socket_type id) {});
    TRty<RQty, RSty>::set_on_bytes_received(
        [](socket_type id, unsigned long bytes) {});
    TRty<RQty, RSty>::set_on_bytes_sent(
        [](socket_type id, unsigned long bytes) {});
    TRty<RQty, RSty>::set_on_request_ok(
        [this](const RQty& req, RSty& res) -> transport::process_result {
          auto process_headers_result = process_headers(req, res);
          if (process_headers_result != transport::process_result::kCompleted) {
            return process_headers_result;
          }

          /*
          pepe
          */

          res.ok_200()
              .add_header("Content-Type", "text/plain")
              .add_header("Connection", "keep-alive")
              .add_header("Content-Length", "13")
              .add_body("Hello, World!");

          /*
          auto fn = router_.match(req.get_method(), req.get_path());
          if (fn.has_value()) {
            res.ok_200()
                .add_header("Content-Type", "text/plain")
                .add_header("Connection", "keep-alive")
                .add_header("Content-Length", "13")
                .add_body("Hello, World!");
          } else {
            res.not_found_404().add_header(headers::kContentLength, 0);
          }
          */

          /*
          pepe fin
          */

          return transport::process_result::kCompleted;
        });
    TRty<RQty, RSty>::set_on_request_error([this](RSty& res) {
      // here we have to generate a BAD REQUEST response!
    });
    setup_headers_functions();
  }
  server(const server&) = delete;
  server(server&&) noexcept = delete;
  ~server() = default;
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  server& operator=(const server&) = delete;
  server& operator=(server&&) noexcept = delete;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  server& add_route(method method, const std::string_view& route,
                    ROty<RQty, RSty>::handler_fn fn) {
    router_.add(method, route, fn);
    return *this;
  }

 private:
  // ___________________________________________________________________________
  // USINGs                                                          ( private )
  //
  using on_header_fn =
      std::function<transport::process_result(const std::string_view&, RSty&)>;
  // ___________________________________________________________________________
  // CONSTANTs                                                       ( private )
  //
  static constexpr uint8_t kDefaultNumberOfWorkers = 8;
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  void setup_headers_functions() {
    static std::string_view kConnection = headers::kConnection;
    // +---------------------+------------------------+
    // | Field               | Definition             |
    // +---------------------+------------------------+
    // | Connection          | 1#connection-option    |
    // | connection-option   | token                  |
    // +---------------------+------------------------+
    headers_fns_[kConnection] = [this](const std::string_view& v,
                                       RSty& res) -> transport::process_result {
      for (auto token : v | std::views::split(constants::character::kComma)) {
        std::string_view value(&*token.begin(), std::ranges::distance(token));
        value = helpers::ows_ltrim(helpers::ows_rtrim(value));
        if (!helpers::is_token(value)) {
          return transport::process_result::kError;
        }
      }
      return transport::process_result::kCompleted;
    };
  }
  transport::process_result process_headers(const RQty& req, RSty& res) const {

    /*
    pepe
    */

    /*
    for (auto const& hdr : req.get_headers()) {
      auto itr = headers_fns_.find(hdr.first);
      if (itr != headers_fns_.end()) {
        if (auto result = itr->second(hdr.second, res);
            result != transport::process_result::kCompleted) {
          return result;
        }
      }
    }
    */

    /*
    pepe fin
    */

    return transport::process_result::kCompleted;
  }
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  std::unordered_map<std::string_view, on_header_fn> headers_fns_;
  ROty<RQty, RSty> router_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
