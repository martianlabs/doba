![doba](resources/doba-small.png)

[![Build & Test](https://github.com/martianlabs/doba/actions/workflows/ci.yml/badge.svg?branch=main&event=push)](https://github.com/martianlabs/doba/actions/workflows/ci.yml?query=branch%3Amain)

**A high-performance, transport- and protocol-agnostic C++20 server framework. HTTP/1.1 is its first protocol, not its limit.**

---

## What is doba?

doba is a **generic server framework** built around one hard rule: transport and protocol are kept strictly separate. The transport layer never sees a header, a method name, or a status line - only a universal channel-lifecycle signal. HTTP/1.1 is the first protocol plugged into that contract; swap it, extend it, or run several side by side.

It ships as **pure headers**. No build step, no binary to link, no dependencies to fetch. `#include` it, point a C++20 compiler at it, and you have a server.

## Features

- **Transport and protocol stay separate.** The transport never needs to know about headers, methods, or status lines. Protocols remain free to define their own semantics, and transports remain free to focus on moving bytes efficiently.

- **Built for the hot path.** Parsing is single-pass and works directly over the received data wherever lifetime permits it. No re-scans, no unnecessary intermediate representations.

- **Direct dispatch.** Header dispatch uses raw function pointers and the routing path stays deliberately simple: no framework machinery or virtual dispatch. Synchronous routes retain their direct hot path.

- **Synchronous and coroutine handlers.** Synchronous route handlers execute inline on the transport worker and complete the response before returning. Asynchronous handlers receive a `std::stop_token`, return `common::task<response>`, and may suspend without changing the synchronous route contract.

- **Memory where it earns its place.** Responses use a fixed buffer for the latency-sensitive write path. Requests retain only the storage they need because inbound data is variable-sized and may outlive parsing.

- **HTTP/1.1 is serious, not incidental.** It is the first protocol implemented on doba's generic foundation: strict request parsing, all request-target forms, header validation, framing rules, connection directives, and body handling.

- **Bodies scale beyond RAM.** Raw and chunked bodies use memory or file-backed storage according to configured limits, while preserving the original wire representation when required.

- **Native event-driven I/O on both platforms.** Windows (IOCP) and Linux (epoll) are both fully implemented and selected at compile time by `transport/server/tcpip.h` - there is no reference or fallback backend. Each is written against the platform primitive directly, while both support immediate and deferred completions and preserve response order.

- **Keep control.** Request, response, decoder, transport, and router remain template parameters, so the framework can be adapted without rewriting its core.

## Quick look

```cpp
#include "protocol/http/v11/server.h"

using namespace martianlabs::doba::protocol::http::v11;

int main() {
  server srv;
  srv.add_route(
      "GET", "/hello",
      [](const request&, response& res) {
        res.ok_200()
           .add_header("Content-Type", "text/plain; charset=utf-8")
           .set_body("hello from doba");
      });
  srv.start("8080");
}
```

The [examples catalog](examples/README.md) groups standalone `common` APIs and
HTTP/1.1 server behavior. The [asynchronous routes example](examples/http/v11/asynchronous_routes/main.cpp)
shows a coroutine handler with a real suspension resumed by an application
executor.

Asynchronous handlers take `std::shared_ptr<const request>` and
`std::stop_token` before any route parameters. Awaited operations must observe
the token and eventually resume after cancellation. The transport requests
stop when the connection disappears or the server stops.

## Build

doba is header-only. To build the bundled examples and tests:

```bash
cmake -S . -B out/build/release -DCMAKE_BUILD_TYPE=Release
cmake --build out/build/release
```

Requires CMake >= 3.20 and a C++20 compiler. The MSVC/Ninja presets
(`msvc-debug`, `msvc-release`) provided in `CMakePresets.json` require
CMake >= 3.25.

`DOBA_BUILD_EXAMPLES` and `DOBA_BUILD_TESTS` default to `ON` when doba is the
top-level project and to `OFF` when it is added with `add_subdirectory` or
FetchContent. `BUILD_TESTING=OFF` disables tests explicitly.

CI builds and tests Debug and Release with GCC, Clang, and MSVC. It also runs
ASan, UBSan, TSan, and CMake 3.20.6. Strict warnings are enabled in CI and can
be enabled locally with `-DDOBA_ENABLE_STRICT_WARNINGS=ON`.

Component tests mirror the public include tree below `tests/unit`,
`tests/integration`, and `tests/package`. Shared harness files remain at each
suite root.

To install doba into a dedicated prefix:

```bash
cmake --install out/build/release --prefix out/install/release
```

A CMake project can then consume the installed package with:

```cmake
find_package(
  doba
  CONFIG
  REQUIRED
)

target_link_libraries(
  application
  PRIVATE
  martianlabs::doba
)
```

Set `CMAKE_PREFIX_PATH` to the installation prefix when it is not in a
standard system location. The exported target provides the include directory,
C++20 requirement, and system threading dependency.

## Current scope

doba is under active development. The transport/protocol contract, HTTP/1.1
request decoding, routing, `Expect: 100-continue`, and body handling are in
place, and both the Windows (IOCP) and Linux (epoll) transports are implemented
and unified behind the same contract.

Deliberately **out of scope for the first release**: TLS (deploy behind a
terminator) and content compression - both are product decisions, the latter
because it would introduce an external dependency and break the "no
dependencies" rule above.

Still pending for compliance: connection timeouts; evaluation of conditional
requests; `Range` evaluation; automatic resource-specific `OPTIONS` responses;
outbound trailers; and effective connection limits. `OPTIONS *` is already
implemented. Protocol upgrade handling is out of scope for the first release.
Pending work is tracked in [docs/HANDOFF.md](docs/HANDOFF.md).
Automated fuzzing and CI execution of the existing h1spec and Http11Probe
adapters are deferred and are not part of the current hardening plan.

## License

Apache License 2.0 - see [LICENSE](LICENSE).

