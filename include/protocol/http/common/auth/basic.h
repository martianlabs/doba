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

#ifndef martianlabs_doba_protocol_http_auth_basic_h
#define martianlabs_doba_protocol_http_auth_basic_h

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
// | [>] basic                                                       ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class basic {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  explicit basic(
      std::string_view realm,
      std::function<bool(std::string_view, std::string_view)> validator)
      : validator_(std::move(validator)) {
    if (!validator_) throw std::invalid_argument("Missing Basic validator");
    std::string quoted;
    for (const unsigned char value : realm) {
      if (value < 0x20 || value > 0x7e) {
        throw std::invalid_argument("Invalid authentication realm");
      }
      if (value == '"' || value == '\\') quoted.push_back('\\');
      quoted.push_back(static_cast<char>(value));
    }
    challenge_ = "Basic realm=\"" + quoted + "\"";
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
      if (found) return {authentication_status::kInvalidRequest, {}};
      found = true;
      credentials = header.second;
    }
    const std::size_t separator = credentials.find(' ');
    if (!found || separator == std::string_view::npos ||
        !helpers::iequals(credentials.substr(0, separator), "Basic")) {
      return {authentication_status::kUnauthenticated, challenge_};
    }
    credentials.remove_prefix(separator);
    while (!credentials.empty() && credentials.front() == ' ') {
      credentials.remove_prefix(1);
    }
    if (credentials.empty() || !helpers::check_base64_value(credentials)) {
      return {authentication_status::kUnauthenticated, challenge_};
    }
    std::string decoded;
    decoded.reserve(credentials.size() / 4 * 3);
    unsigned int bits = 0;
    unsigned int count = 0;
    for (const char value : credentials) {
      if (value == '=') break;
      const unsigned int digit =
          value >= 'A' && value <= 'Z'   ? value - 'A'
          : value >= 'a' && value <= 'z' ? value - 'a' + 26
          : value >= '0' && value <= '9' ? value - '0' + 52
          : value == '+'                 ? 62
                                         : 63;
      bits = (bits << 6) | digit;
      count += 6;
      if (count >= 8) {
        count -= 8;
        decoded.push_back(static_cast<char>((bits >> count) & 0xff));
      }
    }
    if (count && (bits & ((1u << count) - 1u))) {
      return {authentication_status::kUnauthenticated, challenge_};
    }
    // RFC 7617 S2: the first colon separates user-id and password.
    const std::size_t colon = decoded.find(':');
    if (colon == std::string::npos) {
      return {authentication_status::kUnauthenticated, challenge_};
    }
    for (const unsigned char value : decoded) {
      if (value < 0x20 || value == 0x7f) {
        return {authentication_status::kUnauthenticated, challenge_};
      }
    }
    const std::string_view pair(decoded);
    if (validator_(pair.substr(0, colon), pair.substr(colon + 1))) {
      return {authentication_status::kAccepted, {}};
    }
    return {authentication_status::kUnauthenticated, challenge_};
  }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  std::function<bool(std::string_view, std::string_view)> validator_;
  std::string challenge_;
};
}  // namespace martianlabs::doba::protocol::http::auth

#endif
