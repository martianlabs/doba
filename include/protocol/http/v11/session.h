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

#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "protocol/http/common/header_names.h"
#include "protocol/http/common/method_names.h"
#include "protocol/http/common/target.h"

namespace martianlabs::doba::protocol::http::v11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] session                                                     ( class ) |
// +---------------------------------------------------------------------------+
// | This class holds for the HTTP/1.1 session dispatch.                      |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty, typename ROty>
class session {
 public:
  // +=========================================================================+
  // | [>] dispatch                                                 ( public ) |
  // +=========================================================================+
  template <typename Dty, typename EXty>
  void dispatch(const std::shared_ptr<RQty>& req, RSty& res, ROty& router,
                Dty& deferred, EXty& executor) const {
    switch (req->get_target()) {
      case target::kOriginForm:
      case target::kAbsoluteForm: {
        // The request is either in origin-form (RFC 9110 S9.3.5) or
        // absolute-form (RFC 9110 S9.3.4); in either case, the request is
        // routed to a handler based on the method and absolute path.
        std::string_view method = req->get_method();
        std::string_view abs_path = req->get_absolute_path();
        auto match = router.match(method, abs_path);
        if (match.handler) {
          if (match.handler->asynchronous) {
            using dispatch_type =
                async_dispatch<decltype(deferred.defer())>;
            auto sender = deferred.defer();
            dispatch_type task{req, match.handler, std::move(sender)};
            if (!executor.try_submit(std::move(task))) task.reject();
            return;
          }
          match.handler->callback(*req, res);
        } else if (match.parametrized_handler) {
          if (match.parametrized_handler->asynchronous()) {
            using dispatch_type =
                async_dispatch<decltype(deferred.defer())>;
            auto sender = deferred.defer();
            dispatch_type task{req, match.parametrized_handler, abs_path,
                               std::move(sender)};
            if (!executor.try_submit(std::move(task))) task.reject();
            return;
          }
          match.parametrized_handler->invoke(*req, res, abs_path);
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
    finalize_response_(*req, res);
  }

 private:
  using parametrized_handler_pointer =
      decltype(std::declval<typename ROty::route_match>()
                   .parametrized_handler);
  // +=========================================================================+
  // | [>] async_dispatch                                           ( private ) |
  // +=========================================================================+
  template <typename Sty>
  class async_dispatch {
   public:
    async_dispatch(std::shared_ptr<RQty> req,
                   const typename ROty::handler_data* handler, Sty sender)
        : req_{std::move(req)},
          handler_{handler},
          sender_{std::move(sender)} {}
    async_dispatch(
        std::shared_ptr<RQty> req,
        parametrized_handler_pointer handler,
        std::string_view path, Sty sender)
        : req_{std::move(req)},
          parametrized_handler_{handler},
          path_{path},
          sender_{std::move(sender)} {}
    async_dispatch(const async_dispatch&) = delete;
    async_dispatch(async_dispatch&&) noexcept = default;
    ~async_dispatch() = default;
    async_dispatch& operator=(const async_dispatch&) = delete;
    async_dispatch& operator=(async_dispatch&&) noexcept = delete;

    void operator()() {
      try {
        RSty res;
        if (handler_) {
          handler_->callback(*req_, res);
        } else {
          parametrized_handler_->invoke(*req_, res, path_);
        }
        finalize_response_(*req_, res);
        sender_.complete(res.serialize());
        return;
      } catch (...) {
      }
      try {
        RSty res;
        res.internal_server_error_500();
        finalize_response_(*req_, res);
        sender_.complete(res.serialize());
      } catch (...) {
      }
    }

    void reject() {
      try {
        RSty res;
        res.service_unavailable_503();
        finalize_response_(*req_, res);
        sender_.complete(res.serialize());
      } catch (...) {
      }
    }

   private:
    std::shared_ptr<RQty> req_;
    const typename ROty::handler_data* handler_{nullptr};
    parametrized_handler_pointer parametrized_handler_{nullptr};
    std::string_view path_;
    Sty sender_;
  };
  // +=========================================================================+
  // | [>] finalize_response_                                       ( private ) |
  // +=========================================================================+
  static void finalize_response_(const RQty& req, RSty& res) {
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
