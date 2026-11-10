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

#include <chrono>
#include <cstddef>
#include <functional>
#include <iostream>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>

#include "processing_session.h"
#include "protocol/http/v11/engine.h"
#include "protocol/http/v11/request.h"
#include "protocol/http/v11/response.h"

namespace {
using namespace martianlabs::doba;
using protocol::processing_result;
using clock_type = processing_result::clock;
using tests::tls::processing_session;
using namespace std::chrono_literals;
namespace tr = transport;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] require                                                  ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] turn_engine                                                ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct turn_engine {
  // +=========================================================================+
  // | [>] USINGs                                                  ( public  ) |
  // +-------------------------------------------------------------------------+
  using policies_type = int;
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( public  ) |
  // +-------------------------------------------------------------------------+
  protocol::send_delegate send;
  std::function<void()> close;
  std::function<processing_result(turn_engine&, std::string_view)> input;
  std::function<processing_result(turn_engine&, bool)> turn;
  // +=========================================================================+
  // | [>] set_on_send                                             ( public  ) |
  // +-------------------------------------------------------------------------+
  void set_on_send(protocol::send_delegate value) { send = std::move(value); }
  // +=========================================================================+
  // | [>] set_on_close                                            ( public  ) |
  // +-------------------------------------------------------------------------+
  void set_on_close(std::function<void()> value) { close = std::move(value); }
  // +=========================================================================+
  // | [>] on_bytes_received                                       ( public  ) |
  // +-------------------------------------------------------------------------+
  processing_result on_bytes_received(std::string_view bytes) {
    return input(*this, bytes);
  }
  // +=========================================================================+
  // | [>] on_processing_turn                                      ( public  ) |
  // +-------------------------------------------------------------------------+
  processing_result on_processing_turn(bool idle) { return turn(*this, idle); }
};

// +===========================================================================+
// | [>] source                                                   ( function ) |
// +---------------------------------------------------------------------------+
std::unique_ptr<common::reader> source(std::string text) {
  auto owner = std::make_shared<const std::string>(std::move(text));
  return std::make_unique<common::reader>(common::reader::shared(
      owner, std::as_bytes(std::span(*owner))));
}

// +===========================================================================+
// | [>] same                                                     ( function ) |
// +---------------------------------------------------------------------------+
bool same(const processing_result& a, const processing_result& b) {
  return a.turn_requested == b.turn_requested && a.turn_at == b.turn_at &&
      a.turn_after_output_drained == b.turn_after_output_drained;
}

// +===========================================================================+
// | [>] snapshots                                                ( function ) |
// +---------------------------------------------------------------------------+
void snapshots(tr::tls_policies settings, int version) {
  for (unsigned interests : {1U, 2U, 4U, 7U}) {
    processing_result initial{(interests & 1) != 0, {}, (interests & 4) != 0};
    if (interests & 2) initial.turn_at = clock_type::time_point::max();
    const processing_result next{true, clock_type::time_point::max(), true};
    unsigned calls = 0;
    auto factory = [&] {
      turn_engine engine;
      engine.input = [&](auto& current, auto bytes) {
        require(bytes == "x", "input changed or was not bounded");
        current.send("rx", {}, nullptr);
        return initial;
      };
      engine.turn = [&](auto& current, bool idle) {
        require(idle == (calls != 0), "output-idle snapshot changed");
        ++calls;
        if (calls == 1) {
          auto owner = std::make_shared<const std::string>("tail");
          current.send("pre:", "body:", std::make_unique<common::reader>(
              common::reader::shared(owner, std::as_bytes(std::span(*owner)))));
          require(owner.use_count() == 1, "TLS retained an exhausted reader");
          current.send({}, {}, nullptr);
          return next;
        }
        current.send("end:", {}, source("done"));
        current.close();
        return next;
      };
      return engine;
    };
    processing_session<turn_engine> session(settings, factory, version);
    session.write("x", 1);
    require(same(session.progress(), initial), "input snapshot lost");
    if (version == TLS1_3_VERSION) {
      session.update_key();
      require(same(session.progress(), initial), "control RX erased interest");
    }
    require(calls == 0, "TLS recursively invoked a processing turn");
    session.turn(false);
    require(!session.closed() && same(session.progress(), next),
            "turn snapshot lost while output busy");
    require(session.read() == "rxpre:body:tail", "prefix/source not encoded");
    session.turn(true);
    require(session.closed() && same(session.progress(), {}),
            "close did not override scheduling");
    require(session.read() == "end:done", "close lost final source bytes");
  }
}

// +===========================================================================+
// | [>] errors                                                   ( function ) |
// +---------------------------------------------------------------------------+
void errors(tr::tls_policies settings, int version) {
  for (unsigned failure = 0; failure < 9; ++failure) {
    auto limits = settings;
    limits.max_buffered_send_bytes = 4096;
    if (failure == 5) limits.max_buffered_send_bytes = 65536;
    if (failure == 6) {
      limits.max_buffered_send_bytes = 16384;
      limits.max_buffered_ciphertext_bytes = 8192;
    }
    auto factory = [failure] {
      turn_engine engine;
      engine.input = [](auto&, auto) {
        return processing_result{true, {}, false};
      };
      engine.turn = [failure](auto& current, bool) {
        if (failure < 2) current.send({}, std::string(4096 + failure, 'a'), {});
        else if (failure < 4) {
          current.send({}, {}, source(std::string(4094 + failure, 'b')));
        } else if (failure == 4) {
          current.send("discard", {}, source(""));
          current.send({}, {}, source(""));
        } else if (failure == 5) {
          current.send(std::string(20000, 'd'), {}, nullptr);
          throw std::runtime_error("handler failure");
        } else if (failure == 6) {
          current.send({}, {}, source(std::string(9000, 'c')));
        } else if (failure == 7) {
          common::filesystem file;
          std::byte byte{};
          file.read(std::span<std::byte>(&byte, 1));
          current.send("discard", {},
                       std::make_unique<common::reader>(std::move(file)));
        } else {
          current.send({}, std::string(2048, 'e'), nullptr);
          current.send({}, std::string(2049, 'e'), nullptr);
        }
        return processing_result{};
      };
      return engine;
    };
    processing_session<turn_engine> session(limits, factory, version);
    session.write("x");
    session.turn(true);
    const bool accepted = failure == 0 || failure == 2;
    require(session.closed() != accepted, "turn limit/failure not enforced");
    const auto result = session.read();
    require(accepted ? result == std::string(4096, failure == 0 ? 'a' : 'b')
                     : result.empty(), "partial failed turn escaped");
    require(same(session.progress(), {}), "failed turn left scheduling");
  }
}

// +===========================================================================+
// | [>] peer_close                                               ( function ) |
// +---------------------------------------------------------------------------+
void peer_close(tr::tls_policies settings, int version) {
  auto factory = [] {
    turn_engine engine;
    engine.input = [](auto& current, auto) {
      current.send("ok", {}, nullptr);
      return processing_result{true, clock_type::time_point::max(), true};
    };
    engine.turn = [](auto&, bool) -> processing_result {
      throw std::runtime_error("unexpected recursive close turn");
    };
    return engine;
  };
  processing_session<turn_engine> session(settings, factory, version);
  session.write("x");
  require(session.read() == "ok", "pending close lost output");
  session.shutdown();
  require(session.closed() && same(session.progress(), {}),
          "peer close_notify did not withdraw scheduling");
}

// +===========================================================================+
// | [>] concurrent_http                                          ( function ) |
// +---------------------------------------------------------------------------+
void concurrent_http(tr::tls_policies settings, int version) {
  using namespace protocol::http::v11;
  protocol::http::router<request, response> routes;
  unsigned started = 0, completed = 0;
  bool progressed_busy = false;
  routes.add("GET", "/wait/:id", [&](const request&, response& result,
                                      int id) -> common::task<> {
    ++started;
    co_await common::yield();
    ++completed;
    if (id == 2) progressed_busy = true;
    result.ok_200().set_body(std::to_string(id));
  });
  routes.add("GET", "/source", [](const request&, response& result) {
    result.ok_200().set_body(std::move(*source("source")), 6);
  });
  routes.add("GET", "/sync", [](const request&, response& result) {
    result.ok_200().set_body("sync");
  });
  auto factory = [&] { return engine<request, response>({}, routes); };
  processing_session<engine<request, response>> session(settings, factory,
                                                       version);
  const std::string batch =
      "GET /source HTTP/1.1\r\nHost: local\r\n\r\n"
      "GET /wait/1 HTTP/1.1\r\nHost: local\r\n\r\n"
      "GET /sync HTTP/1.1\r\nHost: local\r\n\r\n"
      "GET /wait/2 HTTP/1.1\r\nHost: local\r\n\r\n";
  session.write(batch, 1);
  require(started == 2, "source blocked sibling admission");
  require(session.pending() != 0, "source did not produce held ciphertext");
  session.turn(false);
  require(completed == 2 && progressed_busy, "busy TLS output stalled tasks");
  std::string wire = session.read();
  for (unsigned i = 0; i < 8 && (session.progress().turn_requested ||
       session.progress().turn_after_output_drained); ++i) {
    session.turn(true);
    wire += session.read();
  }
  require(!session.closed(), "mixed TLS engine closed unexpectedly");
  std::size_t position = 0;
  for (const auto body : {"source", "1", "sync", "2"}) {
    const auto found = wire.find(std::string("\r\n\r\n") + body, position);
    require(found != std::string::npos, "mixed TLS response order changed");
    position = found + 4 + std::char_traits<char>::length(body);
  }
}
}  // namespace

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main(int count, char** args) {
  if (count != 3) return 1;
  try {
    unsigned cases = 0;
    for (int version : {TLS1_2_VERSION, TLS1_3_VERSION}) {
      for (std::size_t capacity : {1U, 127U, 17408U}) {
        tr::tls_policies policies;
        policies.certificate_file = args[1];
        policies.private_key_file = args[2];
        policies.receive_buffer_capacity = 7;
        policies.network_bio_buffer_capacity = capacity;
        snapshots(policies, version);
        errors(policies, version);
        concurrent_http(policies, version);
        peer_close(policies, version);
        cases += 15;
      }
    }
    std::cout << "TLS processing cases: " << cases << " PASS\n";
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}


