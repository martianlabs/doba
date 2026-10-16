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

#ifndef martianlabs_doba_protocol_http_router_parameter_storage_h
#define martianlabs_doba_protocol_http_router_parameter_storage_h

#include <concepts>
#include <memory>
#include <string_view>
#include <tuple>
#include <utility>

namespace martianlabs::doba::protocol::http {
namespace detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_parameter_storage                                    ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class router_parameter_storage {
 public:
  // +=========================================================================+
  // | [>] acquire                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  template <typename T>
  T& acquire() {
    auto& head = available<T>().head;
    auto value = std::move(head);
    if (value) {
      head = std::move(value->next);
    } else {
      value = std::make_unique<block<T>>();
    }
    auto& result = value->values;
    storage_ = {value.release(), [](void* address) noexcept {
                  std::unique_ptr<block<T>> value{
                      static_cast<block<T>*>(address)};
                  std::apply([](auto&... values) {
                    auto clear_view = []<typename V>(V& value) {
                      if constexpr (std::same_as<V, std::string_view>) {
                        value = {};
                      }
                    };
                    (clear_view(values), ...);
                  }, value->values);
                  auto& head = available<T>().head;
                  value->next = std::move(head);
                  head = std::move(value);
                }};
    return result;
  }
  // +=========================================================================+
  // | [>] reset                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  void reset() noexcept { storage_.reset(); }

 private:
  // +=========================================================================+
  // | [>] TYPEs                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  template <typename T>
  struct block {
    T values{};
    std::unique_ptr<block> next;
  };
  template <typename T>
  struct pool {
    std::unique_ptr<block<T>> head;
    ~pool() {
      while (head) {
        auto value = std::move(head);
        head = std::move(value->next);
      }
    }
  };
  // +=========================================================================+
  // | [>] available                                               ( private ) |
  // +-------------------------------------------------------------------------+
  template <typename T>
  static pool<T>& available() noexcept {
    thread_local pool<T> value;
    return value;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  std::unique_ptr<void, void (*)(void*)> storage_{nullptr, nullptr};
};
}  // namespace detail
}  // namespace martianlabs::doba::protocol::http

#endif
