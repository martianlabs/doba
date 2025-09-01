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

#ifndef martianlabs_doba_protocol_http11_checkers_connection_h
#define martianlabs_doba_protocol_http11_checkers_connection_h

#include <ranges>
#include <string_view>

#include "protocol/http11/constants.h"
#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::checkers {
// =============================================================================
// |                                                            [ connection ] |
// +---------------------+-----------------------------------------------------+
// | Field               | Definition                                          |
// +---------------------+-----------------------------------------------------+
// | Connection          | 1#connection-option                                 |
// | connection-option   | token                                               |
// +---------------------+-----------------------------------------------------+
// =============================================================================
static auto connection_check_fn = [](std::string_view v) -> bool {
  for (auto token : v | std::views::split(constants::character::kComma)) {
    std::string_view value(&*token.begin(), std::ranges::distance(token));
    value = helpers::ows_ltrim(helpers::ows_rtrim(value));
    if (!helpers::is_token(value)) return false;
  }
  return true;
};
}  // namespace martianlabs::doba::protocol::http11::checkers

#endif
