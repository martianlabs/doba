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

#ifndef martianlabs_doba_protocol_http_auth_bearer_h
#define martianlabs_doba_protocol_http_auth_bearer_h

#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "protocol/http/common/auth/authentication_result.h"
#include "protocol/http/common/helpers.h"

namespace martianlabs::doba::protocol::http::auth {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] bearer                                                      ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class bearer {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  explicit bearer(std::string_view realm,
                  std::function<bool(std::string_view)> validator)
      : validator_(std::move(validator)) {
    if (!validator_) throw std::invalid_argument("Missing Bearer validator");
    std::string quoted;
    for (const unsigned char value : realm) {
      if (value < 0x20 || value > 0x7e) {
        throw std::invalid_argument("Invalid authentication realm");
      }
      if (value == '"' || value == '\\') quoted.push_back('\\');
      quoted.push_back(static_cast<char>(value));
    }
    challenge_ = "Bearer realm=\"" + quoted + "\"";
  }
  // +=========================================================================+
  // | [>] authenticate                                             ( public ) |
  // +-------------------------------------------------------------------------+
  template <typename RQty>
  authentication_result authenticate(const RQty& request) const {
    std::string_view credentials;
    bool found = false;
    for (std::size_t i = 0; i < request.get_headers_length(); i++) {
      const auto header = request.get_header(i);
      if (!helpers::iequals(header.first, "Authorization")) continue;
      if (found) {
        return {authentication_status::kInvalidRequest,
                challenge_ + ", error=\"invalid_request\""};
      }
      found = true;
      credentials = header.second;
    }
    const std::size_t separator = credentials.find(' ');
    const auto scheme = credentials.substr(0, separator);
    if (!found || !helpers::iequals(scheme, "Bearer")) {
      return {authentication_status::kUnauthenticated, challenge_};
    }
    if (separator != std::string_view::npos) {
      credentials.remove_prefix(separator);
      while (!credentials.empty() && credentials.front() == ' ') {
        credentials.remove_prefix(1);
      }
    } else {
      credentials = {};
    }
    if (!helpers::is_token68(credentials)) {
      return {authentication_status::kInvalidRequest,
              challenge_ + ", error=\"invalid_request\""};
    }
    if (validator_(credentials)) {
      return {authentication_status::kAccepted, {}};
    }
    return {authentication_status::kUnauthenticated,
            challenge_ + ", error=\"invalid_token\""};
  }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  std::function<bool(std::string_view)> validator_;
  std::string challenge_;
};
}  // namespace martianlabs::doba::protocol::http::auth

#endif
