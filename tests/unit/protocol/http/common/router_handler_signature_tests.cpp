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
#include "test_helper.h"

namespace {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] request                                                    ( struct ) |
// +---------------------------------------------------------------------------+
// | Test message representation.                                              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct request {};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] response                                                   ( struct ) |
// +---------------------------------------------------------------------------+
// | Test message representation.                                              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct response {};
using martianlabs::doba::protocol::http::router_handler_lambda;
}  // namespace

// +===========================================================================+
// | [>] handler concepts require a request and response         ( test-case ) |
// +===========================================================================+
DOBA_TEST("handler concepts require a request and response") {
  auto sync = [](const request&) {
    response res;
    return res;
  };
  auto legacy = [](const request&, response&) {};
  static_assert(!router_handler_lambda<decltype(legacy)>);
  static_assert(router_handler_lambda<decltype(sync)>);
  DOBA_EXPECT(true);
}
// +===========================================================================+
// | [>] mutable handler signatures retain their argument count  ( test-case ) |
// +===========================================================================+
DOBA_TEST("mutable handler signatures retain their argument count") {
  auto sync = [](const request&, int) mutable {
    return response{};
  };
  using namespace martianlabs::doba::protocol::http;
  static_assert(router_handler_lambda<decltype(sync)>);
  DOBA_EXPECT_EQUAL(
      router_handler_signature<decltype(&decltype(sync)::operator())>::
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
// | [>] noexcept qualifiers preserve routing parameter counts   ( test-case ) |
// +===========================================================================+
DOBA_TEST("noexcept qualifiers preserve routing parameter counts") {
  auto sync = [](const request&, int) noexcept { return response{}; };
  auto mutable_sync = [](const request&, int) mutable noexcept {
    return response{};
  };
  using namespace martianlabs::doba::protocol::http;
  static_assert(router_handler_lambda<decltype(sync)>);
  static_assert(router_handler_lambda<decltype(mutable_sync)>);
  DOBA_EXPECT_EQUAL(
      router_handler_signature<decltype(&decltype(sync)::operator())>::
          parameter_count, 1);
  DOBA_EXPECT_EQUAL(
      router_handler_signature<decltype(&decltype(mutable_sync)::operator())>::
          parameter_count, 1);
}
// +===========================================================================+
// | [>] noexcept handlers retain invalid signature rejection    ( test-case ) |
// +===========================================================================+
DOBA_TEST("noexcept handlers retain invalid signature rejection") {
  auto mutable_request = [](request&) noexcept { return response{}; };
  auto value_request = [](request) noexcept { return response{}; };
  auto no_result = [](const request&) noexcept {};
  DOBA_EXPECT(!router_handler_lambda<decltype(mutable_request)>);
  DOBA_EXPECT(!router_handler_lambda<decltype(value_request)>);
  DOBA_EXPECT(!router_handler_lambda<decltype(no_result)>);
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
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] signature_controller                                       ( struct ) |
// +---------------------------------------------------------------------------+
// | Controller implementation.                                                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
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
