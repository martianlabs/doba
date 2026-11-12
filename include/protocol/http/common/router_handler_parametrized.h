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

#ifndef martianlabs_doba_protocol_http_router_handler_parametrized_h
#define martianlabs_doba_protocol_http_router_handler_parametrized_h

#include <array>
#include <charconv>
#include <concepts>
#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include "protocol/http/common/helpers.h"

namespace martianlabs::doba::protocol::http {
namespace detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] dependent_false_v                                        ( constant ) |
// +---------------------------------------------------------------------------+
// | Rejects unsupported parameter types when their parser is instantiated.    |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename>
inline constexpr bool dependent_false_v = false;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] extract_route_parameters                                 ( function ) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   N - number of route parameters to extract                               |
// +---------------------------------------------------------------------------+
// | Matches literal segments and collects exactly N non-empty :parameters.    |
// | The views borrow path; no type conversion happens here.                   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <std::size_t N>
bool extract_route_parameters(std::string_view pattern, std::string_view path,
                              std::array<std::string_view, N>& parameters) {
  std::size_t pattern_pos = 0;
  std::size_t path_pos = 0;
  std::size_t parameter_pos = 0;
  for (;;) {
    const std::size_t pattern_end = pattern.find('/', pattern_pos);
    const std::size_t path_end = path.find('/', path_pos);
    const std::string_view pattern_segment =
        pattern.substr(pattern_pos, pattern_end == std::string_view::npos
                                        ? pattern.size() - pattern_pos
                                        : pattern_end - pattern_pos);
    const std::string_view path_segment = path.substr(
        path_pos, path_end == std::string_view::npos ? path.size() - path_pos
                                                     : path_end - path_pos);
    if (!pattern_segment.empty() && pattern_segment.front() == ':') {
      if (path_segment.empty() || parameter_pos == N) return false;
      parameters[parameter_pos++] = path_segment;
    } else if (pattern_segment != path_segment) {
      return false;
    }
    if (pattern_end == std::string_view::npos ||
        path_end == std::string_view::npos) {
      return pattern_end == path_end && parameter_pos == N;
    }
    pattern_pos = pattern_end + 1;
    path_pos = path_end + 1;
  }
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parse_route_parameter                                    ( function ) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   T - type of the route parameter being parsed                            |
// +---------------------------------------------------------------------------+
// | Converts the whole value to text, a boolean or a number.                  |
// | Booleans accept 1/0 and true/false, ignoring letter case.                 |
// | A string_view borrows input; a string gets its own copy.                  |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename T>
bool parse_route_parameter(std::string_view input, T& output) {
  if constexpr (std::same_as<T, std::string_view>) {
    output = input;
    return true;
  } else if constexpr (std::same_as<T, std::string>) {
    output.assign(input);
    return true;
  } else if constexpr (std::same_as<T, bool>) {
    if (input == "1" || helpers::iequals(input, "true")) {
      output = true;
      return true;
    }
    if (input == "0" || helpers::iequals(input, "false")) {
      output = false;
      return true;
    }
    return false;
  } else if constexpr (std::integral<T> || std::floating_point<T>) {
    const char* begin = input.data();
    const char* end = begin + input.size();
    const auto result = std::from_chars(begin, end, output);
    return result.ec == std::errc{} && result.ptr == end;
  } else {
    static_assert(dependent_false_v<T>,
                  "No parser is available for this route parameter type");
  }
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] matches_route_parameter                                  ( function ) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   T - route parameter type being used                                     |
// +---------------------------------------------------------------------------+
// | Checks whether the value fits T, without keeping the parsed result.       |
// | Text types need no conversion or copy for this check.                     |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename T>
bool matches_route_parameter(std::string_view input) {
  if constexpr (std::same_as<T, std::string_view> ||
                std::same_as<T, std::string>) {
    return true;
  } else {
    T output{};
    return parse_route_parameter(input, output);
  }
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] match_route_parameters_                                  ( function ) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   Args - route parameters being used                                      |
// +---------------------------------------------------------------------------+
// | Checks each extracted value against its handler argument type.            |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename... Args, std::size_t... I>
bool match_route_parameters_(
    const std::array<std::string_view, sizeof...(Args)>& parameters,
    std::index_sequence<I...>) {
  return (matches_route_parameter<std::decay_t<Args>>(parameters[I]) && ...);
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] match_route_parameters                                   ( function ) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   Args - route parameters being used                                      |
// +---------------------------------------------------------------------------+
// | Matches the path and checks that its parameters fit the handler.          |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename... Args>
bool match_route_parameters(std::string_view pattern, std::string_view path) {
  std::array<std::string_view, sizeof...(Args)> parameters;
  if (!extract_route_parameters(pattern, path, parameters)) return false;
  return match_route_parameters_<Args...>(parameters,
                                          std::index_sequence_for<Args...>{});
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parse_route_parameters_                                  ( function ) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   Args - route parameters being used                                      |
// +---------------------------------------------------------------------------+
// | Fills the argument tuple in route order. Stops at the first bad value.    |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename... Args, std::size_t... I>
bool parse_route_parameters_(
    const std::array<std::string_view, sizeof...(Args)>& parameters,
    std::tuple<std::decay_t<Args>...>& values, std::index_sequence<I...>) {
  return (parse_route_parameter(parameters[I], std::get<I>(values)) && ...);
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] invoke_route_handler                                     ( function ) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   Hty - handler being used                                                |
// |   RQty - request being used                                               |
// |   RSty - response being used                                              |
// |   Args - route parameters being used                                      |
// +---------------------------------------------------------------------------+
// | Invokes the handler with the request and parsed route parameters.         |
// | Throws if the path or parameters do not match.                            |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename Hty, typename RQty, typename RSty, typename... Args>
void invoke_route_handler(Hty& handler, const RQty& req, RSty& res,
                          std::string_view pattern, std::string_view path) {
  std::array<std::string_view, sizeof...(Args)> parameters;
  if (!extract_route_parameters(pattern, path, parameters)) {
    throw std::runtime_error("The route parameters could not be extracted");
  }
  std::tuple<std::decay_t<Args>...> values;
  if (!parse_route_parameters_<Args...>(parameters, values,
                                        std::index_sequence_for<Args...>{})) {
    throw std::runtime_error("The route parameters could not be parsed");
  }
  std::apply([&handler, &req, &res](
                 auto&... value) { std::invoke(handler, req, res, value...); },
             values);
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_handler_parametrized                                 ( class ) |
// +---------------------------------------------------------------------------+
// | Keeps a route pattern with its matching and handler callbacks.            |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   RQty - request being used                                               |
// |   RSty - response being used                                              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty>
class router_handler_parametrized {
 public:
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // | The matcher checks a path; the callback parses and calls the handler.   |
  // +-------------------------------------------------------------------------+
  using matcher_type = bool (*)(std::string_view, std::string_view);
  using callback_type = std::function<void(const RQty&, RSty&, std::string_view,
                                           std::string_view)>;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  // | Takes ownership of the pattern and callback, keeping the matcher too.   |
  // +-------------------------------------------------------------------------+
  router_handler_parametrized(std::string pattern, matcher_type matcher,
                              callback_type callback)
      : pattern_{std::move(pattern)},
        matcher_{matcher},
        callback_{std::move(callback)} {}
  // +=========================================================================+
  // | [>] matches                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  // | Checks the path and parameter types without calling the handler.        |
  // +-------------------------------------------------------------------------+
  [[nodiscard]] bool matches(std::string_view path) const {
    return matcher_(pattern_, path);
  }
  // +=========================================================================+
  // | [>] invoke                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  // | Parses the path parameters and calls the handler with request/response. |
  // | Throws if the path or a parameter cannot be parsed.                     |
  // +-------------------------------------------------------------------------+
  void invoke(const RQty& req, RSty& res, std::string_view path) const {
    callback_(req, res, pattern_, path);
  }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  // | Owned pattern and callbacks used to match and invoke the route.         |
  // +-------------------------------------------------------------------------+
  std::string pattern_;
  matcher_type matcher_;
  callback_type callback_;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] make_router_handler_parametrized                         ( function ) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   RQty - request being used                                               |
// |   RSty - response being used                                              |
// |   Args - route parameters being used                                      |
// |   Hty - handler being used                                                |
// +---------------------------------------------------------------------------+
// | Copies the pattern and stores the synchronous handler.                    |
// | Uses Args to match and parse the route parameters before each call.       |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty, typename... Args, typename Hty>
auto make_router_handler_parametrized(std::string_view pattern, Hty&& handler) {
  using handler_type = std::decay_t<Hty>;
  return router_handler_parametrized<RQty, RSty>(
      std::string(pattern), &detail::match_route_parameters<Args...>,
      [handler = handler_type(std::forward<Hty>(handler))](
          const RQty& req, RSty& res, std::string_view route_pattern,
          std::string_view path) mutable {
        detail::invoke_route_handler<handler_type, RQty, RSty, Args...>(
            handler, req, res, route_pattern, path);
      });
}
}  // namespace detail
}  // namespace martianlabs::doba::protocol::http

#endif
