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

#ifndef martianlabs_doba_protocol_http_v11_request_processor_sync_h
#define martianlabs_doba_protocol_http_v11_request_processor_sync_h

#include <string>

#include "protocol/http/common/header_names.h"
#include "protocol/http/common/router_match_result.h"
#include "protocol/http/common/target.h"

namespace martianlabs::doba::protocol::http::v11::detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] request_processor_sync                                      ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty, typename ROty>
class request_processor_sync {
 public:
  // +=========================================================================+
  // | [>] execute                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  static void execute(const ROty& router, const RQty& request, RSty& response,
                      const router_match_result<RQty, RSty>& match) {
    switch (request.get_target()) {
      case target::kOriginForm:
      case target::kAbsoluteForm:
        if (match.handler) {
          (*match.handler)(request, response);
        } else if (match.parametrized_handler) {
          match.parametrized_handler->invoke(request, response,
                                             request.get_absolute_path());
        } else {
          const std::string allowed =
              router.allowed_methods(request.get_absolute_path());
          if (allowed.empty()) {
            response.not_found_404();
          } else {
            response.method_not_allowed_405();
            response.set_header(header_names::kAllow, allowed);
          }
        }
        break;
      case target::kAuthorityForm:
        response.not_implemented_501();
        break;
      case target::kAsteriskForm:
        response.ok_200();
        break;
      default:
        response.bad_request_400();
        break;
    }
  }
};
}  // namespace martianlabs::doba::protocol::http::v11::detail

#endif
