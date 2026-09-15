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

#include <memory>
#include <stop_token>
#include <type_traits>

#include "protocol/http/common/router_handler_signature.h"
#include "test_helper.h"

namespace {
struct request {};
struct response {};
using martianlabs::doba::common::task;
using martianlabs::doba::protocol::http::router_async_handler_lambda;
using martianlabs::doba::protocol::http::router_handler_lambda;
}  // namespace

// +===========================================================================+
// | [>] handler concepts distinguish sync and async callbacks   ( test-case ) |
// +===========================================================================+
DOBA_TEST("handler concepts distinguish sync and async callbacks") {
  auto sync = [](const request&) {
    response res;
    return res;
  };
  auto async = [](std::shared_ptr<const request>,
                  std::stop_token) -> task<response> {
    co_return response{};
  };
  auto legacy = [](const request&, response&) {};
  static_assert(!router_handler_lambda<decltype(legacy)>);
  static_assert(router_handler_lambda<decltype(sync)>);
  static_assert(!router_async_handler_lambda<decltype(sync)>);
  static_assert(!router_handler_lambda<decltype(async)>);
  static_assert(router_async_handler_lambda<decltype(async)>);
  DOBA_EXPECT(true);
}
// +===========================================================================+
// | [>] mutable handler signatures retain their argument count  ( test-case ) |
// +===========================================================================+
DOBA_TEST("mutable handler signatures retain their argument count") {
  auto sync = [](const request&, int) mutable {
    return response{};
  };
  auto async = [](std::shared_ptr<const request>,
                           std::stop_token, int) mutable -> task<response> {
    co_return response{};
  };
  using namespace martianlabs::doba::protocol::http;
  static_assert(router_handler_lambda<decltype(sync)>);
  static_assert(router_async_handler_lambda<decltype(async)>);
  DOBA_EXPECT_EQUAL(
      router_handler_signature<decltype(&decltype(sync)::operator())>::
          parameter_count, 1);
  DOBA_EXPECT_EQUAL(
      router_async_handler_signature<decltype(&decltype(async)::operator())>::
          parameter_count, 1);
}
// +===========================================================================+
// | [>] noexcept sync handlers retain signature compatibility   ( test-case ) |
// +===========================================================================+
DOBA_TEST("noexcept sync handlers retain signature compatibility") {
  auto handler = [](const request&) noexcept {
    return response{};
  };
  DOBA_EXPECT(router_handler_lambda<decltype(handler)>);
}
// +===========================================================================+
// | [>] noexcept async handlers retain signature compatibility  ( test-case ) |
// +===========================================================================+
DOBA_TEST("noexcept async handlers retain signature compatibility") {
  auto handler = [](std::shared_ptr<const request>,
                     std::stop_token) noexcept -> task<response> {
    co_return response{};
  };
  DOBA_EXPECT(router_async_handler_lambda<decltype(handler)>);
}
// +===========================================================================+
// | [>] noexcept qualifiers preserve routing parameter counts   ( test-case ) |
// +===========================================================================+
DOBA_TEST("noexcept qualifiers preserve routing parameter counts") {
  auto sync = [](const request&, int) noexcept { return response{}; };
  auto mutable_sync = [](const request&, int) mutable noexcept {
    return response{};
  };
  auto async = [](std::shared_ptr<const request>,
                  std::stop_token, int) noexcept -> task<response> {
    co_return response{};
  };
  auto mutable_async = [](std::shared_ptr<const request>,
                          std::stop_token, int) mutable noexcept
      -> task<response> {
    co_return response{};
  };
  using namespace martianlabs::doba::protocol::http;
  static_assert(router_handler_lambda<decltype(sync)>);
  static_assert(router_handler_lambda<decltype(mutable_sync)>);
  static_assert(router_async_handler_lambda<decltype(async)>);
  static_assert(router_async_handler_lambda<decltype(mutable_async)>);
  DOBA_EXPECT_EQUAL(
      router_handler_signature<decltype(&decltype(sync)::operator())>::
          parameter_count, 1);
  DOBA_EXPECT_EQUAL(
      router_handler_signature<decltype(&decltype(mutable_sync)::operator())>::
          parameter_count, 1);
  DOBA_EXPECT_EQUAL(
      router_async_handler_signature<decltype(&decltype(async)::operator())>::
          parameter_count, 1);
  DOBA_EXPECT_EQUAL(
      router_async_handler_signature<
          decltype(&decltype(mutable_async)::operator())>::parameter_count, 1);
}
// +===========================================================================+
// | [>] noexcept handlers retain invalid signature rejection    ( test-case ) |
// +===========================================================================+
DOBA_TEST("noexcept handlers retain invalid signature rejection") {
  auto mutable_request = [](request&) noexcept { return response{}; };
  auto value_request = [](request) noexcept { return response{}; };
  auto no_result = [](const request&) noexcept {};
  auto no_task = [](std::shared_ptr<const request>, std::stop_token) noexcept {
    return response{};
  };
  auto no_cancellation = [](std::shared_ptr<const request>) noexcept
      -> task<response> {
    co_return response{};
  };
  DOBA_EXPECT(!router_handler_lambda<decltype(mutable_request)>);
  DOBA_EXPECT(!router_handler_lambda<decltype(value_request)>);
  DOBA_EXPECT(!router_handler_lambda<decltype(no_result)>);
  DOBA_EXPECT(!router_async_handler_lambda<decltype(no_task)>);
  DOBA_EXPECT(!router_async_handler_lambda<decltype(no_cancellation)>);
}
// +===========================================================================+
// | [>] sync signatures reject mutable and value requests       ( test-case ) |
// +===========================================================================+
DOBA_TEST("sync signatures reject mutable and value requests") {
  auto mutable_request = [](request&) { return response{}; };
  auto value_request = [](request) { return response{}; };
  auto rvalue_request = [](const request&&) { return response{}; };
  auto no_result = [](const request&) {};
  DOBA_EXPECT(!router_handler_lambda<decltype(mutable_request)>);
  DOBA_EXPECT(!router_handler_lambda<decltype(value_request)>);
  DOBA_EXPECT(!router_handler_lambda<decltype(rvalue_request)>);
  DOBA_EXPECT(!router_handler_lambda<decltype(no_result)>);
}

namespace {
struct signature_controller {
  response get(const request&, int) const noexcept { return {}; }
};
}  // namespace

// +===========================================================================+
// | [>] controller signature binding                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("bound controller methods retain argument and noexcept contracts") {
  using namespace martianlabs::doba::protocol::http;
  auto handler = router_handler_signature<decltype(&signature_controller::get)>
      ::bind(std::make_shared<signature_controller>(),
             &signature_controller::get);
  static_assert(router_handler_lambda<decltype(handler)>);
  static_assert(std::is_nothrow_invocable_v<decltype(handler), const request&,
                                           int>);
  static_assert(router_handler_signature<
      decltype(&decltype(handler)::operator())>
                    ::parameter_count == 1);
  DOBA_EXPECT(true);
}
