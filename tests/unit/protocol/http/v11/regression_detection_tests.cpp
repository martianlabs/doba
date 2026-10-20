#include <string>

#include "../../../../helpers/concurrency.h"
#include "protocol/http/v11/request.h"
#include "protocol/http/v11/response.h"
#include "protocol/http/v11/decoder.h"
#include "protocol/http/v11/engine.h"
#include "test_helper.h"

namespace {
using namespace martianlabs::doba;
using namespace martianlabs::doba::protocol::http::v11;
using protocol::http::router;
constexpr std::string_view next = "GET /next HTTP/1.1\r\nHost: x\r\n\r\n";
}  // namespace

DOBA_TEST("detection rejects a body one byte above its configured limit") {
  policies configuration;
  configuration.max_request_body_size = 4;
  decoder<request, response> parser(configuration);
  const std::string wire = "POST / HTTP/1.1\r\nHost: x\r\nContent-Length: 5\r\n\r\n12345";
  std::string_view input(wire);
  const auto result = parser.deserialize(input);
  DOBA_EXPECT(result.code == protocol::deserialization_status::kInvalidSource);
  DOBA_EXPECT(!result.request.has_value());
  decoder<request, response> exact(configuration);
  const std::string allowed = "POST / HTTP/1.1\r\nHost: x\r\nContent-Length: 4\r\n\r\n1234";
  input = allowed;
  DOBA_EXPECT(exact.deserialize(input).code == protocol::deserialization_status::kSucceeded);
}

DOBA_TEST("detection queues a successor response until the first completes") {
  tests::manual_event pending;
  router<request, response> routes;
  routes.add("GET", "/first", [&](const request&, response& res) -> common::task<void> {
    co_await pending;
    res.ok_200().set_body("first");
  });
  routes.add("GET", "/next", [](const request&, response& res) {
    res.ok_200().set_body("second");
  });
  engine<request, response> current({}, routes);
  int sends = 0;
  std::string wire;
  current.set_on_send([&](std::string_view head, std::string_view body,
                           std::unique_ptr<common::reader>) {
    ++sends;
    wire.append(head).append(body);
  });
  current.set_on_close([] {});
  tests::scope_exit release([&] { pending.open(); });
  current.on_bytes_received("GET /first HTTP/1.1\r\nHost: x\r\n\r\n" + std::string(next));
  DOBA_EXPECT(pending.wait_for_waiters(1));
  DOBA_EXPECT_EQUAL(sends, 0);
  pending.open();
  DOBA_EXPECT_EQUAL(sends, 2);
  const auto first = wire.find("\r\n\r\nfirst");
  const auto second = wire.find("\r\n\r\nsecond");
  DOBA_EXPECT(first != std::string::npos && second != std::string::npos && first < second);
}

DOBA_TEST("detection never dispatches a successor after terminal rejection") {
  router<request, response> routes;
  int dispatched = 0, closes = 0;
  routes.add("GET", "/next", [&](const request&, response& res) {
    ++dispatched;
    res.ok_200().set_body("forbidden");
  });
  engine<request, response> current({}, routes);
  current.set_on_send([](std::string_view, std::string_view, std::unique_ptr<common::reader>) {});
  current.set_on_close([&] { ++closes; });
  current.on_bytes_received("GET /invalid HTTP/1.1\r\n\r\n" + std::string(next));
  DOBA_EXPECT_EQUAL(dispatched, 0);
  DOBA_EXPECT_EQUAL(closes, 1);
  current.on_bytes_received(next);
  DOBA_EXPECT_EQUAL(dispatched, 0);
  DOBA_EXPECT_EQUAL(closes, 1);
}
