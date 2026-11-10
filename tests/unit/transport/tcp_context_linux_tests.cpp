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

#include <array>
#include <cerrno>
#include <cstddef>
#include <functional>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

#include "protocol/processing_result.h"
#include "common/reader.h"
#include "test_helper.h"
#include "transport/tcp_linux_context.h"

namespace {
using martianlabs::doba::protocol::processing_result;
namespace common = martianlabs::doba::common;
namespace detail = martianlabs::doba::transport::detail;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] byte_engine                                                ( struct ) |
// +---------------------------------------------------------------------------+
// | Test engine follows the synchronous transport callback contract.          |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct byte_engine {
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using policies_type = int;
  // Test behavior is invoked only by on_bytes_received.
  std::function<void(byte_engine&, std::string_view)> action;
  // Output and closure callbacks supplied by the transport.
  martianlabs::doba::protocol::send_delegate send;
  std::function<void()> close;
  // +=========================================================================+
  // | [>] set_on_send                                              ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_send(martianlabs::doba::protocol::send_delegate callback) {
    send = std::move(callback);
  }
  // +=========================================================================+
  // | [>] set_on_close                                             ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_close(std::function<void()> callback) {
    close = std::move(callback);
  }
  // +=========================================================================+
  // | [>] on_bytes_received                                        ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_bytes_received(std::string_view bytes) {
    action(*this, bytes);
    return progress;
  }
  // +=========================================================================+
  // | [>] on_processing_turn                                       ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_processing_turn(bool output_idle) {
    return on_turn ? on_turn(*this, output_idle) : processing_result{};
  }
  // Turn snapshots and behavior are owned by the serialized engine.
  processing_result progress{};
  std::function<processing_result(byte_engine&, bool)> on_turn{};
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] socket_pair                                                 ( class ) |
// +---------------------------------------------------------------------------+
// | Nonblocking endpoints force partial sends without timing assumptions.     |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class socket_pair {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  socket_pair() {
    if (::socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, sockets.data())) {
      throw std::runtime_error("socketpair failed");
    }
    const int capacity = 4096;
    const char input = 'x';
    if (::setsockopt(sockets[0], SOL_SOCKET, SO_SNDBUF, &capacity,
                     sizeof(capacity)) != 0 ||
        ::send(sockets[1], &input, 1, MSG_NOSIGNAL) != 1) {
      ::close(sockets[0]);
      ::close(sockets[1]);
      throw std::runtime_error("socketpair setup failed");
    }
  }
  socket_pair(const socket_pair&) = delete;
  socket_pair& operator=(const socket_pair&) = delete;
  ~socket_pair() {
    ::close(sockets[0]);
    ::close(sockets[1]);
  }
  // +=========================================================================+
  // | [>] drain                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  std::string drain() {
    std::string result;
    std::array<char, 8192> bytes{};
    for (;;) {
      const auto size = ::recv(sockets[1], bytes.data(), bytes.size(), 0);
      if (size < 0 && errno == EINTR) continue;
      if (size < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) break;
      if (size <= 0) throw std::runtime_error("unexpected peer closure");
      result.append(bytes.data(), static_cast<std::size_t>(size));
    }
    return result;
  }
  // Endpoints are owned by the fixture, not by the context under test.
  std::array<int, 2> sockets{};
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] source                                                   ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
std::unique_ptr<common::reader> source(const std::string& bytes) {
  return std::make_unique<common::reader>(common::reader::borrowed(
      std::as_bytes(std::span(bytes.data(), bytes.size()))));
}
}  // namespace

// +===========================================================================+
// | [>] Linux batches bytes and preserves partial sends         ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux batches inline output and preserves partial sends") {
  const std::string body(256 * 1024, 'b');
  auto factory = [&]() {
    return byte_engine{[&](byte_engine& engine, std::string_view) {
      engine.send("first", {}, nullptr);
      engine.send("prefix", body, nullptr);
    }, {}, {}};
  };
  socket_pair peer;
  detail::linux_context<byte_engine> context(peer.sockets[0], 8,
                                             body.size() + 11, factory);
  DOBA_EXPECT(context.receive());
  DOBA_EXPECT(context.flush());
  DOBA_EXPECT(context.pending());
  std::string received = peer.drain();
  for (int turn = 0; context.pending() && turn < 1000; ++turn) {
    DOBA_EXPECT(context.flush());
    received += peer.drain();
  }
  DOBA_EXPECT(!context.pending());
  DOBA_EXPECT_EQUAL(received, "firstprefix" + body);
}

// +===========================================================================+
// | [>] Linux streams a source through bounded TX               ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux streams a source through bounded TX") {
  const std::string body(128 * 1024, 's');
  auto factory = [&]() {
    return byte_engine{[&](byte_engine& engine, std::string_view) {
      engine.send("before", {}, nullptr);
      engine.send("prefix", "body", source(body));
    }, {}, {}};
  };
  socket_pair peer;
  detail::linux_context<byte_engine> context(peer.sockets[0], 8, 32, factory);
  DOBA_EXPECT(context.receive());
  DOBA_EXPECT(context.flush());
  DOBA_EXPECT_EQUAL(peer.drain(), "beforeprefixbody");
  std::string received;
  for (int turn = 0; context.pending() && turn < 5000; ++turn) {
    DOBA_EXPECT(context.flush());
    received += peer.drain();
  }
  DOBA_EXPECT(!context.pending());
  DOBA_EXPECT_EQUAL(received, body);
}

// +===========================================================================+
// | [>] Linux accepts an empty source without output            ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux accepts an empty source without output") {
  const std::string empty;
  auto factory = [&]() {
    return byte_engine{[&](byte_engine& engine, std::string_view) {
      engine.send({}, {}, source(empty));
    }, {}, {}};
  };
  socket_pair peer;
  detail::linux_context<byte_engine> context(peer.sockets[0], 8, 1, factory);
  DOBA_EXPECT(context.receive());
  DOBA_EXPECT(context.flush());
  DOBA_EXPECT(!context.pending());
  DOBA_EXPECT(peer.drain().empty());
}

// +===========================================================================+
// | [>] Linux rejects output following a source                 ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux rejects output following a source") {
  const std::string body("source");
  auto factory = [&]() {
    return byte_engine{[&](byte_engine& engine, std::string_view) {
      engine.send("prefix", {}, source(body));
      engine.send("later", {}, nullptr);
    }, {}, {}};
  };
  socket_pair peer;
  detail::linux_context<byte_engine> context(peer.sockets[0], 8, 64, factory);
  DOBA_EXPECT(!context.receive());
  DOBA_EXPECT(!context.flush());
  DOBA_EXPECT(peer.drain().empty());
}

// +===========================================================================+
// | [>] Linux reports reader failure after its prefix           ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux reports reader failure after its prefix") {
  auto factory = []() {
    return byte_engine{[](byte_engine& engine, std::string_view) {
      auto reader = std::make_unique<common::reader>(common::filesystem{});
      std::byte byte{};
      reader->read(std::span<std::byte>(&byte, 1));
      engine.send("prefix", {}, std::move(reader));
    }, {}, {}};
  };
  socket_pair peer;
  detail::linux_context<byte_engine> context(peer.sockets[0], 8, 64, factory);
  DOBA_EXPECT(context.receive());
  DOBA_EXPECT(context.flush());
  DOBA_EXPECT_EQUAL(peer.drain(), "prefix");
  DOBA_EXPECT(!context.flush());
}

// +===========================================================================+
// | [>] Linux enforces the accumulated output limit             ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux enforces the accumulated output limit") {
  auto factory = []() {
    return byte_engine{[](byte_engine& engine, std::string_view) {
      engine.send("123", "45", nullptr);
      engine.send("6", {}, nullptr);
    }, {}, {}};
  };
  socket_pair peer;
  detail::linux_context<byte_engine> context(peer.sockets[0], 8, 5, factory);
  DOBA_EXPECT(!context.receive());
  DOBA_EXPECT(!context.flush());
  DOBA_EXPECT(peer.drain().empty());
}

// +===========================================================================+
// | [>] Linux releases sources on completion and cancellation   ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux releases sources on completion and cancellation") {
  for (const bool complete : {false, true}) {
    int released = 0;
    auto factory = [&]() {
      return byte_engine{[&](byte_engine& engine, std::string_view) {
        auto owner = std::shared_ptr<const std::string>(
            new std::string("data"),
            [&](const std::string* value) { ++released; delete value; });
        const auto bytes =
            std::as_bytes(std::span(owner->data(), owner->size()));
        auto reader = std::make_unique<common::reader>(
            common::reader::shared(owner, bytes));
        engine.send("prefix", {}, std::move(reader));
      }, {}, {}};
    };
    socket_pair peer;
    {
      detail::linux_context<byte_engine> context(
          peer.sockets[0], 8, 8, factory);
      DOBA_EXPECT(context.receive());
      DOBA_EXPECT_EQUAL(released, 0);
      if (complete) {
        DOBA_EXPECT(context.flush());
        DOBA_EXPECT_EQUAL(peer.drain(), "prefix");
        DOBA_EXPECT(context.flush());
        DOBA_EXPECT_EQUAL(peer.drain(), "data");
        DOBA_EXPECT_EQUAL(released, 1);
      }
    }
    DOBA_EXPECT_EQUAL(released, 1);
  }
}

// +===========================================================================+
// | [>] Linux preserves output across EAGAIN                    ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux preserves output across EAGAIN") {
  const std::string body(256 * 1024, 'q');
  auto factory = [&]() {
    return byte_engine{[&](byte_engine& engine, std::string_view) {
      engine.send("prefix", body, nullptr);
    }, {}, {}};
  };
  socket_pair peer;
  detail::linux_context<byte_engine> context(
      peer.sockets[0], 8, body.size() + 6, factory);
  DOBA_EXPECT(context.receive());
  bool blocked = false;
  for (int turn = 0; turn < 32 && !blocked; ++turn) {
    errno = 0;
    DOBA_EXPECT(context.flush());
    blocked = errno == EAGAIN || errno == EWOULDBLOCK;
  }
  DOBA_EXPECT(blocked);
  DOBA_EXPECT(context.pending());
  std::string received = peer.drain();
  for (int turn = 0; context.pending() && turn < 1000; ++turn) {
    DOBA_EXPECT(context.flush());
    received += peer.drain();
  }
  DOBA_EXPECT(!context.pending());
  DOBA_EXPECT_EQUAL(received, "prefix" + body);
}

// +===========================================================================+
// | [>] Linux reuses the exact TX budget after drain            ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux reuses the exact TX budget after drain") {
  auto factory = []() {
    return byte_engine{[](byte_engine& engine, std::string_view) {
      engine.send("abc", "de", nullptr);
    }, {}, {}};
  };
  socket_pair peer;
  detail::linux_context<byte_engine> context(peer.sockets[0], 8, 5, factory);
  for (int turn = 0; turn < 3; ++turn) {
    if (turn) {
      DOBA_EXPECT(::send(peer.sockets[1], "x", 1, MSG_NOSIGNAL) == 1);
    }
    DOBA_EXPECT(context.receive());
    DOBA_EXPECT(context.flush());
    DOBA_EXPECT(!context.pending());
    DOBA_EXPECT_EQUAL(peer.drain(), "abcde");
  }
}

// +===========================================================================+
// | [>] Linux accepts a new source after the previous drains    ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux accepts a new source after the previous drains") {
  const std::string body("source");
  auto factory = [&]() {
    return byte_engine{[&](byte_engine& engine, std::string_view) {
      engine.send("prefix", {}, source(body));
    }, {}, {}};
  };
  socket_pair peer;
  detail::linux_context<byte_engine> context(peer.sockets[0], 8, 8, factory);
  for (int turn = 0; turn < 3; ++turn) {
    if (turn) {
      DOBA_EXPECT(::send(peer.sockets[1], "x", 1, MSG_NOSIGNAL) == 1);
    }
    DOBA_EXPECT(context.receive());
    DOBA_EXPECT(context.flush());
    DOBA_EXPECT_EQUAL(peer.drain(), "prefix");
    DOBA_EXPECT(context.flush());
    DOBA_EXPECT(!context.pending());
    DOBA_EXPECT_EQUAL(peer.drain(), body);
  }
}

// +===========================================================================+
// | [>] Linux closes an empty terminal delivery                 ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux closes an empty terminal delivery") {
  const std::string empty;
  auto factory = [&]() {
    return byte_engine{[&](byte_engine& engine, std::string_view) {
      engine.send({}, {}, source(empty));
      engine.close();
    }, {}, {}};
  };
  socket_pair peer;
  detail::linux_context<byte_engine> context(peer.sockets[0], 8, 1, factory);
  DOBA_EXPECT(context.receive());
  DOBA_EXPECT(!context.flush());
  DOBA_EXPECT(!context.pending());
  DOBA_EXPECT(peer.drain().empty());
}

// +===========================================================================+
// | [>] Linux handles a broken output socket without SIGPIPE    ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux handles a broken output socket without SIGPIPE") {
  auto factory = []() {
    return byte_engine{[](byte_engine& engine, std::string_view) {
      engine.send("response", {}, nullptr);
    }, {}, {}};
  };
  socket_pair peer;
  DOBA_EXPECT(::close(peer.sockets[1]) == 0);
  peer.sockets[1] = -1;
  detail::linux_context<byte_engine> context(peer.sockets[0], 8, 16, factory);
  DOBA_EXPECT(context.receive());
  DOBA_EXPECT(!context.flush());
}

// +===========================================================================+
// | [>] empty sends preserve a terminal reader                  ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux empty sends preserve prefix source and exact TX capacity") {
  for (const std::string& body : {std::string{}, std::string(257, 's')}) {
    auto factory = [&]() {
      return byte_engine{[&](byte_engine& engine, std::string_view) {
        engine.send({}, {}, nullptr);
        engine.send("pre", {}, nullptr);
        engine.send({}, {}, nullptr);
        engine.send("h", "b", source(body));
        engine.send({}, {}, nullptr);
        engine.send({}, {}, nullptr);
      }, {}, {}};
    };
    socket_pair peer;
    detail::linux_context<byte_engine> context(peer.sockets[0], 8, 5, factory);
    DOBA_EXPECT(context.receive());
    std::string received;
    for (int turn = 0; context.pending() && turn < 128; ++turn) {
      DOBA_EXPECT(context.flush());
      received += peer.drain();
    }
    DOBA_EXPECT(!context.pending());
    DOBA_EXPECT_EQUAL(received, "prehb" + body);
  }
}

// +===========================================================================+
// | [>] empty sends do not permit another reader                ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux empty sends do not permit a second terminal reader") {
  const std::string body("source");
  auto factory = [&]() {
    return byte_engine{[&](byte_engine& engine, std::string_view) {
      engine.send("pre", {}, source(body));
      engine.send({}, {}, nullptr);
      engine.send({}, {}, source(body));
    }, {}, {}};
  };
  socket_pair peer;
  detail::linux_context<byte_engine> context(peer.sockets[0], 8, 5, factory);
  DOBA_EXPECT(!context.receive());
  DOBA_EXPECT(!context.flush());
  DOBA_EXPECT(peer.drain().empty());
}

// +===========================================================================+
// | [>] Requested turns are retained for the worker             ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux context retains each requested processing condition") {
  for (int condition = 0; condition < 3; ++condition) {
    socket_pair sockets;
    auto factory = [condition] {
      byte_engine engine;
      engine.action = [](byte_engine&, std::string_view bytes) {
        DOBA_EXPECT_EQUAL(bytes, "x");
      };
      if (condition == 0) engine.progress.turn_requested = true;
      if (condition == 1) {
        engine.progress.turn_at = processing_result::clock::time_point{};
      }
      if (condition == 2) engine.progress.turn_after_output_drained = true;
      return engine;
    };
    detail::linux_context<byte_engine> context(sockets.sockets[0], 8, 64,
                                               factory);
    DOBA_EXPECT(context.receive());
    DOBA_EXPECT_EQUAL(context.progress().turn_requested, condition == 0);
    DOBA_EXPECT_EQUAL(context.progress().turn_at.has_value(), condition == 1);
    DOBA_EXPECT_EQUAL(context.progress().turn_after_output_drained,
                      condition == 2);
    DOBA_EXPECT(context.process());
    DOBA_EXPECT(!context.progress().turn_requested);
    DOBA_EXPECT(!context.progress().turn_at);
    DOBA_EXPECT(!context.progress().turn_after_output_drained);
    DOBA_EXPECT(sockets.drain().empty());
  }
}

// +===========================================================================+
// | [>] Closing wins over requested scheduling                  ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux close retains output when a result also requests processing") {
  socket_pair sockets;
  auto factory = [] {
    byte_engine engine;
    engine.action = [](byte_engine& current, std::string_view) {
      current.send("ok", {}, nullptr);
      current.close();
    };
    engine.progress = {true, processing_result::clock::time_point{}, true};
    return engine;
  };
  detail::linux_context<byte_engine> context(sockets.sockets[0], 8, 64,
                                             factory);
  DOBA_EXPECT(context.receive());
  DOBA_EXPECT(!context.flush());
  DOBA_EXPECT_EQUAL(sockets.drain(), "ok");
}

DOBA_TEST("Linux processing callbacks can send only inside a later turn") {
  socket_pair sockets;
  int calls = 0;
  auto factory = [&] {
    byte_engine engine;
    engine.action = [](byte_engine&, std::string_view) {};
    engine.progress.turn_requested = true;
    engine.on_turn = [&](byte_engine& current, bool idle) {
      ++calls;
      if (idle) current.send("turn", {}, nullptr);
      current.close();
      return processing_result{true, {}, true};
    };
    return engine;
  };
  detail::linux_context<byte_engine> context(sockets.sockets[0], 8, 64,
                                             factory);
  DOBA_EXPECT(context.receive());
  DOBA_EXPECT_EQUAL(calls, 0);
  DOBA_EXPECT(context.process());
  DOBA_EXPECT_EQUAL(calls, 1);
  DOBA_EXPECT(!context.progress().turn_requested);
  DOBA_EXPECT(!context.flush());
  DOBA_EXPECT_EQUAL(sockets.drain(), "turn");
  DOBA_EXPECT(context.process());
  DOBA_EXPECT_EQUAL(calls, 1);
}
