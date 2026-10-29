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

Measured on **Scaleway, 2026-10-28**, using the official HttpArena suite:
native **gcannon** and **wrk**, with **zrk** and servers in Docker, following
upstream CPU affinity and host tuning. **RUNS=3**. Results are selected by
upstream from its repetitions, not averaged across runs.
The campaign's reference-model verification is **incomplete**; these are
local results, not the published HttpArena leaderboard.

Every chart shows the **same top 15 from the overall ranking**, ordered by each
test's metric. Scores retain normalization across all **21 measured servers**.
The total includes eight scored profiles and the upstream completeness factor;
**pipelined**, **latency-500k-8cpu** and **static-tls** are reference-only profiles.
Failed and skipped measurements are shown explicitly; they are not measured zeroes.

The test machine uses an **AMD EPYC 8534P at 2.30 GHz**
(**64 cores / 128 threads**), **576 GiB of DDR5 RAM** and
**2 x 3.84 TB NVMe** storage. It provides a **25 Gbps private network**
and a **1 Gbps public connection**, upgradable to **25 Gbps**.

<a href="resources/benchmarks/benchmark-summary.svg#gh-light-mode-only" title="Open full-size light card">
  <picture>
    <source media="(max-width: 600px)" srcset="resources/benchmarks/benchmark-summary-narrow.svg">
    <img src="resources/benchmarks/benchmark-summary.svg" alt="HttpArena local run on Scaleway, 2026-10-28. doba ranks 1 of 21 with 7,366.65 overall points, 6.1% more points than spliffy. Upstream totals include completeness. Top 15 shown. Overall ranking, not a claim of winning every profile. Reference-model verification is incomplete; see docs/benchmarks/README.md." width="100%">
  </picture>
</a>
<a href="resources/benchmarks/benchmark-summary.svg#gh-dark-mode-only" title="Open full-size light card">
  <picture>
    <source media="(max-width: 600px)" srcset="resources/benchmarks/benchmark-summary-narrow-dark.svg">
    <img src="resources/benchmarks/benchmark-summary-dark.svg" alt="HttpArena local run on Scaleway, 2026-10-28. doba ranks 1 of 21 with 7,366.65 overall points, 6.1% more points than spliffy. Upstream totals include completeness. Top 15 shown. Overall ranking, not a claim of winning every profile. Reference-model verification is incomplete; see docs/benchmarks/README.md." width="100%">
  </picture>
</a>

[See the full ranking, all 11 workloads, and machine specs](docs/benchmarks/README.md).

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

We like a good benchmark as much as the next person. But if a server needs
well-behaved clients and a bit of luck to stay up, the numbers can wait.
Here's what we put doba through before letting it brag about being fast.

<a href="resources/readme/quality-summary.svg#gh-light-mode-only" title="Open full-size light card">
  <picture>
    <source media="(max-width: 600px)" srcset="resources/readme/quality-summary-narrow.svg">
    <img src="resources/readme/quality-summary.svg" alt="1,100+ unit test cases; 220+ integration cases; 200 concurrency repetitions in CI; ten deliberately planted bugs checked by mutation tests. Fuzzing, ASan and UBSan on Linux and Windows; TSan on Linux. Release builds. Counts describe source test declarations, not results of a newly executed test run." width="100%">
  </picture>
</a>
<a href="resources/readme/quality-summary.svg#gh-dark-mode-only" title="Open full-size light card">
  <picture>
    <source media="(max-width: 600px)" srcset="resources/readme/quality-summary-narrow-dark.svg">
    <img src="resources/readme/quality-summary-dark.svg" alt="1,100+ unit test cases; 220+ integration cases; 200 concurrency repetitions in CI; ten deliberately planted bugs checked by mutation tests. Fuzzing, ASan and UBSan on Linux and Windows; TSan on Linux. Release builds. Counts describe source test declarations, not results of a newly executed test run." width="100%">
  </picture>
</a>

All of this has to get past the [CI matrix](.github/workflows/ci.yml):
**GCC and Clang on Linux**, **MSVC on Windows**, strict warnings enabled.
Linux runs **ASan, UBSan, and TSan with TLS**; Windows runs **ASan and UBSan
with TLS**, including a Clang build alongside MSVC. Both platforms repeat
the critical concurrency cases **200 times**; TSan checks for races on Linux.
And yes, **the tests run in Release, optimizations and all**. That's the code
we care about running. Debug gets a compile check. The test tooling uses
C++ and CMake; building doba is quite enough setup for testing doba.

Fancy a second opinion? There are also adapters for manual protocol checks
with [h1spec](compliance/http/v11/h1spec/README.md) and
[Http11Probe](compliance/http/v11/http11probe/README.md). We wrote the server
and the tests; letting somebody else's tools have a go seems only fair.

[Inside the test suites](tests/README.md)

---

[Apache 2.0](LICENSE)
