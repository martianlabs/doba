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

#include <variant>
#include <future>

#include "common/execution_policy.h"
#include "transport/server/tcpip.h"
#include "protocol/http11/constants.h"
#include "protocol/http11/helpers.h"
#include "protocol/http11/request.h"
#include "protocol/http11/response.h"
#include "protocol/http11/router.h"
#include "protocol/http11/headers.h"
#include "protocol/http11/decoder.h"
#include "protocol/http11/checkers/host.h"
#include "protocol/http11/checkers/connection.h"

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// server                                                              ( class )
// -----------------------------------------------------------------------------
// This class holds for the http 1.1 server implementation.
// -----------------------------------------------------------------------------
// Template parameters:
//    TRty - transport being used (tcp/ip by default).
// =============================================================================
template <template <typename, typename, typename> class TRty =
              transport::server::tcpip>
class server {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  server(const char port[]) {
    transport_.set_port(port);
    transport_.set_number_of_workers(kDefaultNumberOfWorkers);
    transport_.set_on_client_connection([](socket_type id) {});
    transport_.set_on_client_disconnection([](socket_type id) {});
    transport_.set_on_bytes_received(
        [](socket_type id, unsigned long bytes) {});
    transport_.set_on_bytes_sent([](socket_type id, unsigned long bytes) {});
    transport_.set_on_client_request(
        [this](std::shared_ptr<const request> req)
            -> TRty<request, response,
                    router>::on_client_request_result_prototype {
          using req_in = std::shared_ptr<const request>;
          using res_in = std::shared_ptr<response>;
          auto fn_400 = [this](req_in req, res_in res) {
            res->bad_request_400().add_header(headers::kContentLength, 0);
          };
          auto fn_404 = [this](req_in req, res_in res) {
            res->not_found_404().add_header(headers::kContentLength, 0);
          };
          std::shared_ptr<response> res = std::make_shared<response>();
          // let's check if the incoming request is following the standard..
          if (process_headers(req)) {
            switch (req->get_target()) {
              case target::kOriginForm:
              case target::kAbsoluteForm:
                if (auto handler = router_.match(req->get_method(),
                                                 req->get_absolute_path())) {
                  return {handler->first, res, handler->second};
                } else {
                  return {fn_404, res, common::execution_policy::kSync};
                }
                break;
              case target::kAuthorityForm:
              case target::kAsteriskForm:
                // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
                // [to-do] -> add support for this!
                // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                break;
              default:
                break;
            }
          }
          return {fn_400, res, common::execution_policy::kSync};
        });
    transport_.set_on_error([this]() -> auto {
      std::shared_ptr<response> res = std::make_shared<response>();
      res->bad_request_400().add_header(headers::kContentLength, 0);
      return res;
    });
    setup_headers_functions();
  }
  server(const server&) = delete;
  server(server&&) noexcept = delete;
  ~server() { stop(); }
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  server& operator=(const server&) = delete;
  server& operator=(server&&) noexcept = delete;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  void start() { transport_.start(); }
  void stop() { transport_.stop(); }
  server& add_route(
      method method, std::string_view route, router::handler handler,
      common::execution_policy policy = common::execution_policy::kSync) {
    router_.add(method, route, handler, policy);
    return *this;
  }

 private:
  // ___________________________________________________________________________
  // USINGs                                                          ( private )
  //
  using on_header_check_delegate = std::function<bool(std::string_view)>;
  // ___________________________________________________________________________
  // CONSTANTs                                                       ( private )
  //
  static constexpr uint8_t kDefaultNumberOfWorkers = 8;
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  void setup_headers_functions() {
    // +-----------------------------------------------------------------------+
    // | ESSENTIAL HEADERS (MANDATORY)                                         |
    // +-----------------------------------------------------------------------+
    // | [x] Host                                                              |
    // | [x] Content-Length                                                (*) |
    // | [x] Connection                                                        |
    // | [ ] Date                                                              |
    // | [ ] Transfer-Encoding                                                 |
    // | [ ] TE                                                                |
    // +-----------------------------------------------------------------------+
    // | STRONGLY RECOMMENDED HEADERS                                          |
    // +-----------------------------------------------------------------------+
    // | [ ] Content-Type                                                      |
    // | [ ] Accept                                                            |
    // | [ ] Allow                                                             |
    // | [ ] Server                                                            |
    // | [ ] User-Agent                                                        |
    // | [ ] Expect                                                            |
    // | [ ] Upgrade                                                           |
    // +-----------------------------------------------------------------------+
    // | OPTIONAL / ADVANCED HEADERS                                           |
    // +-----------------------------------------------------------------------+
    // | [ ] Range                                                             |
    // | [ ] If-Modified-Since                                                 |
    // | [ ] Cache-Control                                                     |
    // | [ ] ETag                                                              |
    // | [ ] Location                                                          |
    // | [ ] Access-Control-*                                                  |
    // | [ ] Trailer                                                           |
    // | [ ] Vary                                                              |
    // +-----------------------------------------------------------------------+
    // +                                           (*) implemented by decoder. |
    // +-----------------------------------------------------------------------+
    headers_fns_[headers::kConnection] = checkers::connection_check_fn;
    headers_fns_[headers::kHost] = checkers::host_check_fn;
  }
  bool process_headers(std::shared_ptr<const request> req) const {
    if (!req) return false;
    for (auto const& hdr : req->get_headers()) {
      if (auto itr = headers_fns_.find(hdr.first); itr != headers_fns_.end()) {
        if (!itr->second(hdr.second)) return false;
      }
    }
    return true;
  }
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  std::unordered_map<std::string_view, on_header_check_delegate> headers_fns_;
  TRty<request, response, decoder> transport_;
  router router_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
