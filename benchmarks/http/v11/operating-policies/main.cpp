//                              _       _
//                           __| | ___ | |__   __ _
//                          / _` |/ _ \| '_ \ / _` |
//                         | (_| | (_) | |_) | (_| |
//                          \__,_|\___/|_.__/ \__,_|
//
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/
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

#include <atomic>
#include <cstdio>
#include <ctime>
#include <memory>
#include <mutex>
#include <thread>
#include "common/task.h"
#include "protocol/http/v11/engine.h"
#include "protocol/http/v11/request.h"
#include "protocol/http/v11/response.h"
using namespace martianlabs::doba;
using namespace martianlabs::doba::protocol::http::v11;
// Model the bundled transport pressure callback without socket I/O.
struct pressure_state {
  std::mutex mutex;
  std::size_t queries = 0;
};
template <class E>
void run(const char* route, E& e, std::shared_ptr<pressure_state> state) {
  std::size_t bytes = 0, wakes = 0, resumes = 0;
  e.set_on_send([&](auto h, auto b, auto) { bytes += h.size() + b.size(); });
  e.set_on_close([] {});
  if constexpr (requires(E& value) {
                  value.set_input_flow_control(std::function<bool()>{},
                                               std::function<void()>{});
                }) {
#ifdef PUBLISHED_PRESSURE
    auto published = std::make_shared<std::atomic<bool>>(false);
    e.set_input_flow_control(
        [published, counter = &state->queries] {
          ++*counter;
          return published->load(std::memory_order_acquire);
        },
        [&] { ++wakes; });
#else
    e.set_input_flow_control(
        [weak = std::weak_ptr(state)] {
          auto s = weak.lock();
          std::lock_guard lock(s->mutex);
          ++s->queries;
          return false;
        },
        [&] { ++wakes; });
#endif
  }
  const std::string wire =
      std::string("GET /") + route + " HTTP/1.1\r\nHost: a\r\n\r\n";
  constexpr std::size_t count = 1000000;
  auto start = std::clock();
  for (std::size_t i = 0; i < count; ++i) {
    e.on_bytes_received(wire);
    if constexpr (requires(E& value) { value.resume_input(); }) {
#ifdef OPTIMIZED_NATIVE
      if (e.input_paused()) {
        ++resumes;
        e.resume_input();
      }
#else
      ++resumes;
      e.resume_input();
#endif
    }
  }
  const double ns = 1e9 * double(std::clock() - start) / CLOCKS_PER_SEC / count;
  std::printf("%s %.3f bytes=%zu queries=%zu wakes=%zu resumes=%zu\n", route,
              ns, bytes, state->queries, wakes, resumes);
}
int main() {
  // Keep the standard library on its multithreaded synchronization paths, as
  // in a live server, without adding competing CPU work to the measurement.
  std::atomic<bool> done{false};
  std::jthread parked([&] { done.wait(false); });
  protocol::http::router<request, response> routes;
  routes.add("GET", "/baseline",
             [](const request&, response& r) { r.ok_200().set_body("hello"); });
  routes.add("GET", "/inline",
             [](const request&, response& r) -> common::task<void> {
               r.ok_200().set_body("hello");
               co_return;
             });
  for (int rep = 0; rep < 5; ++rep) {
    for (const char* name : {"baseline", "inline"}) {
      engine<request, response, decltype(routes)> e({}, routes);
      run(name, e, std::make_shared<pressure_state>());
    }
  }
  done.store(true);
  done.notify_one();
}
