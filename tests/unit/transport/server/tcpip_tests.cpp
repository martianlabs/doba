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

#include <coroutine>
#include <memory>
#include <stop_token>
#include <utility>
#include <variant>

#include "transport/server/tcpip.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::transport::server::detail::detached_operation;
using martianlabs::doba::transport::server::detail::resume_on;
using martianlabs::doba::transport::server::types;

detached_operation increment(std::shared_ptr<int> count) {
  ++*count;
  co_return;
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] scheduler                                                  ( struct ) |
// +---------------------------------------------------------------------------+
// | Internal implementation detail.                                           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct scheduler {
  bool accepted = false;
  std::size_t calls = 0;
  std::coroutine_handle<> received{};

  bool schedule(std::coroutine_handle<> continuation) {
    calls++;
    received = continuation;
    return accepted;
  }
};
}  // namespace

// +===========================================================================+
// | [>] detached operation destroys an unstarted frame          ( test-case ) |
// +===========================================================================+
DOBA_TEST("detached operation destroys an unstarted frame") {
  auto count = std::make_shared<int>(0);
  const std::weak_ptr<int> lifetime = count;
  {
    auto value = increment(count);
    DOBA_EXPECT(value.get_coroutine() != nullptr);
    DOBA_EXPECT(!value.get_coroutine().done());
    DOBA_EXPECT_EQUAL(*count, 0);
    count.reset();
    DOBA_EXPECT(!lifetime.expired());
  }
  DOBA_EXPECT(lifetime.expired());
}
// +===========================================================================+
// | [>] detached operation moves replace the owned frame        ( test-case ) |
// +===========================================================================+
DOBA_TEST("detached operation moves replace the owned frame") {
  auto source_count = std::make_shared<int>(0);
  auto previous_count = std::make_shared<int>(0);
  const std::weak_ptr<int> source_lifetime = source_count;
  const std::weak_ptr<int> previous_lifetime = previous_count;
  {
    auto source = increment(source_count);
    const auto handle = source.get_coroutine();
    auto moved = std::move(source);
    DOBA_EXPECT(!source.get_coroutine());
    DOBA_EXPECT(moved.get_coroutine() == handle);
    auto destination = increment(previous_count);
    previous_count.reset();
    destination = std::move(moved);
    DOBA_EXPECT(previous_lifetime.expired());
    DOBA_EXPECT(!moved.get_coroutine());
    DOBA_EXPECT(destination.get_coroutine() == handle);
    auto& same = destination;
    destination = std::move(same);
    DOBA_EXPECT(destination.get_coroutine() == handle);
    source_count.reset();
    DOBA_EXPECT(!source_lifetime.expired());
  }
  DOBA_EXPECT(source_lifetime.expired());
}
// +===========================================================================+
// | [>] released operations complete and destroy their frame    ( test-case ) |
// +===========================================================================+
DOBA_TEST("released operations complete and destroy their frame") {
  auto count = std::make_shared<int>(0);
  auto value = increment(count);
  const auto handle = value.get_coroutine();
  value.release();
  DOBA_EXPECT(!value.get_coroutine());
  DOBA_EXPECT_EQUAL(count.use_count(), 2);
  handle.resume();
  DOBA_EXPECT_EQUAL(*count, 1);
  DOBA_EXPECT_EQUAL(count.use_count(), 1);
  value.release();
  DOBA_EXPECT(!value.get_coroutine());
}
// +===========================================================================+
// | [>] resume on reports scheduler acceptance and rejection    ( test-case ) |
// +===========================================================================+
DOBA_TEST("resume on reports scheduler acceptance and rejection") {
  for (bool accepted : {false, true}) {
    auto target = std::make_shared<scheduler>();
    target->accepted = accepted;
    resume_on<scheduler> value{target};
    DOBA_EXPECT(!value.await_ready());
    DOBA_EXPECT(!value.await_resume());
    const auto continuation = std::noop_coroutine();
    DOBA_EXPECT_EQUAL(value.await_suspend(continuation), accepted);
    DOBA_EXPECT_EQUAL(value.await_resume(), accepted);
    DOBA_EXPECT_EQUAL(target->calls, 1);
    DOBA_EXPECT(target->received == continuation);
  }
}
// +===========================================================================+
// | [>] resume on does not retain an expired scheduler          ( test-case ) |
// +===========================================================================+
DOBA_TEST("resume on does not retain an expired scheduler") {
  auto target = std::make_shared<scheduler>();
  const std::weak_ptr<scheduler> lifetime = target;
  resume_on<scheduler> value{target};
  target.reset();
  DOBA_EXPECT(lifetime.expired());
  DOBA_EXPECT(!value.await_ready());
  DOBA_EXPECT(!value.await_suspend(std::noop_coroutine()));
  DOBA_EXPECT(!value.await_resume());
}
// +===========================================================================+
// | [>] transport delegates preserve callback arguments         ( test-case ) |
// +===========================================================================+
DOBA_TEST("transport delegates preserve callback arguments") {
  auto request = std::make_shared<int>(42);
  std::stop_source stop;
  stop.request_stop();
  std::shared_ptr<int> received;
  bool stopped = false;
  types::on_request_delegate<int, int> on_request =
      [&](const std::shared_ptr<int>& input, const std::stop_token& token) {
        received = input;
        stopped = token.stop_requested();
        return *input + 1;
      };
  auto response = on_request(request, stop.get_token());
  DOBA_EXPECT(received == request);
  DOBA_EXPECT(stopped);
  DOBA_EXPECT_EQUAL(response.index(), 0);
  DOBA_EXPECT_EQUAL(std::get<0>(response), 43);
  int status = 0;
  std::string_view reason;
  types::on_bad_request_delegate<int> on_bad_request =
      [&](int code, std::string_view message) {
        status = code;
        reason = message;
        return code;
      };
  DOBA_EXPECT_EQUAL(on_bad_request(400, "invalid"), 400);
  DOBA_EXPECT_EQUAL(status, 400);
  DOBA_EXPECT_EQUAL(reason, "invalid");
  std::size_t connected = 0;
  std::size_t disconnected = 0;
  types::on_client_connected_delegate on_connection = [&] { connected++; };
  types::on_client_disconnected_delegate on_disconnection =
      [&] { disconnected++; };
  on_connection();
  on_disconnection();
  DOBA_EXPECT_EQUAL(connected, 1);
  DOBA_EXPECT_EQUAL(disconnected, 1);
}
