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

#ifndef martianlabs_doba_protocol_http_v11_session_h
#define martianlabs_doba_protocol_http_v11_session_h

#include <string>
#include <string_view>

#include "protocol/http/common/header_names.h"
#include "protocol/http/common/method_names.h"
#include "protocol/http/common/target.h"

namespace martianlabs::doba::protocol::http::v11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] session                                                     ( class ) |
// +---------------------------------------------------------------------------+
// | This class holds for the synchronous HTTP/1.1 session dispatch.          |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty, typename ROty>
class session {
 public:
  // +=========================================================================+
  // | [>] dispatch                                                 ( public ) |
  // +=========================================================================+
  void dispatch(const RQty& req, RSty& res, ROty& router) const {
    switch (req.get_target()) {
      case target::kOriginForm:
      case target::kAbsoluteForm: {
        // The request is either in origin-form (RFC 9110 S9.3.5) or
        // absolute-form (RFC 9110 S9.3.4); in either case, the request is
        // routed to a handler based on the method and absolute path.
        std::string_view method = req.get_method();
        std::string_view abs_path = req.get_absolute_path();
        auto match = router.match(method, abs_path);
        if (match.handler) {
          match.handler->callback(req, res);
        } else if (match.parametrized_handler) {
          match.parametrized_handler->invoke(req, res, abs_path);
        } else {
          std::string allowed_methods = router.allowed_methods(abs_path);
          if (allowed_methods.empty()) {
            res.not_found_404();
          } else {
            res.method_not_allowed_405();
            res.set_header(header_names::kAllow, allowed_methods);
          }
        }
        break;
      }
      case target::kAuthorityForm:
        // CONNECT tunnelling (RFC 9110 S9.3.6) is deliberately deferred to
        // Doba's future client (dial-out) module. Until that module exists,
        // the request must not be left unanswered.
        res.not_implemented_501();
        break;
      case target::kAsteriskForm:
        // OPTIONS * (RFC 9110 S9.3.7) addresses the server in general rather
        // than a specific resource; acknowledge it without routing.
        res.ok_200();
        break;
      default:
        res.bad_request_400();
        break;
    }
    if (req.wants_connection_close()) {
      res.set_header(header_names::kConnection, "close");
    }
    if (req.get_method() == method_names::kHead) {
      // RFC 9110 S9.3.2: HEAD preserves GET framing headers without a body.
      bool had_cl = res.has_header(header_names::kContentLength);
      std::string cl =
          had_cl ? res.get_header(header_names::kContentLength).second
                 : std::string();
      bool had_te = res.has_header(header_names::kTransferEncoding);
      std::string te =
          had_te ? res.get_header(header_names::kTransferEncoding).second
                 : std::string();
      res.clear_body();
      if (had_cl) res.set_header(header_names::kContentLength, cl);
      if (had_te) res.set_header(header_names::kTransferEncoding, te);
    }
  }
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
