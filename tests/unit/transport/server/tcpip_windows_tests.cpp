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

#ifdef _WIN32

#include <atomic>
#include <coroutine>
#include <utility>

#include "transport/server/tcpip.h"
#include "transport/server/tcpip_windows.h"
#include "test_helper.h"

namespace {
using namespace martianlabs::doba::transport::server;
using detail::executor_dispatcher;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] scheduled_probe                                             ( class ) |
// +---------------------------------------------------------------------------+
// | Internal implementation detail.                                           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class scheduled_probe {
 public:
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +=========================================================================+
  struct promise_type {
    scheduled_probe get_return_object() noexcept {
      return scheduled_probe(
          std::coroutine_handle<promise_type>::from_promise(*this));
    }
    std::suspend_always initial_suspend() const noexcept { return {}; }
    std::suspend_always final_suspend() const noexcept { return {}; }
    void return_void() const noexcept {}
    void unhandled_exception() const noexcept { std::terminate(); }
  };
  scheduled_probe(const scheduled_probe&) = delete;
  scheduled_probe(scheduled_probe&& in) noexcept
      : coroutine_(std::exchange(in.coroutine_, nullptr)) {}
  ~scheduled_probe() {
    if (coroutine_) coroutine_.destroy();
  }
  scheduled_probe& operator=(const scheduled_probe&) = delete;
  scheduled_probe& operator=(scheduled_probe&& in) noexcept {
    if (this == &in) return *this;
    if (coroutine_) coroutine_.destroy();
    coroutine_ = std::exchange(in.coroutine_, nullptr);
    return *this;
  }
  std::coroutine_handle<> get_coroutine() const noexcept { return coroutine_; }

 private:
  // +=========================================================================+
  // | [>] METHODs                                                 ( private ) |
  // +=========================================================================+
  explicit scheduled_probe(
      std::coroutine_handle<promise_type> coroutine) noexcept
      : coroutine_(coroutine) {}
  std::coroutine_handle<promise_type> coroutine_;
};

scheduled_probe increment(std::atomic<std::size_t>& value) {
  value.fetch_add(1, std::memory_order_relaxed);
  co_return;
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] wake_resource                                               ( class ) |
// +---------------------------------------------------------------------------+
// | Internal implementation detail.                                           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class wake_resource {
 public:
  // +=========================================================================+
  // | [>] METHODs                                                  ( public ) |
  // +=========================================================================+
  wake_resource()
      : handle_(CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 1)) {}
  ~wake_resource() {
    if (handle_ != nullptr) CloseHandle(handle_);
  }
  HANDLE get() const { return handle_; }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  HANDLE handle_;
};
}  // namespace

// +===========================================================================+
// | [>] dispatcher without a wake resource rejects scheduling   ( test-case ) |
// +===========================================================================+
DOBA_TEST("dispatcher without a wake resource rejects scheduling") {
  executor_dispatcher value{nullptr};
  std::atomic<std::size_t> count{0};
  auto continuation = increment(count);
  DOBA_EXPECT(!value.schedule(continuation.get_coroutine()));
  value.wake();
  value.stop();
  value.close();
  value.close();
  DOBA_EXPECT(!value.run());
  DOBA_EXPECT_EQUAL(count.load(), 0);
}
// +===========================================================================+
// | [>] dispatcher wakes and resumes queued work once           ( test-case ) |
// +===========================================================================+
DOBA_TEST("dispatcher wakes and resumes queued work once") {
  wake_resource resource;
  DOBA_EXPECT(resource.get() != nullptr);
  std::atomic<std::size_t> count{0};
  auto continuation = increment(count);
  executor_dispatcher value{resource.get()};
  DOBA_EXPECT(value.schedule(continuation.get_coroutine()));
  DOBA_EXPECT_EQUAL(count.load(), 0);
  DWORD bytes = 99;
  ULONG_PTR key = 0;
  OVERLAPPED* operation = nullptr;
  DOBA_EXPECT(GetQueuedCompletionStatus(resource.get(), &bytes, &key,
                                       &operation, 0) != FALSE);
  DOBA_EXPECT_EQUAL(bytes, 0);
  DOBA_EXPECT_EQUAL(key, kExecutorCompletionKey);
  DOBA_EXPECT(operation == nullptr);
  DOBA_EXPECT(!value.run());
  DOBA_EXPECT_EQUAL(count.load(), 1);
  DOBA_EXPECT(continuation.get_coroutine().done());
  DOBA_EXPECT(!value.run());
  DOBA_EXPECT_EQUAL(count.load(), 1);
}
// +===========================================================================+
// | [>] closed dispatchers retain work and reject additions     ( test-case ) |
// +===========================================================================+
DOBA_TEST("closed dispatchers retain queued work and reject additions") {
  wake_resource resource;
  DOBA_EXPECT(resource.get() != nullptr);
  std::atomic<std::size_t> count{0};
  auto first = increment(count);
  auto second = increment(count);
  executor_dispatcher value{resource.get()};
  DOBA_EXPECT(value.schedule(first.get_coroutine()));
  value.close();
  DOBA_EXPECT(!value.schedule(second.get_coroutine()));
  value.wake();
  DOBA_EXPECT(!value.run());
  DOBA_EXPECT_EQUAL(count.load(), 1);
  DOBA_EXPECT(first.get_coroutine().done());
  DOBA_EXPECT(!second.get_coroutine().done());
}
// +===========================================================================+
// | [>] stopped dispatchers drain work and reject additions     ( test-case ) |
// +===========================================================================+
DOBA_TEST("stopped dispatchers drain work and reject additions") {
  wake_resource resource;
  DOBA_EXPECT(resource.get() != nullptr);
  std::atomic<std::size_t> count{0};
  auto first = increment(count);
  auto second = increment(count);
  executor_dispatcher value{resource.get()};
  DOBA_EXPECT(value.schedule(first.get_coroutine()));
  value.stop();
  value.stop();
  DOBA_EXPECT(!value.schedule(second.get_coroutine()));
  DOBA_EXPECT(!value.run());
  DOBA_EXPECT_EQUAL(count.load(), 1);
  DOBA_EXPECT(first.get_coroutine().done());
  DOBA_EXPECT(!second.get_coroutine().done());
}
// +===========================================================================+
// | [>] overlapped accept initializes its native operation      ( test-case ) |
// +===========================================================================+
DOBA_TEST("overlapped accept initializes its native operation") {
  overlapped_accept value{INVALID_SOCKET};
  DOBA_EXPECT_EQUAL(value.get_type(), io_type::kAccept);
  DOBA_EXPECT_EQUAL(value.socket, INVALID_SOCKET);
  DOBA_EXPECT_EQUAL(value.Internal, 0);
  DOBA_EXPECT_EQUAL(value.InternalHigh, 0);
  DOBA_EXPECT_EQUAL(value.Offset, 0);
  DOBA_EXPECT_EQUAL(value.OffsetHigh, 0);
  DOBA_EXPECT(value.hEvent == nullptr);
  for (char byte : value.addresses) DOBA_EXPECT_EQUAL(byte, 0);
}

#endif
