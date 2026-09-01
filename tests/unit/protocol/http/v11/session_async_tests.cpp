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

#include <cstddef>
#include <cstdint>
#include <deque>
#include <future>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "protocol/http/common/router.h"
#include "protocol/http/common/target.h"
#include "protocol/http/v11/response.h"
#include "protocol/http/v11/session.h"
#include "test_helper.h"
#include "transport/server/response_scheduler.h"

namespace {
using martianlabs::doba::protocol::http::router;
using martianlabs::doba::protocol::http::target;
using martianlabs::doba::protocol::http::v11::response;
using martianlabs::doba::protocol::http::v11::session;
using martianlabs::doba::protocol::serialization_result;
using martianlabs::doba::transport::server::detail::response_scheduler;

struct request {
  std::string_view get_method() const { return method; }
  target get_target() const { return target::kOriginForm; }
  std::string_view get_absolute_path() const { return path; }
  bool wants_connection_close() const { return close; }

  std::string_view method{"GET"};
  std::string_view path{"/"};
  bool close{false};
};

struct deferred_context {
  struct sender {
    sender() = default;
    sender(response_scheduler* scheduler, uint64_t position)
        : scheduler{scheduler}, position{position} {}
    sender(const sender&) = delete;
    sender(sender&& other) noexcept
        : scheduler{std::exchange(other.scheduler, nullptr)},
          position{other.position} {}
    sender& operator=(const sender&) = delete;
    sender& operator=(sender&&) noexcept = delete;

    bool complete(std::unique_ptr<serialization_result> value) {
      if (!scheduler || !value) return false;
      response_scheduler* target = std::exchange(scheduler, nullptr);
      return target->complete(position, std::move(value));
    }

    response_scheduler* scheduler{nullptr};
    uint64_t position{0};
  };

  sender defer() {
    deferrals++;
    return sender{&scheduler, scheduler.reserve()};
  }

  std::vector<std::unique_ptr<serialization_result>> take_ready() {
    std::vector<std::unique_ptr<serialization_result>> ready;
    while (!scheduler.empty() && scheduler.front().ready()) {
      ready.push_back(std::move(scheduler.front().response));
      scheduler.pop_front();
    }
    return ready;
  }

  response_scheduler scheduler;
  std::size_t deferrals{0};
};

class executor {
 public:
  template <typename FNty>
  bool try_submit(FNty&& fn) {
    submissions++;
    if (!accepting) return false;
    tasks.emplace_back(std::forward<FNty>(fn));
    return true;
  }

  void run(std::size_t index = 0) {
    std::packaged_task<void()> task = std::move(tasks[index]);
    tasks.erase(tasks.begin() + static_cast<std::ptrdiff_t>(index));
    task();
  }

  std::deque<std::packaged_task<void()>> tasks;
  std::size_t submissions{0};
  bool accepting{true};
};

using test_router = router<request, response>;
using test_session = session<request, response, test_router>;
}  // namespace

// +===========================================================================+
// | [>] synchronous route avoids async machinery                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("synchronous route avoids async machinery") {
  test_router routes;
  routes.add("GET", "/", [](const request&, response& res) {
    res.ok_200().set_body("sync");
  });
  auto req = std::make_shared<request>();
  response res;
  deferred_context deferred;
  executor work;
  test_session value;
  value.dispatch(req, res, routes, deferred, work);
  DOBA_EXPECT_EQUAL(deferred.deferrals, 0);
  DOBA_EXPECT_EQUAL(work.submissions, 0);
  DOBA_EXPECT(res.serialize()->prefix.ends_with("sync"));
}
// +===========================================================================+
// | [>] asynchronous route retains request and completes         ( test-case ) |
// +===========================================================================+
DOBA_TEST("asynchronous route retains request and completes") {
  test_router routes;
  routes.add_async("GET", "/", [](const request&, response& res) {
    res.ok_200().set_body("async");
  });
  auto req = std::make_shared<request>();
  std::weak_ptr<request> retained = req;
  response res;
  deferred_context deferred;
  executor work;
  test_session value;
  value.dispatch(req, res, routes, deferred, work);
  req.reset();
  DOBA_EXPECT(!retained.expired());
  DOBA_EXPECT_EQUAL(deferred.deferrals, 1);
  DOBA_EXPECT_EQUAL(work.submissions, 1);
  work.run();
  DOBA_EXPECT(retained.expired());
  auto ready = deferred.take_ready();
  DOBA_EXPECT_EQUAL(ready.size(), 1);
  DOBA_EXPECT(ready.front()->prefix.ends_with("async"));
}
// +===========================================================================+
// | [>] asynchronous route shapes preserve precedence           ( test-case ) |
// +===========================================================================+
DOBA_TEST("asynchronous route shapes preserve precedence") {
  test_router routes;
  routes.add_async("GET", "/items/*", [](const request&, response& res) {
    res.ok_200().set_body("wildcard");
  });
  routes.add_async("GET", "/items/:id",
                   [](const request&, response& res, int id) {
                     res.ok_200().set_body(std::to_string(id));
                   });
  routes.add("GET", "/items/42", [](const request&, response& res) {
    res.ok_200().set_body("exact");
  });
  test_session value;
  {
    auto req = std::make_shared<request>();
    req->path = "/items/42";
    response res;
    deferred_context deferred;
    executor work;
    value.dispatch(req, res, routes, deferred, work);
    DOBA_EXPECT_EQUAL(deferred.deferrals, 0);
    DOBA_EXPECT_EQUAL(work.submissions, 0);
    DOBA_EXPECT(res.serialize()->prefix.ends_with("exact"));
  }
  {
    auto req = std::make_shared<request>();
    req->path = "/items/7";
    std::weak_ptr<request> retained = req;
    response res;
    deferred_context deferred;
    executor work;
    value.dispatch(req, res, routes, deferred, work);
    req.reset();
    DOBA_EXPECT(!retained.expired());
    work.run();
    DOBA_EXPECT(retained.expired());
    auto ready = deferred.take_ready();
    DOBA_EXPECT_EQUAL(ready.size(), 1);
    DOBA_EXPECT(ready.front()->prefix.ends_with("7"));
  }
  {
    auto req = std::make_shared<request>();
    req->path = "/items/name";
    response res;
    deferred_context deferred;
    executor work;
    value.dispatch(req, res, routes, deferred, work);
    work.run();
    auto ready = deferred.take_ready();
    DOBA_EXPECT_EQUAL(ready.size(), 1);
    DOBA_EXPECT(ready.front()->prefix.ends_with("wildcard"));
  }
}
// +===========================================================================+
// | [>] asynchronous rejection and exception produce errors     ( test-case ) |
// +===========================================================================+
DOBA_TEST("asynchronous rejection and exception produce errors") {
  test_session value;
  {
    test_router routes;
    routes.add_async("GET", "/", [](const request&, response&) {});
    auto req = std::make_shared<request>();
    response res;
    deferred_context deferred;
    executor work;
    work.accepting = false;
    value.dispatch(req, res, routes, deferred, work);
    auto ready = deferred.take_ready();
    DOBA_EXPECT_EQUAL(ready.size(), 1);
    DOBA_EXPECT(ready.front()->prefix.starts_with(
        "HTTP/1.1 503 Service Unavailable\r\n"));
  }
  {
    test_router routes;
    routes.add_async("GET", "/items/:id", [](const request&, response&, int) {
      throw std::runtime_error("handler failure");
    });
    auto req = std::make_shared<request>();
    req->path = "/items/7";
    response res;
    deferred_context deferred;
    executor work;
    value.dispatch(req, res, routes, deferred, work);
    work.run();
    auto ready = deferred.take_ready();
    DOBA_EXPECT_EQUAL(ready.size(), 1);
    DOBA_EXPECT(ready.front()->prefix.starts_with(
        "HTTP/1.1 500 Internal Server Error\r\n"));
  }
}
// +===========================================================================+
// | [>] asynchronous responses preserve HTTP and scheduler order ( test-case ) |
// +===========================================================================+
DOBA_TEST("asynchronous responses preserve HTTP and scheduler order") {
  test_router routes;
  routes.add_async("HEAD", "/slow", [](const request&, response& res) {
    res.ok_200().set_body("slow");
  });
  routes.add_async("GET", "/fast", [](const request&, response& res) {
    res.ok_200().set_body("fast");
  });
  deferred_context deferred;
  executor work;
  test_session value;
  auto slow = std::make_shared<request>();
  slow->method = "HEAD";
  slow->path = "/slow";
  slow->close = true;
  response slow_res;
  value.dispatch(slow, slow_res, routes, deferred, work);
  auto fast = std::make_shared<request>();
  fast->path = "/fast";
  response fast_res;
  value.dispatch(fast, fast_res, routes, deferred, work);
  work.run(1);
  DOBA_EXPECT(deferred.take_ready().empty());
  work.run();
  auto ready = deferred.take_ready();
  DOBA_EXPECT_EQUAL(ready.size(), 2);
  DOBA_EXPECT(ready[0]->prefix.find("Connection: close\r\n") !=
              std::string_view::npos);
  DOBA_EXPECT(ready[0]->prefix.find("Content-Length: 4\r\n") !=
              std::string_view::npos);
  DOBA_EXPECT(!ready[0]->prefix.ends_with("slow"));
  DOBA_EXPECT(ready[1]->prefix.ends_with("fast"));
}
