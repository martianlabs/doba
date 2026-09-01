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

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <thread>
#include <vector>

#include "test_helper.h"
#include "transport/server/completion_mailbox.h"

#ifdef __linux__
#include <cerrno>
#include <sys/eventfd.h>
#include <unistd.h>
#endif

namespace {
using martianlabs::doba::transport::server::detail::completion_mailbox;
using martianlabs::doba::transport::server::detail::response_completion;

struct test_waker {
  bool operator()() const noexcept {
    calls->fetch_add(1, std::memory_order_relaxed);
    return succeeds->load(std::memory_order_relaxed);
  }

  std::atomic<std::size_t>* calls;
  std::atomic<bool>* succeeds;
};

#ifdef __linux__
struct eventfd_waker {
  bool operator()() const noexcept {
    uint64_t wake = 1;
    ssize_t written = 0;
    do {
      written = ::write(fd, &wake, sizeof(wake));
    } while (written == -1 && errno == EINTR);
    return written == sizeof(wake) ||
           (written == -1 && errno == EAGAIN);
  }

  int fd;
};
#endif
}  // namespace

// +===========================================================================+
// | [>] mailbox coalesces wakeups and preserves commands        ( test-case ) |
// +===========================================================================+
DOBA_TEST("mailbox coalesces wakeups and preserves commands") {
  std::atomic<std::size_t> calls{0};
  std::atomic<bool> succeeds{true};
  completion_mailbox<test_waker> mailbox{
      test_waker{&calls, &succeeds}};
  auto publisher = mailbox.get_publisher();

  DOBA_EXPECT(publisher.publish(response_completion{3, 7, nullptr}));
  DOBA_EXPECT(publisher.publish(response_completion{3, 8, nullptr}));
  DOBA_EXPECT_EQUAL(calls.load(), 1);

  auto completions = mailbox.drain();
  DOBA_EXPECT_EQUAL(completions.size(), 2);
  DOBA_EXPECT_EQUAL(completions[0].connection_key, 3);
  DOBA_EXPECT_EQUAL(completions[0].position, 7);
  DOBA_EXPECT_EQUAL(completions[1].position, 8);
}
// +===========================================================================+
// | [>] mailbox accepts concurrent producers                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("mailbox accepts concurrent producers") {
  constexpr std::size_t kThreadCount = 4;
  constexpr std::size_t kCommandsPerThread = 1024;
  constexpr std::size_t kCommandCount =
      kThreadCount * kCommandsPerThread;
  std::atomic<std::size_t> calls{0};
  std::atomic<bool> succeeds{true};
  completion_mailbox<test_waker> mailbox{
      test_waker{&calls, &succeeds}};
  auto publisher = mailbox.get_publisher();
  std::vector<std::thread> threads;
  threads.reserve(kThreadCount);
  for (std::size_t thread = 0; thread < kThreadCount; thread++) {
    threads.emplace_back([publisher, thread]() {
      std::size_t first = thread * kCommandsPerThread;
      for (std::size_t i = 0; i < kCommandsPerThread; i++) {
        if (!publisher.publish(
                response_completion{5, first + i, nullptr})) {
          return;
        }
      }
    });
  }
  for (auto& thread : threads) thread.join();

  auto completions = mailbox.drain();
  DOBA_EXPECT_EQUAL(completions.size(), kCommandCount);
  std::vector<uint64_t> positions;
  positions.reserve(completions.size());
  for (const auto& completion : completions) {
    positions.emplace_back(completion.position);
  }
  std::sort(positions.begin(), positions.end());
  for (std::size_t i = 0; i < positions.size(); i++) {
    DOBA_EXPECT_EQUAL(positions[i], i);
  }
}
// +===========================================================================+
// | [>] mailbox rejects failed wake and closed publication      ( test-case ) |
// +===========================================================================+
DOBA_TEST("mailbox rejects failed wake and closed publication") {
  std::atomic<std::size_t> calls{0};
  std::atomic<bool> succeeds{false};
  completion_mailbox<test_waker> mailbox{
      test_waker{&calls, &succeeds}};
  auto publisher = mailbox.get_publisher();

  DOBA_EXPECT(!publisher.publish(response_completion{1, 0, nullptr}));
  DOBA_EXPECT(mailbox.drain().empty());
  succeeds.store(true);
  DOBA_EXPECT(publisher.publish(response_completion{1, 1, nullptr}));
  mailbox.close();
  DOBA_EXPECT(!publisher.publish(response_completion{1, 2, nullptr}));
  DOBA_EXPECT(mailbox.drain().empty());
}
// +===========================================================================+
// | [>] publisher does not retain mailbox                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("publisher does not retain mailbox") {
  std::atomic<std::size_t> calls{0};
  std::atomic<bool> succeeds{true};
  completion_mailbox<test_waker>::publisher publisher;
  {
    completion_mailbox<test_waker> mailbox{
        test_waker{&calls, &succeeds}};
    publisher = mailbox.get_publisher();
  }
  DOBA_EXPECT(!publisher.publish(response_completion{1, 0, nullptr}));
}

#ifdef _WIN32
// +===========================================================================+
// | [>] IOCP mailbox publishes and revokes completions          ( test-case ) |
// +===========================================================================+
DOBA_TEST("IOCP mailbox publishes and revokes completions") {
  using martianlabs::doba::transport::server::detail::
      iocp_completion_mailbox;
  using martianlabs::doba::transport::server::detail::
      iocp_response_completion;
  using martianlabs::doba::transport::server::detail::
      kResponseCompletionKey;
  HANDLE port = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 1);
  DOBA_EXPECT(port != nullptr);
  iocp_completion_mailbox mailbox;
  mailbox.open(port);
  auto publisher = mailbox.get_publisher();
  DOBA_EXPECT(publisher.publish(response_completion{11, 17, nullptr}));

  DWORD bytes = 0;
  ULONG_PTR key = 0;
  LPOVERLAPPED overlapped = nullptr;
  DOBA_EXPECT(GetQueuedCompletionStatus(
                  port, &bytes, &key, &overlapped, 1000) == TRUE);
  DOBA_EXPECT_EQUAL(key, kResponseCompletionKey);
  auto packet = reinterpret_cast<iocp_response_completion*>(overlapped);
  DOBA_EXPECT_EQUAL(packet->completion.connection_key, 11);
  DOBA_EXPECT_EQUAL(packet->completion.position, 17);
  delete packet;

  mailbox.close();
  DOBA_EXPECT(!publisher.publish(response_completion{11, 18, nullptr}));
  CloseHandle(port);
}
#endif

#ifdef __linux__
// +===========================================================================+
// | [>] eventfd wakes mailbox consumer                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("eventfd wakes mailbox consumer") {
  int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  DOBA_EXPECT(fd != -1);
  completion_mailbox<eventfd_waker> mailbox{eventfd_waker{fd}};
  auto publisher = mailbox.get_publisher();
  DOBA_EXPECT(publisher.publish(response_completion{13, 19, nullptr}));

  uint64_t wake = 0;
  DOBA_EXPECT_EQUAL(::read(fd, &wake, sizeof(wake)), sizeof(wake));
  DOBA_EXPECT_EQUAL(wake, 1);
  auto completions = mailbox.drain();
  DOBA_EXPECT_EQUAL(completions.size(), 1);
  DOBA_EXPECT_EQUAL(completions.front().connection_key, 13);
  DOBA_EXPECT_EQUAL(completions.front().position, 19);

  mailbox.close();
  ::close(fd);
}
#endif
