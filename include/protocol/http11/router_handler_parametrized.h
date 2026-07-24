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

#ifndef martianlabs_doba_protocol_http11_router_handler_parametrized_h
#define martianlabs_doba_protocol_http11_router_handler_parametrized_h

#include <charconv>
#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "protocol/http11/router_handler_parametrized_base.h"

namespace martianlabs::doba::protocol::http11 {
template <typename RQty, typename RSty, typename... Args>
class router_handler_parametrized;

namespace detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_handler_signature                                   ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename>
struct router_handler_signature;

template <typename OUty, typename RQty, typename RSty,
          typename... Args>
struct router_handler_signature_base {
  using return_type = OUty;
  using request_type = RQty;
  using response_type = RSty;
  static constexpr std::size_t parameter_count = sizeof...(Args);

  template <typename RQty, typename RSty, typename Hty>
  static auto make_parametrized(Hty&& handler) {
    return router_handler_parametrized<RQty, RSty, std::decay_t<Args>...>(
        std::forward<Hty>(handler));
  }
};

template <typename Cty, typename Retty, typename Reqty, typename Resty,
          typename... Args>
struct router_handler_signature<Retty (Cty::*)(Reqty, Resty, Args...) const>
    : router_handler_signature_base<Retty, Reqty, Resty, Args...> {};

template <typename Cty, typename Retty, typename Reqty, typename Resty,
          typename... Args>
struct router_handler_signature<Retty (Cty::*)(Reqty, Resty, Args...)>
    : router_handler_signature_base<Retty, Reqty, Resty, Args...> {};

template <typename Hty>
concept router_handler_lambda = requires {
  typename router_handler_signature<
      decltype(&std::decay_t<Hty>::operator())>::request_type;
};
}  // namespace detail
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] dependent_false_v                                            ( type ) |
// +---------------------------------------------------------------------------+
// | This is a helper variable template that is always false, used for static  |
// | assertions in templates.                                                  |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename>
inline constexpr bool dependent_false_v = false;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_handler_parametrized                                 ( class ) |
// +---------------------------------------------------------------------------+
// | This class is a concrete implementation of the                            |
// | router_handler_parametrized_base interface. It allows for parameterized   |
// | routing by converting parameters in a local tuple and invoking a callback |
// | function. The class supports parsing various types of                     |
// | parameters from string values, including strings, booleans,               |
// | integral types, and floating-point types. If a parameter type is not      |
// | supported, a static assertion will trigger a compile-time error.          |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   RQty - request being used                                               |
// |   RSty - response being used                                              |
// |   Args - parameter types for the handler function                         |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty, typename... Args>
class router_handler_parametrized final
    : public router_handler_parametrized_base<RQty, RSty> {
  // +=========================================================================+
  // | [>] TYPEs                                                   ( private ) |
  // +=========================================================================+
  using tuple_type = std::tuple<std::decay_t<Args>...>;
  using callback_type =
      std::function<void(std::shared_ptr<const RQty>, std::shared_ptr<RSty>,
                         const std::decay_t<Args>&...)>;

 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  explicit router_handler_parametrized(callback_type callback) {
    callback_ = std::move(callback);
  }
  // +=========================================================================+
  // | [>] invoke                                                   ( public ) |
  // +=========================================================================+
  [[nodiscard]]
  bool invoke(
      std::shared_ptr<const RQty> req, std::shared_ptr<RSty> res,
      const std::vector<std::pair<std::string_view, std::string_view>>&
          parameters) const override {
    if (parameters.size() != sizeof...(Args)) return false;
    tuple_type values;
    for (std::size_t i = 0; i < parameters.size(); i++) {
      if (!set_impl(values, i, parameters[i].second)) {
        return false;
      }
    }
    std::apply(
        [this, &req, &res](const auto&... values) {
          callback_(std::forward<const std::shared_ptr<const RQty>>(req),
                    std::forward<const std::shared_ptr<RSty>>(res), values...);
        },
        values);
    return true;
  }

 private:
  // +=========================================================================+
  // | [>] parse                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  // | This method parses a string value into the appropriate type for a       |
  // | parameter. It supports std::string, bool, integral types, and floating  |
  // | point types. If the parsing is successful, it returns true; otherwise,  |
  // | it returns false.                                                       |
  // +=========================================================================+
  template <typename T>
  static bool parse(std::string_view input, T& output) {
    if constexpr (std::same_as<T, std::string>) {
      output.assign(input);
      return true;
    } else if constexpr (std::same_as<T, bool>) {
      if (helpers::iequals(input, "true") || input == "1") {
        output = true;
        return true;
      }
      if (helpers::iequals(input, "false") || input == "0") {
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
                    "No parser is available for this parameter type");
    }
  }
  // +=========================================================================+
  // | [>] set_impl                                                ( private ) |
  // +-------------------------------------------------------------------------+
  // | This method converts the parameter at the specified index and stores it |
  // | in the local tuple.                                                     |
  // +=========================================================================+
  template <std::size_t Index = 0>
  bool set_impl(tuple_type& parameters, std::size_t index,
                std::string_view value) const {
    if constexpr (Index >= sizeof...(Args)) {
      return false;
    } else {
      if (index == Index) {
        using value_type = std::tuple_element_t<Index, tuple_type>;
        value_type parsed{};
        if (!parse(value, parsed)) return false;
        std::get<Index>(parameters) = std::move(parsed);
        return true;
      }
      return set_impl<Index + 1>(parameters, index, value);
    }
  }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  callback_type callback_;  // Callback function to be invoked
};
}  // namespace martianlabs::doba::protocol::http11

#endif
