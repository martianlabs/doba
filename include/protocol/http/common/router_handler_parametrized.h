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

#ifndef martianlabs_doba_protocol_http_router_handler_parametrized_h
#define martianlabs_doba_protocol_http_router_handler_parametrized_h

#include <array>
#include <charconv>
#include <concepts>
#include <functional>
#include <memory>
#include <stop_token>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include "common/task.h"
#include "protocol/http/common/helpers.h"

namespace martianlabs::doba::protocol::http {
namespace detail {
template <typename>
inline constexpr bool dependent_false_v = false;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] extract_route_parameters                                 ( function ) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   N - number of route parameters to extract                               |
// +---------------------------------------------------------------------------+
// | This function parses route parameters from the provided string view into  |
// | the appropriate types and stores them in the output array.                |
// | It returns true if all parameters were successfully parsed, and false     |
// | otherwise. If the number of parameters does not match N, it returns false.|
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
// | This function parses a route parameter from the provided string view into |
// | the appropriate type and stores it in the output parameter.               |
// | It returns true if the parameter was successfully parsed, and false       |
// | otherwise. If the type T is not supported, a static assertion will        |
// | fail at compile time.                                                     |
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
// | This function checks if the route parameter from the provided string view |
// | matches the expected type. It returns true if the parameter matches, and  |
// | false otherwise.                                                          |
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
// | This function checks if the route parameters from the provided array of   |
// | string views match the expected types. It returns true if all parameters  |
// | match, and false otherwise.                                               |
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
// | This function checks if the route parameters from the provided array of   |
// | string views match the expected types. It returns true if all parameters  |
// | match, and false otherwise.                                               |
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
// | This function parses the route parameters from the provided array of      |
// | string views into the appropriate types and stores them in the provided   |
// | tuple. It returns true if all parameters were successfully parsed, and    |
// | false otherwise.                                                          |
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
// | This function invokes a route handler with the provided request,          |
// | response, pattern, and path. It extracts the route parameters from the    |
// | pattern and path, parses them into the appropriate types, and then        |
// | invokes the handler with the request, response, and parsed parameters.    |
// | If the route parameters cannot be extracted or parsed, it returns         |
// | without invoking the handler.                                             |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename Hty, typename RQty, typename RSty, typename... Args>
void invoke_route_handler(Hty& handler, const RQty& req, RSty& res,
                          std::string_view pattern, std::string_view path) {
  std::array<std::string_view, sizeof...(Args)> parameters;
  if (!extract_route_parameters(pattern, path, parameters)) return;
  std::tuple<std::decay_t<Args>...> values;
  if (!parse_route_parameters_<Args...>(parameters, values,
                                        std::index_sequence_for<Args...>{})) {
    return;
  }
  std::apply([&handler, &req, &res](
                 auto&... value) { std::invoke(handler, req, res, value...); },
             values);
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] invoke_async_route_handler                               ( function ) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   Hty - handler being used                                                |
// |   RQty - request being used                                               |
// |   RSty - response being used                                              |
// |   Args - route parameters being used                                      |
// +---------------------------------------------------------------------------+
// | This function invokes an asynchronous route handler with the provided     |
// | request, stop token, pattern, and path. It extracts the route parameters  |
// | from the pattern and path, parses them into the appropriate types,        |
// | and then invokes the handler with the request, stop token, and parsed     |
// | parameters. If the route parameters cannot be extracted or parsed,        |
// | it throws a runtime error.                                                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename Hty, typename RQty, typename RSty, typename... Args>
common::task<RSty> invoke_async_route_handler(std::shared_ptr<Hty> handler,
                                              std::shared_ptr<const RQty> req,
                                              std::stop_token stop_token,
                                              std::string_view pattern,
                                              std::string_view path) {
  std::array<std::string_view, sizeof...(Args)> parameters;
  if (!extract_route_parameters(pattern, path, parameters)) {
    throw std::runtime_error("The route parameters could not be extracted");
  }
  std::tuple<std::decay_t<Args>...> values;
  if (!parse_route_parameters_<Args...>(parameters, values,
                                        std::index_sequence_for<Args...>{})) {
    throw std::runtime_error("The route parameters could not be parsed");
  }
  co_return co_await std::apply(
      [&handler, &req, &stop_token](auto&... value) {
        return std::invoke(*handler, req, stop_token, value...);
      },
      values);
}
}  // namespace detail

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_handler_parametrized                                 ( class ) |
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
  // +=========================================================================+
  using matcher_type = bool (*)(std::string_view, std::string_view);
  using callback_type = std::function<void(const RQty&, RSty&, std::string_view,
                                           std::string_view)>;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  router_handler_parametrized(std::string pattern, matcher_type matcher,
                              callback_type callback)
      : pattern_{std::move(pattern)},
        matcher_{matcher},
        callback_{std::move(callback)} {}
  router_handler_parametrized(std::string pattern, matcher_type matcher,
                              std::function<common::task<RSty>(
                                  std::shared_ptr<const RQty>, std::stop_token,
                                  std::string_view, std::string_view)>
                                  callback)
      : pattern_{std::move(pattern)},
        matcher_{matcher},
        async_callback_{std::move(callback)} {}
  // +=========================================================================+
  // | [>] matches                                                  ( public ) |
  // +=========================================================================+
  [[nodiscard]] bool matches(std::string_view path) const {
    return matcher_(pattern_, path);
  }
  // +=========================================================================+
  // | [>] invoke                                                   ( public ) |
  // +=========================================================================+
  void invoke(const RQty& req, RSty& res, std::string_view path) const {
    callback_(req, res, pattern_, path);
  }
  // +=========================================================================+
  // | [>] invoke_async                                             ( public ) |
  // +=========================================================================+
  common::task<RSty> invoke_async(std::shared_ptr<const RQty> req,
                                  std::stop_token stop_token,
                                  std::string_view path) const {
    return async_callback_(std::move(req), stop_token, pattern_, path);
  }
  // +=========================================================================+
  // | [>] is_async                                                 ( public ) |
  // +=========================================================================+
  [[nodiscard]] bool is_async() const {
    return static_cast<bool>(async_callback_);
  }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  std::string pattern_;
  matcher_type matcher_;
  callback_type callback_;
  std::function<common::task<RSty>(std::shared_ptr<const RQty>, std::stop_token,
                                   std::string_view, std::string_view)>
      async_callback_;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] make_router_handler_parametrized                           (function) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   RQty - request being used                                               |
// |   RSty - response being used                                              |
// |   Args - route parameters being used                                      |
// |   Hty - handler being used                                                |
// +---------------------------------------------------------------------------+
// | This function creates a router_handler_parametrized object for a          |
// | synchronous route handler. It takes a pattern and a handler as input,     |
// | and returns a router_handler_parametrized object that can be used to      |
// | match and invoke the handler with the appropriate request, response, and  |
// | parsed route parameters.                                                  |
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

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] make_router_handler_parametrized_async                     (function) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   RQty - request being used                                               |
// |   RSty - response being used                                              |
// |   Args - route parameters being used                                      |
// |   Hty - handler being used                                                |
// +---------------------------------------------------------------------------+
// | This function creates a router_handler_parametrized object for an         |
// | asynchronous route handler. It takes a pattern and a handler as input,    |
// | and returns a router_handler_parametrized object that can be used to      |
// | match and invoke the handler with the appropriate request, stop token,    |
// | and parsed route parameters.                                              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty, typename... Args, typename Hty>
auto make_router_handler_parametrized_async(std::string_view pattern,
                                            Hty&& handler) {
  auto shared_handler =
      std::make_shared<std::decay_t<Hty>>(std::forward<Hty>(handler));
  return router_handler_parametrized<RQty, RSty>(
      std::string(pattern), &detail::match_route_parameters<Args...>,
      [handler = std::move(shared_handler)](
          std::shared_ptr<const RQty> req, std::stop_token stop_token,
          std::string_view route_pattern, std::string_view path) {
        return detail::invoke_async_route_handler<std::decay_t<Hty>, RQty, RSty,
                                                  Args...>(
            handler, std::move(req), stop_token, route_pattern, path);
      });
}
}  // namespace martianlabs::doba::protocol::http

#endif
