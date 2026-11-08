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
// Copyright 2026 martianLabs
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

#include "../../helpers/tool_peer.h"
#include "../../helpers/tool_json.h"
#include "../../helpers/tool_process.h"
#include "../../helpers/tool_response.h"

#include <algorithm>
#include <barrier>
#include <future>
#include <numeric>
#ifdef __linux__
#include <sched.h>
#endif

using namespace martianlabs::doba::tests::tools;

static std::string post(const std::string& path, const std::string& body,
                         bool chunked = false) {
  std::string head = "POST " + path + " HTTP/1.1\r\nHost: local\r\n";
  if (!chunked) return head + "Content-Length: " +
    std::to_string(body.size()) + "\r\n\r\n" + body;
  std::ostringstream size;
  size << std::hex << body.size();
  return head + "Transfer-Encoding: chunked\r\n\r\n" + size.str() +
    "\r\n" + body + "\r\n0\r\n\r\n";
}

static double delay(uint16_t port, int milliseconds,
                    std::barrier<>* barrier = nullptr) {
  std::unique_ptr<peer> connection;
  try {
    connection = std::make_unique<peer>(port);
    connection->timeout(15s);
  } catch (...) {
    if (barrier) barrier->arrive_and_drop();
    throw;
  }
  if (barrier) barrier->arrive_and_wait();
  auto start = clock::now();
  connection->send(request("/delay/" + std::to_string(milliseconds)));
  auto value = response(*connection, true);
  auto elapsed = clock::now() - start;
  require(value.status == 200 && value.body == std::to_string(milliseconds) &&
    value.headers.at("content-type") == "text/plain", "Dynamic delay mismatch");
  require(elapsed >= std::chrono::milliseconds(milliseconds),
    "Dynamic delay completed before its deadline");
  return std::chrono::duration<double>(elapsed).count();
}

static void verify(uint16_t port) {
  std::random_device entropy;
  std::mt19937 random(entropy());
  std::uniform_int_distribution<int> integer(-100000, 100000);
  {
    peer connection(port);
    for (int mode = 0; mode < 3; ++mode) {
      for (int index = 0; index < 9; ++index) {
        int a = integer(random), b = integer(random), c = integer(random);
        auto path = "/baseline11?a=" + std::to_string(a) +
          "&b=" + std::to_string(b);
        connection.send(mode == 0 ? request(path) :
          post(path, std::to_string(c), mode == 2));
        expect_response(connection, 200,
          std::to_string(a + b + (mode ? c : 0)), true);
      }
    }
    for (char byte : post("/baseline11?a=17&b=29", "31", true)) {
      connection.send(std::string_view(&byte, 1));
    }
    expect_response(connection, 200, "77", true);
    std::string pipeline;
    for (int index = 0; index < 16; ++index) pipeline += request("/pipeline");
    connection.send(pipeline);
    for (int index = 0; index < 16; ++index) {
      auto value = response(connection, true);
      require(value.status == 200 && value.body == "ok" &&
        value.headers.at("content-type") == "text/plain", "Pipeline mismatch");
    }
    for (auto raw : {request("/baseline11?a=9223372036854775807&b=1"),
      request("/baseline11?a=-9223372036854775808&b=-1"),
      post("/baseline11?a=9223372036854775807&b=0", "1"),
      post("/baseline11?a=1&b=2", "not-an-integer"), request("/delay/-1"),
      request("/delay/60001"), request("/delay/not-a-number")}) {
      connection.send(raw);
      require(response(connection, true).status == 400,
        "Invalid driver input was accepted");
    }
    auto start = clock::now();
    connection.send(request("/delay/90") + request("/baseline11?a=11&b=23") +
      request("/delay/10") + request("/pipeline"));
    for (auto expected : {"90", "34", "10", "ok"}) {
      expect_response(connection, 200, expected, true);
    }
    require(clock::now() - start >= 90ms, "Ordered batch completed too early");
  }
  delay(port, 0);
  std::vector<int> values(81);
  std::iota(values.begin(), values.end(), 10);
  std::shuffle(values.begin(), values.end(), random);
  for (int index = 0; index < 4; ++index) delay(port, values[index]);
  double short_delay = 1000;
  for (int index = 0; index < 3; ++index) {
    short_delay = std::min(short_delay, delay(port, 10));
  }
  require(delay(port, 500) - short_delay >= .3, "Fixed delay detected");
  std::vector<int> concurrent(400);
  std::iota(concurrent.begin(), concurrent.end(), 100);
  std::shuffle(concurrent.begin(), concurrent.end(), random);
  std::barrier barrier(33);
  std::vector<std::future<double>> futures;
  futures.reserve(32);
  // Reach the barrier even if connecting failed.
  try {
    for (int index = 0; index < 32; ++index) {
      futures.push_back(std::async(std::launch::async, [&, index] {
        return delay(port, concurrent[index], &barrier);
      }));
    }
  } catch (...) {
    // Release peers before unwinding waits for their futures.
    for (size_t missing = futures.size(); missing <= 32; ++missing) {
      barrier.arrive_and_drop();
    }
    throw;
  }
  barrier.arrive_and_wait();
  for (auto& future : futures) future.get();
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
  return guarded([&] {
    require(argc == 3 || (argc == 4 && std::string(argv[3]) == "--plain"),
      "Expected HttpArena executable, metadata and optional --plain");
    static_assert(clock::is_steady);
    require(std::chrono::duration<double>(clock::duration(1)).count() <= .0001,
      "A monotonic high-resolution clock is required");
    auto metadata = json::load(argv[2]);
    std::set<std::string> names;
    for (const auto& name : metadata["tests"].items()) {
      names.insert(name.string());
    }
    require(names == std::set<std::string>{"baseline", "pipelined",
      "limited-conn", "async", "latency-10k", "latency-1m",
      "latency-500k-8cpu", "json-comp", "json-tls", "8gbit", "static-tls"},
      "HttpArena profile inventory changed");
    for (auto key : {"WORKERS", "PORT"}) {
      for (auto value : std::string(key) == "WORKERS" ?
        std::vector<std::string>{"0", "-1", "x"} :
        std::vector<std::string>{"0", "65536", "x"}) {
        process::environment environment{{"WORKERS", "1"}, {"PORT", "8080"}};
        environment[key] = value;
        auto result = run({argv[1]}, 5s, environment);
        clean_diagnostics(result.errors);
        require(result.code == 1 &&
          result.errors.find(key) != std::string::npos,
          "Invalid driver environment was accepted");
      }
    }
    auto exercise = [&](int workers, bool automatic) {
      temporary_directory directory;
      auto folder = directory.path();
      write(folder / "dataset.json", "[]");
      fs::create_directory(folder / "static");
      if (argc == 3) {
        auto result = run({"openssl", "req", "-x509", "-newkey", "rsa:2048",
          "-nodes", "-keyout", (folder / "server.key").string(), "-out",
          (folder / "server.crt").string(), "-days", "1", "-subj",
          "/CN=localhost"}, 20s);
        require(result.code == 0, "Cannot generate driver test certificate");
      }
      auto port = peer::available_port();
      auto tls_port = peer::available_port();
      while (tls_port == port) tls_port = peer::available_port();
      process::environment environment{{"PORT", std::to_string(port)},
        {"TLS_PORT", std::to_string(tls_port)},
        {"DATASET_PATH", (folder / "dataset.json").string()},
        {"STATIC_DIR", (folder / "static").string()},
        {"TLS_CERT", (folder / "server.crt").string()},
        {"TLS_KEY", (folder / "server.key").string()}};
      if (!automatic) environment["WORKERS"] = std::to_string(workers);
      process child({argv[1]}, environment,
        automatic ? std::vector<std::string>{"WORKERS"} :
          std::vector<std::string>{});
      auto deadline = clock::now() + 10s;
      for (;;) {
        require(!child.poll(), "Driver exited: " + child.errors());
        try { peer probe(port); break; }
        catch (const std::exception&) {
          require(clock::now() < deadline, "Driver startup timeout");
          std::this_thread::sleep_for(20ms);
        }
      }
      verify(port);
      require(!child.poll(), "Driver exited during validation");
      child.terminate(); child.wait(5s);
      clean_diagnostics(child.errors());
      require(child.errors().find("workers per listener: " +
        std::to_string(workers) + ';') != std::string::npos,
        "Effective driver worker count differs: " + child.errors());
    };
    exercise(1, false);
#ifdef __linux__
    cpu_set_t original, limited;
    CPU_ZERO(&limited);
    require(sched_getaffinity(0, sizeof(original), &original) == 0,
      "Cannot read test CPU affinity");
    int count = 0;
    for (int index = 0; index < CPU_SETSIZE && count < 2; ++index) {
      if (CPU_ISSET(index, &original)) { CPU_SET(index, &limited); ++count; }
    }
    require(count > 0 && sched_setaffinity(0, sizeof(limited), &limited) == 0,
      "Cannot limit driver affinity");
    try { exercise(count, true); }
    catch (...) { sched_setaffinity(0, sizeof(original), &original); throw; }
    require(sched_setaffinity(0, sizeof(original), &original) == 0,
      "Cannot restore test CPU affinity");
#endif
    std::cout << "HttpArena baseline, pipeline and dynamic async: PASS\n";
  });
}
