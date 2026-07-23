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

#include <concepts>
#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

template <typename... Args>
class parameter_store {
 private:
  using tuple_type = std::tuple<std::decay_t<Args>...>;

  using callback_type = std::function<void(const std::decay_t<Args>&...)>;

  template <std::size_t Index = 0, typename T>
  bool set_impl(std::size_t index, T&& value) {
    if constexpr (Index < sizeof...(Args)) {
      if (index == Index) {
        using element_type = std::tuple_element_t<Index, tuple_type>;

        if constexpr (std::assignable_from<element_type&, T&&>) {
          std::get<Index>(parameters_) = std::forward<T>(value);

          return true;
        }

        return false;
      }

      return set_impl<Index + 1>(index, std::forward<T>(value));
    }

    return false;
  }

 public:
  explicit parameter_store(Args&&... args)
      : parameters_(std::forward<Args>(args)...) {}

  template <typename T>
  bool set(std::size_t index, T&& value) {
    return set_impl(index, std::forward<T>(value));
  }

  template <typename Callable>
  void set_callback(Callable&& callback) {
    callback_ = std::forward<Callable>(callback);
  }

  void invoke() const { std::apply(callback_, parameters_); }

 private:
  tuple_type parameters_;
  callback_type callback_;
};

template <typename... Args>
parameter_store(Args&&...) -> parameter_store<Args...>;

int main(int argc, char* argv[]) {
  parameter_store pepe(42, 3.14);
  pepe.set(0, 42);
  pepe.set(1, 3.14);
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
