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

#include "platform.h"

#include <charconv>
#include <concepts>
#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

template <typename... Args>
class parameter_store {
 private:
  using tuple_type = std::tuple<std::optional<std::decay_t<Args>>...>;

  using callback_type = std::function<void(const std::decay_t<Args>&...)>;

 public:
  enum class set_result { success, invalid_index, invalid_value };

  explicit parameter_store(callback_type callback)
      : callback_(std::move(callback)) {}

  set_result set(std::size_t index, std::string_view value) {
    return set_impl(index, value);
  }

  bool complete() const {
    return std::apply(
        [](const auto&... values) { return (values.has_value() && ...); },
        parameters_);
  }

  bool invoke() const {
    if (!complete()) return false;

    std::apply([this](const auto&... values) { callback_((*values)...); },
               parameters_);

    return true;
  }

 private:
  template <typename T>
  static bool parse(std::string_view input, T& output) {
    if constexpr (std::same_as<T, std::string>) {
      output.assign(input);
      return true;
    } else if constexpr (std::integral<T> || std::floating_point<T>) {
      const char* begin = input.data();
      const char* end = begin + input.size();

      const auto result = std::from_chars(begin, end, output);

      return result.ec == std::errc{} && result.ptr == end;
    } else {
      static_assert(std::same_as<T, void>,
                    "No parser available for this parameter type");

      return false;
    }
  }

  template <std::size_t Index = 0>
  set_result set_impl(std::size_t index, std::string_view value) {
    if constexpr (Index < sizeof...(Args)) {
      if (index == Index) {
        using optional_type = std::tuple_element_t<Index, tuple_type>;

        using value_type = typename optional_type::value_type;

        value_type parsed{};

        if (!parse(value, parsed)) return set_result::invalid_value;

        std::get<Index>(parameters_) = std::move(parsed);

        return set_result::success;
      }

      return set_impl<Index + 1>(index, value);
    }

    return set_result::invalid_index;
  }

 private:
  tuple_type parameters_;
  callback_type callback_;
};

int main(int argc, char* argv[]) {
  parameter_store<int, std::string, double> store{
      [](const int& id, const std::string& name, const double& score) {
        // Parámetros validados y de solo lectura.
      }};

  store.set(0, "42");
  store.set(1, "marcos");
  store.set(2, "9.75");

  store.invoke();
  return 0;
}

/*
#include "network/environment.h"
#include "protocol/http11/server.h"

using namespace martianlabs::doba::common;
using namespace martianlabs::doba::protocol::http11;

int main(int argc, char* argv[]) {
  martianlabs::doba::network::startup();
  date_server::get().start();
  server http_server;
  http_server.add_route(
      "GET", "/pipeline",
      [](std::shared_ptr<const request> req, std::shared_ptr<response> res) {
        res->ok_200()
            .add_header("Server", "doba.")
            .add_header("Date", date_server::get().current())
            .add_header("Content-Type", "text/plain; charset=utf-8")
            .set_body("ok");
      });
  http_server.start("8080");
  std::cin.get();
  date_server::get().stop();
  martianlabs::doba::network::cleanup();
  return 0;
}
*/
