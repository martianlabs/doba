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

A segfault with excellent throughput is still a segfault. So, before doba
gets anywhere near a victory lap, we poke it where it hurts.

<a href="resources/readme/tests/unit.svg#gh-light-mode-only">
  <picture>
    <source media="(max-width: 600px)" srcset="resources/readme/tests/unit-narrow.svg">
    <img src="resources/readme/tests/unit.svg" alt="1,000+ unit tests pick fights with the details." width="100%">
  </picture>
</a>
<a href="resources/readme/tests/unit-dark.svg#gh-dark-mode-only">
  <picture>
    <source media="(max-width: 600px)" srcset="resources/readme/tests/unit-narrow-dark.svg">
    <img src="resources/readme/tests/unit-dark.svg" alt="1,000+ unit tests pick fights with the details." width="100%">
  </picture>
</a>

HTTP parsing, raw and
chunked bodies, routing, authentication, storage, and byte ownership.
The [unit suite](tests/unit) brings empty input, malformed headers,
exact limits, and that one extra byte somebody swore would never arrive.

<a href="resources/readme/tests/integration.svg#gh-light-mode-only">
  <picture>
    <source media="(max-width: 600px)" srcset="resources/readme/tests/integration-narrow.svg">
    <img src="resources/readme/tests/integration.svg" alt="160+ integration cases mess with real connections." width="100%">
  </picture>
</a>
<a href="resources/readme/tests/integration-dark.svg#gh-dark-mode-only">
  <picture>
    <source media="(max-width: 600px)" srcset="resources/readme/tests/integration-narrow-dark.svg">
    <img src="resources/readme/tests/integration-dark.svg" alt="160+ integration cases mess with real connections." width="100%">
  </picture>
</a>

The
[integration suite](tests/integration) fragments messages, pipelines requests,
disconnects clients, and shuts down actual TCP and TLS servers. Hostile
requests arrive with another request tucked behind them. Getting rejected
doesn't buy the one behind a free pass to a handler.

<a href="resources/readme/tests/concurrency.svg#gh-light-mode-only">
  <picture>
    <source media="(max-width: 600px)" srcset="resources/readme/tests/concurrency-narrow.svg">
    <img src="resources/readme/tests/concurrency.svg" alt="Handlers finish when we say so." width="100%">
  </picture>
</a>
<a href="resources/readme/tests/concurrency-dark.svg#gh-dark-mode-only">
  <picture>
    <source media="(max-width: 600px)" srcset="resources/readme/tests/concurrency-narrow-dark.svg">
    <img src="resources/readme/tests/concurrency-dark.svg" alt="Handlers finish when we say so." width="100%">
  </picture>
</a>

We suspend them, release them out of
order, and arrange inconvenient meetings between receives, completions,
late callbacks, and shutdown. Responses still need to keep their place;
objects still need to stay alive for their users. CI repeats the
[critical interleavings](tests/unit/protocol/http/v11/engine_concurrency_tests.cpp)
**200 times under ThreadSanitizer**.

<a href="resources/readme/tests/resilience.svg#gh-light-mode-only">
  <picture>
    <source media="(max-width: 600px)" srcset="resources/readme/tests/resilience-narrow.svg">
    <img src="resources/readme/tests/resilience.svg" alt="Our test clients are absolute pests." width="100%">
  </picture>
</a>
<a href="resources/readme/tests/resilience-dark.svg#gh-dark-mode-only">
  <picture>
    <source media="(max-width: 600px)" srcset="resources/readme/tests/resilience-narrow-dark.svg">
    <img src="resources/readme/tests/resilience-dark.svg" alt="Our test clients are absolute pests." width="100%">
  </picture>
</a>

Tiny reads. Suspended handlers.
Abrupt resets. The
[resilience suite](tests/integration/protocol/http/v11/server_resilience_tests.cpp)
mixes them across multiple TCP and TLS clients, spills bodies to disk,
drains, restarts, and does it again. Then comes the headcount: payloads in
order, handlers cleaned up, temporary files gone, handles/descriptors
checked after draining. Memory use gets recorded after warmup, too.

<a href="resources/readme/tests/fuzzing.svg#gh-light-mode-only">
  <picture>
    <source media="(max-width: 600px)" srcset="resources/readme/tests/fuzzing-narrow.svg">
    <img src="resources/readme/tests/fuzzing.svg" alt="The fuzzers have terrible manners." width="100%">
  </picture>
</a>
<a href="resources/readme/tests/fuzzing-dark.svg#gh-dark-mode-only">
  <picture>
    <source media="(max-width: 600px)" srcset="resources/readme/tests/fuzzing-narrow-dark.svg">
    <img src="resources/readme/tests/fuzzing-dark.svg" alt="The fuzzers have terrible manners." width="100%">
  </picture>
</a>

Our
[HTTP decoder and chunked harnesses](tests/fuzz/README.md) check consumption,
message boundaries, fragmentation, buffer ownership, and memory/disk
storage while libFuzzer supplies the questionable bytes. Each gets
**60 seconds per CI run** under AddressSanitizer and UndefinedBehaviorSanitizer,
with a cached corpus. If something breaks, we keep the input. Receipts matter.

<a href="resources/readme/tests/mutation.svg#gh-light-mode-only">
  <picture>
    <source media="(max-width: 600px)" srcset="resources/readme/tests/mutation-narrow.svg">
    <img src="resources/readme/tests/mutation.svg" alt="Three bugs, planted on purpose." width="100%">
  </picture>
</a>
<a href="resources/readme/tests/mutation-dark.svg#gh-dark-mode-only">
  <picture>
    <source media="(max-width: 600px)" srcset="resources/readme/tests/mutation-narrow-dark.svg">
    <img src="resources/readme/tests/mutation-dark.svg" alt="Three bugs, planted on purpose." width="100%">
  </picture>
</a>

In private header copies, the
[mutation checks](tests/mutation/README.md) disable a body limit, break
response ordering, and allow dispatch after rejection. Each bug must trip
its intended assertion; the original code must pass. Green is only useful
if red still works.

<a href="resources/readme/tests/runners.svg#gh-light-mode-only">
  <picture>
    <source media="(max-width: 600px)" srcset="resources/readme/tests/runners-narrow.svg">
    <img src="resources/readme/tests/runners.svg" alt="Even the test runner gets interrogated." width="100%">
  </picture>
</a>
<a href="resources/readme/tests/runners-dark.svg#gh-dark-mode-only">
  <picture>
    <source media="(max-width: 600px)" srcset="resources/readme/tests/runners-narrow-dark.svg">
    <img src="resources/readme/tests/runners-dark.svg" alt="Even the test runner gets interrogated." width="100%">
  </picture>
</a>

Filters, failure reports, exceptions,
timeouts, and cleanup paths have their own
[unit](tests/unit/helpers/test_helper_tests.cpp) and
[integration](tests/integration/helpers/test_helper_tests.cpp) metatests.
A stuck test has a deadline and leaves its name at the scene.

The [CI matrix](.github/workflows/ci.yml) builds with **GCC and Clang on Linux**
and **MSVC on Windows**, with strict warnings. Linux runs **ASan, UBSan, and
TSan with TLS**; Windows runs **ASan with TLS**. **Release gets the workout,
optimizations and all. Debug gets a compile check.** The test tooling itself
uses C++ and CMake.

We also check **CMake 3.20**, then build and run separate applications using
the installed package and **add_subdirectory**. A library should survive
moving out of its parents' build directory.

For more awkward questions from outside the family, there are adapters for
manual protocol checks with [h1spec](compliance/http/v11/h1spec/README.md) and
[Http11Probe](compliance/http/v11/http11probe/README.md).

---

[Apache 2.0](LICENSE)

