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
//        --- martianLabs Anti-AI Usage and Model-Training Addendum ---
//
// TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION
//
// Copyright 2025 martianLabs
//
// Except as otherwise stated in this Addendum, this software is licensed
// under the Apache License, Version 2.0 (the "License"); you may not use
// this file except in compliance with the License.
//
// The following additional terms are hereby added to the Apache License for
// the purpose of restricting the use of this software by Artificial
// Intelligence systems, machine learning models, data-scraping bots, and
// automated systems.
//
// 1.  MACHINE LEARNING AND AI RESTRICTIONS
//     1.1. No entity, organization, or individual may use this software,
//          its source code, object code, or any derivative work for the
//          purpose of training, fine-tuning, evaluating, or improving any
//          machine learning model, artificial intelligence system, large
//          language model, or similar automated system.
//     1.2. No automated system may copy, parse, analyze, index, or
//          otherwise process this software for any AI-related purpose.
//     1.3. Use of this software as input, prompt material, reference
//          material, or evaluation data for AI systems is expressly
//          prohibited.
//
// 2.  SCRAPING AND AUTOMATED ACCESS RESTRICTIONS
//     2.1. No automated crawler, training pipeline, or data-extraction
//          system may collect, store, or incorporate any portion of this
//          software in any dataset used for machine learning or AI
//          training.
//     2.2. Any automated access must comply with this License and with
//          applicable copyright law.
//
// 3.  PROHIBITION ON DERIVATIVE DATASETS
//     3.1. You may not create datasets, corpora, embeddings, vector
//          stores, or similar derivative data intended for use by
//          automated systems, AI models, or machine learning algorithms.
//
// 4.  NO WAIVER OF RIGHTS
//     4.1. These restrictions apply in addition to, and do not limit,
//          the rights and protections provided to the copyright holder
//          under the Apache License Version 2.0 and applicable law.
//
// 5.  ACCEPTANCE
//     5.1. Any use of this software constitutes acceptance of both the
//          Apache License Version 2.0 and this Anti-AI Addendum.
//
// You may obtain a copy of the Apache License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.  See the License for the specific language governing
// permissions and limitations under the Apache License Version 2.0.

#ifndef martianlabs_doba_protocol_http11_server_h
#define martianlabs_doba_protocol_http11_server_h

#include "common/execution_policy.h"
#include "common/thread_pool.h"
#include "transport/server/tcpip.h"
#include "protocol/http11/constants.h"
#include "protocol/http11/helpers.h"
#include "protocol/http11/request.h"
#include "protocol/http11/response.h"
#include "protocol/http11/router.h"
#include "protocol/http11/headers.h"
#include "protocol/http11/checkers/host.h"
#include "protocol/http11/checkers/date.h"
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
template <template <typename, typename, int> class TRty =
              transport::server::tcpip>
class server {
  using router_in = std::shared_ptr<const request>;
  using router_out = std::shared_ptr<response>;
  using router_fn = std::function<router_out(router_in)>;

 public:
  // ---------------------------------------------------------------------------
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  server() {
    transport_.set_on_request(
        [this](std::shared_ptr<const request> req, auto sender) -> void {
          // let's check if the incoming request is following the standard..
          if (!process_headers(req)) {
            sender(response::bad_request_400()->add_header(
                headers::kContentLength, 0));
          }
          switch (req->get_target()) {
            case target::kOriginForm:
            case target::kAbsoluteForm: {
              auto router_entry =
                  router_.match(req->get_method(), req->get_absolute_path());
              if (!router_entry.has_value()) {
                sender(response::not_found_404()->add_header(
                    headers::kContentLength, 0));
                return;
              }
              switch (router_entry->second) {
                case common::execution_policy::kSync:
                  sender(router_entry->first(req));
                  break;
                case common::execution_policy::kAsync:
                  thread_pool_->enqueue([req, sender, router_entry]() {
                    sender(router_entry->first(req));
                  });
                  break;
              }
              break;
            }
            case target::kAuthorityForm:
            case target::kAsteriskForm:
              // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
              // [to-do] -> add support for this!
              // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
              break;
            default:
              break;
          }

          /*
          pepe
          */

          /*
           */

          /*
          pepe fin
          */
        });
    transport_.set_on_connection([](uint32_t id) {
      // nothing to do here by default..
    });
    transport_.set_on_disconnection([](uint32_t id) {
      // nothing to do here by default..
    });
    setup_headers_functions();
  }
  server(const server&) = delete;
  server(server&&) noexcept = delete;
  ~server() { stop(); }
  // ---------------------------------------------------------------------------
  // OPERATORs                                                        ( public )
  //
  server& operator=(const server&) = delete;
  server& operator=(server&&) noexcept = delete;
  // ---------------------------------------------------------------------------
  // METHODs                                                          ( public )
  //
  void start(const char port[]) {
    common::date_server::get()->start();
    thread_pool_ = std::make_shared<common::thread_pool>(
        std::thread::hardware_concurrency() / 2);
    transport_.start(port);
  }
  void stop() {
    thread_pool_->stop();
    transport_.stop();
  }
  server& add_route(
      method method, std::string_view route, router_fn fn,
      common::execution_policy policy = common::execution_policy::kSync) {
    router_.add(method, route, fn, policy);
    return *this;
  }

 private:
  // ---------------------------------------------------------------------------
  // USINGs                                                          ( private )
  //
  using on_header_check_delegate = std::function<bool(std::string_view)>;
  // ---------------------------------------------------------------------------
  // METHODs                                                         ( private )
  //
  void setup_headers_functions() {
    // +-----------------------------------------------------------------------+
    // | ESSENTIAL HEADERS (MANDATORY)                                         |
    // +-----------------------------------------------------------------------+
    // | [x] Host                                                              |
    // | [x] Content-Length                                                (*) |
    // | [x] Connection                                                        |
    // | [x] Date                                                              |
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
    headers_fns_[headers::kHost] = checkers::host_check_fn;
    headers_fns_[headers::kConnection] = checkers::connection_check_fn;
    headers_fns_[headers::kDate] = checkers::date_check_fn;
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
  // ---------------------------------------------------------------------------
  // ATTRIBUTEs                                                      ( private )
  //
  std::unordered_map<std::string_view, on_header_check_delegate> headers_fns_;
  std::shared_ptr<common::thread_pool> thread_pool_;
  TRty<request, response, 4096> transport_;
  router<router_fn> router_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
