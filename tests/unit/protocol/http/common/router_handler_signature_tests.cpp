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
#include <type_traits>

#include "protocol/http/common/router_handler_signature.h"
#include "router_empty_request.h"
#include "router_empty_response.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::task;
using martianlabs::doba::protocol::http::router_handler_lambda;
using martianlabs::doba::tests::unit::router_empty_request;
using martianlabs::doba::tests::unit::router_empty_response;
}  // namespace

// +===========================================================================+
// | [>] handler concepts require a request and response         ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("handler concepts require a request and response") {
  auto handler = [](const router_empty_request&, router_empty_response&)
      -> task<void> {
    co_return;
  };
  auto synchronous = [](const router_empty_request&,
                        router_empty_response&) {};
  static_assert(!router_handler_lambda<decltype(synchronous)>);
  auto legacy = [](const router_empty_request&) {
    return router_empty_response{};
  };
  static_assert(!router_handler_lambda<decltype(legacy)>);
  static_assert(router_handler_lambda<decltype(handler)>);
  DOBA_EXPECT(true);
}

// +===========================================================================+
// | [>] mutable handler signatures retain their argument count  ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("mutable handler signatures retain their argument count") {
  auto handler = [](const router_empty_request&, router_empty_response&,
                    int) mutable -> task<void> {
    co_return;
  };
  using namespace martianlabs::doba::protocol::http;
  static_assert(router_handler_lambda<decltype(handler)>);
  DOBA_EXPECT_EQUAL(
      router_handler_signature<
          decltype(&decltype(handler)::operator())>::parameter_count,
      1);
}

// +===========================================================================+
// | [>] noexcept task handlers retain signature compatibility   ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("noexcept task handlers retain signature compatibility") {
  auto handler = [](const router_empty_request&,
                    router_empty_response&) noexcept -> task<void> {
    co_return;
  };
  DOBA_EXPECT(router_handler_lambda<decltype(handler)>);
}

// +===========================================================================+
// | [>] noexcept qualifiers preserve routing parameter counts   ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("noexcept qualifiers preserve routing parameter counts") {
  auto handler = [](const router_empty_request&, router_empty_response&,
                    int) noexcept -> task<void> {
    co_return;
  };
  auto mutable_handler = [](const router_empty_request&, router_empty_response&,
                            int) mutable noexcept -> task<void> {
    co_return;
  };
  using namespace martianlabs::doba::protocol::http;
  static_assert(router_handler_lambda<decltype(handler)>);
  static_assert(router_handler_lambda<decltype(mutable_handler)>);
  DOBA_EXPECT_EQUAL(
      router_handler_signature<
          decltype(&decltype(handler)::operator())>::parameter_count,
      1);
  DOBA_EXPECT_EQUAL(
      router_handler_signature<
          decltype(&decltype(mutable_handler)::operator())>::parameter_count,
      1);
}

// +===========================================================================+
// | [>] noexcept handlers retain invalid signature rejection    ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("noexcept handlers retain invalid signature rejection") {
  auto mutable_request = [](router_empty_request&,
                            router_empty_response&) noexcept -> task<void> {
    co_return;
  };
  auto value_request = [](router_empty_request,
                          router_empty_response&) noexcept -> task<void> {
    co_return;
  };
  auto no_result = [](const router_empty_request&,
                      router_empty_response&) noexcept {
    return router_empty_response{};
  };
  DOBA_EXPECT(!router_handler_lambda<decltype(mutable_request)>);
  DOBA_EXPECT(!router_handler_lambda<decltype(value_request)>);
  DOBA_EXPECT(!router_handler_lambda<decltype(no_result)>);
}

// +===========================================================================+
// | [>] task signatures reject mutable and value requests       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("task signatures reject mutable and value requests") {
  auto mutable_request = [](router_empty_request&, router_empty_response&)
      -> task<void> {
    co_return;
  };
  auto value_request = [](router_empty_request, router_empty_response&)
      -> task<void> {
    co_return;
  };
  auto rvalue_request = [](const router_empty_request&&,
                           router_empty_response&) -> task<void> {
    co_return;
  };
  auto no_result = [](const router_empty_request&, router_empty_response&) {
    return router_empty_response{};
  };
  DOBA_EXPECT(!router_handler_lambda<decltype(mutable_request)>);
  DOBA_EXPECT(!router_handler_lambda<decltype(value_request)>);
  DOBA_EXPECT(!router_handler_lambda<decltype(rvalue_request)>);
  DOBA_EXPECT(!router_handler_lambda<decltype(no_result)>);
}

namespace {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] signature_controller                                       ( struct ) |
// +---------------------------------------------------------------------------+
// | Controller implementation.                                                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct signature_controller {
  // +=========================================================================+
  // | [>] get                                                      ( public ) |
  // +-------------------------------------------------------------------------+
  task<void> get(const router_empty_request&, router_empty_response&,
           int) const noexcept {
    co_return;
  }
};
}  // namespace

// +===========================================================================+
// | [>] controller signature binding                            ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("bound controller methods retain argument and noexcept contracts") {
  using namespace martianlabs::doba::protocol::http;
  auto handler =
      router_handler_signature<decltype(&signature_controller::get)>::bind(
          std::make_shared<signature_controller>(), &signature_controller::get);
  static_assert(router_handler_lambda<decltype(handler)>);
  static_assert(std::is_nothrow_invocable_v<decltype(handler),
                                            const router_empty_request&,
                                            router_empty_response&, int>);
  static_assert(
      router_handler_signature<
          decltype(&decltype(handler)::operator())>::parameter_count == 1);
  DOBA_EXPECT(true);
}
