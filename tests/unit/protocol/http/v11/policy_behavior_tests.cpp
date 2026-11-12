// Copyright 2026 martianLabs
// SPDX-License-Identifier: Apache-2.0

#include <array>
#include <limits>
#include <memory>
#include <span>
#include <string>
#include <string_view>

#include "engine_probe.h"
#include "test_helper.h"

namespace {
using namespace martianlabs::doba;
using probe = tests::unit::engine_probe;
using policies = probe::policies;

void check_request(policies limits, const std::string& wire, bool accepted,
                   std::string_view rejection = "HTTP/1.1 400 ") {
  for (const std::size_t fragment : {std::size_t{1}, wire.size() + 128}) {
    probe::router routes;
    unsigned calls = 0, successors = 0;
    routes.add("POST", "/", [&](const probe::request&, probe::response& res) {
      ++calls;
      res.ok_200().set_body("accepted");
    });
    routes.add("GET", "/sentinel",
               [&](const probe::request&, probe::response& res) {
                 ++successors;
                 res.ok_200().set_body("sentinel");
               });
    probe current(routes, limits);
    const auto input = wire + probe::get("/sentinel");
    for (std::size_t offset = 0; offset < input.size(); offset += fragment) {
      current.receive(std::string_view(input).substr(offset, fragment));
    }
    DOBA_EXPECT_EQUAL(calls, accepted ? 1U : 0U);
    DOBA_EXPECT_EQUAL(successors, accepted ? 1U : 0U);
    DOBA_EXPECT_EQUAL(current.closes, accepted ? 0 : 1);
    DOBA_EXPECT_EQUAL(current.outputs.size(), accepted ? 2U : 1U);
    if (accepted) {
      DOBA_EXPECT(current.outputs[0].ends_with("\r\n\r\naccepted"));
      DOBA_EXPECT(current.outputs[1].ends_with("\r\n\r\nsentinel"));
    } else {
      DOBA_EXPECT(current.outputs[0].starts_with(rejection));
      const auto output = current.wire;
      current.turn();
      current.receive(probe::get("/sentinel"));
      DOBA_EXPECT_EQUAL(current.wire, output);
      DOBA_EXPECT_EQUAL(successors, 0U);
      DOBA_EXPECT(!current.needs.turn_requested);
      DOBA_EXPECT(!current.needs.turn_at);
      DOBA_EXPECT(!current.needs.turn_after_output_drained);
    }
  }
}

std::string post(std::string_view headers, std::string_view body = {}) {
  return "POST / HTTP/1.1\r\nHost: local\r\n" + std::string(headers) + "\r\n" +
         std::string(body);
}
}  // namespace

DOBA_TEST(
    "maximum HTTP policy values do not wrap or preallocate their limits") {
  const auto maximum = std::numeric_limits<std::size_t>::max();
  policies limits;
  limits.max_request_body_size = maximum;
  limits.max_forwarding_hops = maximum;
  limits.max_transfer_codings = maximum;
  limits.max_pending_requests = maximum;
  limits.max_pending_response_bytes = maximum;
  limits.max_task_steps_per_turn = maximum;
  limits.max_deferred_input_size = maximum;
  limits.max_query_parameters = maximum;
  limits.max_chunk_extension_size = maximum;
  limits.max_trailer_section_size = maximum;
  limits.max_request_head_size = maximum;
  limits.request_body_memory_threshold = maximum;
  limits.max_response_head_size = maximum;
  limits.response_body_memory_threshold = maximum;
  check_request(limits,
                post("Via: 1.1 proxy\r\nTransfer-Encoding: gzip, chunked\r\n",
                     "1;a=b\r\nx\r\n0\r\nX: y\r\n\r\n"),
                true);
}

DOBA_TEST("body policies enforce raw and chunked boundaries before dispatch") {
  for (const std::size_t maximum : {0U, 1U, 5U}) {
    for (const std::size_t size : {0U, 1U, 4U, 5U, 6U}) {
      policies limits;
      limits.max_request_body_size = maximum;
      const std::string body(size, 'x');
      check_request(
          limits,
          post("Content-Length: " + std::to_string(size) + "\r\n", body),
          !maximum || size <= maximum, "HTTP/1.1 413 ");
      std::string chunks;
      for (std::size_t i = 0; i < size; ++i) chunks += "1\r\nx\r\n";
      check_request(
          limits, post("Transfer-Encoding: chunked\r\n", chunks + "0\r\n\r\n"),
          !maximum || size <= maximum, "HTTP/1.1 413 ");
    }
  }
}

DOBA_TEST("forwarding policy counts mixed repeated fields with custom limits") {
  const std::array<std::string_view, 4> fields{
      "Via: 1.1 first\r\n", "Forwarded: for=192.0.2.1\r\n",
      "X-Forwarded-For: 192.0.2.2\r\n", "Via: 1.1 last\r\n"};
  for (const std::size_t maximum : {0U, 1U, 3U}) {
    for (std::size_t count = 0; count <= fields.size(); ++count) {
      policies limits;
      limits.max_forwarding_hops = maximum;
      std::string headers;
      for (std::size_t i = 0; i < count; ++i) headers += fields[i];
      check_request(limits, post(headers), !maximum || count <= maximum);
    }
  }
}

DOBA_TEST("transfer coding policy counts lists and repeated fields equally") {
  for (const bool repeated : {false, true}) {
    for (const std::size_t maximum : {0U, 1U, 3U}) {
      for (const std::size_t count : {1U, 2U, 3U, 4U}) {
        policies limits;
        limits.max_transfer_codings = maximum;
        std::string headers = "Transfer-Encoding: ";
        for (std::size_t i = 1; i < count; ++i) {
          headers += repeated ? "gzip\r\nTransfer-Encoding: " : "gzip, ";
        }
        headers += "chunked\r\n";
        check_request(limits, post(headers, "0\r\n\r\n"),
                      !maximum || count <= maximum, "HTTP/1.1 501 ");
      }
    }
  }
}

DOBA_TEST("feature switches reject offers without blocking ordinary requests") {
  for (const bool enabled : {false, true}) {
    policies limits;
    limits.allow_chunked = enabled;
    limits.allow_upgrade = enabled;
    check_request(limits, post("Content-Length: 1\r\n", "x"), true);
    check_request(limits,
                  post("Transfer-Encoding: chunked\r\n", "1\r\nx\r\n0\r\n\r\n"),
                  enabled, "HTTP/1.1 501 ");
    check_request(limits, post("Connection: upgrade\r\nUpgrade: websocket\r\n"),
                  enabled, "HTTP/1.1 501 ");
  }
}

DOBA_TEST("query policy counts pairs rather than empty separators") {
  for (const std::size_t maximum : {0U, 1U, 3U}) {
    for (const std::size_t count : {1U, 2U, 3U, 4U}) {
      policies limits;
      limits.max_query_parameters = maximum;
      std::string wire = "POST /?&&";
      for (std::size_t i = 0; i < count; ++i) wire += "a=x&&";
      wire += " HTTP/1.1\r\nHost: local\r\n\r\n";
      check_request(limits, wire, !maximum || count <= maximum);
    }
  }
}

DOBA_TEST("chunk metadata policies preserve the rejection frontier") {
  for (const bool extension : {false, true}) {
    const std::string body =
        extension ? "1;a=b\r\nx\r\n0\r\n\r\n" : "0\r\nX: y\r\n\r\n";
    const std::size_t size = extension ? 4 : 8;
    for (const auto maximum : {std::size_t{0}, size - 1, size, size + 1}) {
      policies limits;
      if (extension) {
        limits.max_chunk_extension_size = maximum;
      } else {
        limits.max_trailer_section_size = maximum;
      }
      check_request(limits, post("Transfer-Encoding: chunked\r\n", body),
                    !maximum || size <= maximum);
    }
  }
}

DOBA_TEST("deferred input budget accumulates fragments and is reusable") {
  for (const bool overflow : {false, true}) {
    probe::router routes;
    unsigned calls = 0;
    routes.add(
        "GET", "/",
        [&](const probe::request&, probe::response& res) -> common::task<> {
          ++calls;
          co_await common::yield();
          res.ok_200().set_body("ok");
        });
    const auto wire = probe::get("/");
    policies limits;
    limits.max_pending_requests = 1;
    limits.max_deferred_input_size = wire.size();
    probe current(routes, limits);
    for (unsigned cycle = 0; cycle < 8; ++cycle) {
      current.receive(wire);
      for (const char byte : wire) current.receive(std::string_view(&byte, 1));
      DOBA_EXPECT_EQUAL(calls, cycle * 2 + 1);
      DOBA_EXPECT_EQUAL(current.closes, 0);
      if (overflow) {
        current.receive("G");
        DOBA_EXPECT_EQUAL(current.closes, 1);
        current.turn();
        DOBA_EXPECT(current.outputs.empty());
        DOBA_EXPECT_EQUAL(calls, 1U);
        break;
      }
      current.turn();
      current.turn();
      DOBA_EXPECT_EQUAL(current.outputs.size(), (cycle + 1) * 2);
      DOBA_EXPECT_EQUAL(current.closes, 0);
    }
  }
}

DOBA_TEST(
    "response inline threshold preserves exact bytes around the boundary") {
  for (const std::size_t threshold : {0U, 1U, 8U}) {
    for (const std::size_t size : {0U, 1U, 7U, 8U, 9U}) {
      probe::router routes;
      const std::string body(size, '\0');
      routes.add("GET", "/", [&](const probe::request&, probe::response& res) {
        res.ok_200().set_body(body);
      });
      policies limits;
      limits.response_body_memory_threshold = threshold;
      probe current(routes, limits);
      current.receive(probe::get("/"));
      DOBA_EXPECT_EQUAL(current.outputs.size(), 1U);
      DOBA_EXPECT_EQUAL(current.sources, size > threshold ? 1 : 0);
      DOBA_EXPECT(current.wire.ends_with("\r\n\r\n" + body));
      DOBA_EXPECT_EQUAL(current.closes, 0);
    }
  }
}

DOBA_TEST(
    "queued response bytes accumulate exactly and release after draining") {
  const std::string reply =
      "HTTP/1.1 200 OK\r\nDate: fixed\r\nContent-Length: 1\r\n\r\nx";
  for (const auto maximum :
       {reply.size() * 2 - 1, reply.size() * 2, reply.size() * 2 + 1}) {
    probe::router routes;
    auto owner = std::make_shared<const std::string>("x");
    routes.add("GET", "/wait",
               [&](const probe::request&, probe::response& res) {
                 res.ok_200()
                     .set_header("Date", "fixed")
                     .set_body(common::reader::shared(
                                   owner, std::as_bytes(std::span(*owner))),
                               1);
               });
    routes.add("GET", "/next", [](const probe::request&, probe::response& res) {
      res.ok_200().set_header("Date", "fixed").set_body("x");
    });
    policies limits;
    limits.max_pending_response_bytes = maximum;
    probe current(routes, limits);
    current.hold_source = true;
    const auto batch =
        probe::get("/wait") + probe::get("/next") + probe::get("/next");
    for (unsigned cycle = 0; cycle < 8; ++cycle) {
      current.receive(batch);
      DOBA_EXPECT_EQUAL(current.closes, maximum < reply.size() * 2 ? 1 : 0);
      current.drain();
      DOBA_EXPECT_EQUAL(owner.use_count(), 1);
      if (maximum < reply.size() * 2) {
        DOBA_EXPECT_EQUAL(current.outputs.size(), 1U);
        DOBA_EXPECT_EQUAL(current.wire, reply);
        break;
      }
      DOBA_EXPECT_EQUAL(current.outputs.size(), (cycle + 1) * 3);
      DOBA_EXPECT_EQUAL(current.wire.size(), (cycle + 1) * reply.size() * 3);
      std::string expected;
      for (unsigned i = 0; i < (cycle + 1) * 3; ++i) expected += reply;
      DOBA_EXPECT_EQUAL(current.wire, expected);
    }
  }
}

DOBA_TEST("HTTP policies keep bounded work and ordered output when combined") {
  constexpr unsigned requests = 24;
  for (const std::size_t window : {1U, 2U, 4U}) {
    for (const std::size_t budget : {1U, 3U}) {
      for (const std::size_t threshold : {0U, 8U, 1024U}) {
        probe::router routes;
        unsigned steps = 0;
        routes.add("GET", "/:id",
                   [&](const probe::request&, probe::response& res,
                       int id) -> common::task<> {
                     for (unsigned i = 0; i < 3; ++i) {
                       ++steps;
                       co_await common::yield();
                     }
                     ++steps;
                     res.ok_200()
                         .set_header("Date", "fixed")
                         .set_body(std::to_string(id));
                   });
        policies limits;
        limits.max_pending_requests = window;
        limits.max_task_steps_per_turn = budget;
        limits.max_pending_response_bytes = 256;
        limits.max_deferred_input_size = 2048;
        limits.response_body_memory_threshold = threshold;
        probe current(routes, limits);
        current.hold_source = true;
        std::string batch;
        for (unsigned i = 0; i < requests; ++i) {
          batch += probe::get("/" + std::to_string(i));
        }
        current.receive(batch);
        DOBA_EXPECT(steps <= budget);
        for (unsigned turn = 0;
             turn < 512 && (current.needs.turn_requested ||
                            current.needs.turn_after_output_drained);
             ++turn) {
          auto before = steps;
          current.turn(false);
          DOBA_EXPECT(steps - before <= budget);
          before = steps;
          current.drain();
          DOBA_EXPECT(steps - before <= budget);
        }
        DOBA_EXPECT_EQUAL(steps, requests * 4);
        DOBA_EXPECT_EQUAL(current.outputs.size(), requests);
        DOBA_EXPECT_EQUAL(current.closes, 0);
        DOBA_EXPECT(!current.held);
        DOBA_EXPECT(!current.needs.turn_requested);
        DOBA_EXPECT(!current.needs.turn_after_output_drained);
        std::string expected;
        for (unsigned i = 0; i < requests; ++i) {
          const auto body = std::to_string(i);
          expected += "HTTP/1.1 200 OK\r\nDate: fixed\r\nContent-Length: " +
                      std::to_string(body.size()) + "\r\n\r\n" + body;
        }
        DOBA_EXPECT_EQUAL(current.wire, expected);
      }
    }
  }
}
