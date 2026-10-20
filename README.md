<a href="resources/branding/doba.png#gh-light-mode-only">
  <img src="resources/branding/doba.png" alt="doba" width="640">
</a>
<a href="resources/branding/doba-dark.png#gh-dark-mode-only">
  <img src="resources/branding/doba-dark.png" alt="doba" width="640">
</a>
<br><br>

<a href="resources/readme/readme-hero.svg#gh-light-mode-only">
  <img src="resources/readme/readme-hero.svg" alt="You write the handler. doba moves the bytes." width="540">
</a>
<a href="resources/readme/readme-hero-dark.svg#gh-dark-mode-only">
  <img src="resources/readme/readme-hero-dark.svg" alt="You write the handler. doba moves the bytes." width="540">
</a>
<br><br>

[![Build & Test](https://github.com/martianlabs/doba/actions/workflows/ci.yml/badge.svg?branch=main&event=push)](https://github.com/martianlabs/doba/actions/workflows/ci.yml?query=branch%3Amain)

**Meet doba:** a header-only C++20 server framework. Protocols and transports
mind their own business. HTTP/1.1 comes first; the architecture leaves room
for more.

**Native IOCP on Windows. Native epoll on Linux.**
The TCP transport needs no external libraries; optional TLS uses OpenSSL.

[Try the examples](examples/README.md)

<a name="okay-how-fast"></a>
<h2>
  <a href="resources/readme/readme-performance.svg#gh-light-mode-only">
    <img src="resources/readme/readme-performance.svg" alt="Okay, how fast?" width="280">
  </a>
  <a href="resources/readme/readme-performance-dark.svg#gh-dark-mode-only">
    <img src="resources/readme/readme-performance-dark.svg" alt="Okay, how fast?" width="280">
  </a>
</h2>

Big throughput numbers are fun. Waiting for a response isn't. Let's look at both.

Measured on **Scaleway, 2026-10-20**, using the official HttpArena suite:
native **gcannon** and **wrk**, with **zrk** and servers in Docker, following
upstream CPU affinity and host tuning. **RUNS=3**. Results are selected by
upstream from its repetitions, not averaged across runs.
The campaign's reference-model verification is **incomplete**; these are
local results, not the published HttpArena leaderboard.

Every chart shows the **same top 10 from the overall ranking**, ordered by each
test's metric. Scores retain normalization across all **15 measured servers**.
The total includes eight scored profiles and the upstream completeness factor;
**pipelined**, **latency-500k-8cpu** and **static-tls** are reference-only profiles.
Failed and skipped measurements are shown explicitly; they are not measured zeroes.

The test machine uses an **AMD EPYC 8534P at 2.30 GHz**
(**64 cores / 128 threads**), **576 GiB of DDR5 RAM** and
**2 x 3.84 TB NVMe** storage. It provides a **25 Gbps private network**
and a **1 Gbps public connection**, upgradable to **25 Gbps**.

<a href="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-overall.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-overall.svg" alt="HttpArena overall top 10: total points including completeness; higher is better." width="100%">
</a>
<a href="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-overall.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-overall-dark.svg" alt="HttpArena overall top 10: total points including completeness; higher is better." width="100%">
</a>
<br><br>

<a href="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-baseline.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-baseline.svg" alt="HttpArena baseline: selected requests per second; higher is better." width="100%">
</a>
<a href="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-baseline.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-baseline-dark.svg" alt="HttpArena baseline: selected requests per second; higher is better." width="100%">
</a>
<br><br>

<a href="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-pipelined.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-pipelined.svg" alt="HttpArena pipelined: selected requests per second; higher is better." width="100%">
</a>
<a href="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-pipelined.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-pipelined-dark.svg" alt="HttpArena pipelined: selected requests per second; higher is better." width="100%">
</a>
<br><br>

<a href="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-limited-conn.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-limited-conn.svg" alt="HttpArena limited-conn: selected requests per second; higher is better." width="100%">
</a>
<a href="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-limited-conn.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-limited-conn-dark.svg" alt="HttpArena limited-conn: selected requests per second; higher is better." width="100%">
</a>
<br><br>

<a href="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-latency-10k.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-latency-10k.svg" alt="HttpArena latency-10k: upstream fixed-rate score, p99 latency, achieved rate and status." width="100%">
</a>
<a href="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-latency-10k.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-latency-10k-dark.svg" alt="HttpArena latency-10k: upstream fixed-rate score, p99 latency, achieved rate and status." width="100%">
</a>
<br><br>

<a href="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-latency-1m.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-latency-1m.svg" alt="HttpArena latency-1m: upstream fixed-rate score, p99 latency, achieved rate and status." width="100%">
</a>
<a href="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-latency-1m.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-latency-1m-dark.svg" alt="HttpArena latency-1m: upstream fixed-rate score, p99 latency, achieved rate and status." width="100%">
</a>
<br><br>

<a href="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-latency-500k-8cpu.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-latency-500k-8cpu.svg" alt="HttpArena latency-500k-8cpu: upstream fixed-rate score, p99 latency, achieved rate and status." width="100%">
</a>
<a href="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-latency-500k-8cpu.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-latency-500k-8cpu-dark.svg" alt="HttpArena latency-500k-8cpu: upstream fixed-rate score, p99 latency, achieved rate and status." width="100%">
</a>
<br><br>

<a href="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-async.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-async.svg" alt="HttpArena async: selected requests per second; higher is better." width="100%">
</a>
<a href="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-async.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-async-dark.svg" alt="HttpArena async: selected requests per second; higher is better." width="100%">
</a>
<br><br>

<a href="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-json-comp.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-json-comp.svg" alt="HttpArena json-comp: compression-adjusted requests per second; higher is better." width="100%">
</a>
<a href="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-json-comp.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-json-comp-dark.svg" alt="HttpArena json-comp: compression-adjusted requests per second; higher is better." width="100%">
</a>
<br><br>

<a href="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-json-tls.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-json-tls.svg" alt="HttpArena json-tls: selected requests per second; higher is better." width="100%">
</a>
<a href="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-json-tls.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-json-tls-dark.svg" alt="HttpArena json-tls: selected requests per second; higher is better." width="100%">
</a>
<br><br>

<a href="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-8gbit.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-8gbit.svg" alt="HttpArena 8gbit: upstream fixed-rate score, p99 latency, achieved rate and status." width="100%">
</a>
<a href="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-8gbit.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-8gbit-dark.svg" alt="HttpArena 8gbit: upstream fixed-rate score, p99 latency, achieved rate and status." width="100%">
</a>
<br><br>

<a href="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-static-tls.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-static-tls.svg" alt="HttpArena static-tls: selected requests per second; higher is better." width="100%">
</a>
<a href="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-static-tls.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="resources/benchmarks/scaleway-test-011-feature-memory/benchmark-static-tls-dark.svg" alt="HttpArena static-tls: selected requests per second; higher is better." width="100%">
</a>
<br><br>

<a name="no-magic-a-few-deliberate-choices"></a>
<h2>
  <a href="resources/readme/readme-design.svg#gh-light-mode-only">
    <img src="resources/readme/readme-design.svg" alt="No magic." width="175">
  </a>
  <a href="resources/readme/readme-design-dark.svg#gh-dark-mode-only">
    <img src="resources/readme/readme-design-dark.svg" alt="No magic." width="175">
  </a>
</h2>

*A few deliberate choices.*

- **HTTP does HTTP. I/O does I/O.** Protocols handle message rules; transports
  move bytes. The same HTTP implementation runs on both native backends.

- **Copies should earn their keep.** Metadata views, explicit ownership, and
  a fixed buffer for small responses help avoid unnecessary copies.
  Not "zero-copy everything". Just deliberate data movement.

- **Handlers run directly.** Synchronous execution preserves request order
  without a separate response scheduler.

<a name="hello-world-lets-not-overcomplicate-it"></a>
<h2>
  <a href="resources/readme/readme-hello.svg#gh-light-mode-only">
    <img src="resources/readme/readme-hello.svg" alt="Hello, world." width="220">
  </a>
  <a href="resources/readme/readme-hello-dark.svg#gh-dark-mode-only">
    <img src="resources/readme/readme-hello-dark.svg" alt="Hello, world." width="220">
  </a>
</h2>

*Let's not overcomplicate it.*

```cpp
#include "common/signaler.h"
#include "http_server.h"

using namespace martianlabs::doba::common;
using namespace martianlabs::doba::protocol::http::v11;

int main() {
  martianlabs::doba::http_server<> srv({.ip = "0.0.0.0", .port = "8080"});
  srv.add_route(
      "GET", "/hello",
      [](const request&, response& res) {
        res.ok_200();
        res.add_header("Content-Type", "text/plain")
            .set_body("hello from doba");
      });
  srv.start();
  signaler::wait();
}
```

Once running: `curl http://localhost:8080/hello`

**Build & integrate**

<details>
<summary>Build, install, and use with CMake</summary>

With a C++20 compiler and CMake 3.20 or later, build the bundled examples and
tests, then install the headers and package configuration:

```sh
cmake -S . -B out/build/release -DCMAKE_BUILD_TYPE=Release
cmake --build out/build/release --config Release
cmake --install out/build/release --config Release --prefix out/install/release
```

In your application:

```cmake
find_package(doba CONFIG REQUIRED)
target_link_libraries(application PRIVATE martianlabs::doba)
```

For TLS, configure with `-DDOBA_ENABLE_TLS=ON` and link
`martianlabs::doba_tls`. OpenSSL is required for this optional transport.
See the [HTTPS example](examples/http/v11/https_hello_world/README.md).

Set `CMAKE_PREFIX_PATH` to the installation prefix when it is not in a
standard system location. The exported target provides the include directory,
C++20 requirement, and system threading dependency.

</details>

<a name="fast-is-nice-correct-is-non-negotiable"></a>
<h2>
  <a href="resources/readme/readme-correctness.svg#gh-light-mode-only">
    <img src="resources/readme/readme-correctness.svg" alt="Fast is nice." width="203">
  </a>
  <a href="resources/readme/readme-correctness-dark.svg#gh-dark-mode-only">
    <img src="resources/readme/readme-correctness-dark.svg" alt="Fast is nice." width="203">
  </a>
</h2>

*Correct is non-negotiable.*

We like fast code. We also like sleeping at night. So the test suite asks
some fairly awkward questions before we trust the numbers.

- **1,000+ unit tests get into the small print.** HTTP parsing, raw and
  chunked bodies, routing, authentication, storage, and byte ownership.
  Empty input, malformed headers, exact limits, one byte too many: all get
  a turn in the [unit suite](tests/unit).

- **160+ integration cases bring real sockets.** TCP and TLS connections,
  fragmented messages, pipelined requests, disconnects, and shutdowns go
  through the actual server. Hostile requests get successors attached so
  the [integration tests](tests/integration) can check they never sneak
  through to a handler.

- **Concurrency gets stage directions.** We suspend handlers and control
  their completions to exercise response ordering, receive/completion races,
  late callbacks, and object lifetimes. CI repeats the
  [critical interleavings](tests/unit/protocol/http/v11/engine_concurrency_tests.cpp)
  **200 times under ThreadSanitizer**.

- **Clients are allowed to be annoying.** The
  [resilience suite](tests/integration/protocol/http/v11/server_resilience_tests.cpp)
  mixes multiple clients, small reads, pending handlers, abrupt resets,
  disk spill, draining, and repeated restarts over TCP and TLS. It checks
  payloads, ordering, handler cleanup, temporary files, and handle/descriptor
  counts after draining, and records memory use after warmup.

- **Fuzzers keep feeding the weird stuff.** Separate
  [HTTP decoder and chunked harnesses](tests/fuzz/README.md) check consumption,
  message boundaries, fragmentation, buffer ownership, and memory/disk
  storage behavior under AddressSanitizer and UndefinedBehaviorSanitizer.
  Each gets **60 seconds per CI run**, with a cached corpus and saved failure
  inputs for reproduction.

- **We break things on purpose, too.** The
  [mutation checks](tests/mutation/README.md) disable a body limit, break
  response ordering, and allow dispatch after rejection in private copies
  of the headers. Each change has to trip its intended assertion. The
  original code has to pass. A test that politely ignores its assigned bug
  doesn't get a gold star.

- **Even the test runners get tested.** Filters, failure reports, exceptions,
  timeouts, and cleanup paths have their own
  [unit](tests/unit/helpers/test_helper_tests.cpp) and
  [integration](tests/integration/helpers/test_helper_tests.cpp) metatests.
  A stuck test gets a name and a deadline.

The [CI matrix](.github/workflows/ci.yml) builds with **GCC and Clang on Linux**
and **MSVC on Windows**, with strict warnings. Linux runs **ASan, UBSan, and
TSan with TLS**; Windows runs **ASan with TLS**. **Tests run in Release,
optimizations included. Debug is a compile check.** The test tooling itself
uses C++ and CMake.

We also check **CMake 3.20**, installed-package consumers, and
**add_subdirectory** consumers. Building inside our own tree is only half
the story; your application needs to build and run with it, too.

Want another opinion on the wire? The repository includes adapters for
manual runs of [h1spec](compliance/http/v11/h1spec/README.md) and
[Http11Probe](compliance/http/v11/http11probe/README.md).

---

[Apache 2.0](LICENSE)

