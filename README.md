![doba](resources/doba-small.png)

**A high-performance, transport- and protocol-agnostic C++20 server framework. HTTP/1.1 is its first protocol, not its limit.**

---

## What is doba?

doba is a **generic server framework** built around one hard rule: transport and protocol are kept strictly separate. The transport layer never sees a header, a method name, or a status line — only a universal channel-lifecycle signal. HTTP/1.1 is the first protocol plugged into that contract; swap it, extend it, or run several side by side.

It ships as **pure headers**. No build step, no binary to link, no dependencies to fetch. `#include` it, point a C++20 compiler at it, and you have a server.

## Features

- **Transport and protocol stay separate.** The transport never needs to know about headers, methods, or status lines. Protocols remain free to define their own semantics, and transports remain free to focus on moving bytes efficiently.

- **Built for the hot path.** Parsing is single-pass and works directly over the received data wherever lifetime permits it. No re-scans, no unnecessary intermediate representations.

- **Direct dispatch.** Header dispatch uses raw function pointers and the routing path stays deliberately simple: no framework machinery, no virtual dispatch, and no per-request indirection for its own sake.

- **Choose how a route runs.** Routes run synchronously by default. Mark one as asynchronous when its handler should leave the transport worker and complete later without blocking the synchronous path.

- **Memory where it earns its place.** Responses use a fixed buffer for the latency-sensitive write path. Requests retain only the storage they need because inbound data is variable-sized and may outlive parsing.

- **HTTP/1.1 is serious, not incidental.** It is the first protocol implemented on doba’s generic foundation: strict request parsing, all request-target forms, header validation, framing rules, connection directives, and body handling.

- **Bodies scale beyond RAM.** Raw and chunked bodies use memory or file-backed storage according to configured limits, while preserving the original wire representation when required.

- **Native asynchronous I/O.** Windows uses IOCP and Linux uses epoll, each written against the platform primitive directly. Both backends support pipelined responses and accept completions from asynchronous route handlers while preserving response order.

- **Keep control.** Request, response, decoder, transport, and router remain template parameters, so the framework can be adapted without rewriting its core.

## Quick look

```cpp
#include <memory>

#include "common/execution_policy.h"
#include "protocol/http/v11/server.h"

using namespace martianlabs::doba::common;
using namespace martianlabs::doba::protocol::http::v11;

int main() {
  server srv;
  srv.add_route(
      "GET", "/hello",
      [](std::shared_ptr<const request>, std::shared_ptr<response> res) {
        res->ok_200()
           .add_header("Content-Type", "text/plain; charset=utf-8")
           .set_body("hello from doba");
      },
      execution_policy::kSynchronous);
  srv.start("8080");
}
```

## Build

doba is header-only — consuming it is just an include path. To build the bundled example:

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Requires CMake ≥ 3.20 and a C++20 compiler. MSVC/Ninja presets (`msvc-debug`, `msvc-release`) are provided in `CMakePresets.json`.

## Current scope

doba is under active development. The transport/protocol contract, HTTP/1.1 request decoding, routing, and body handling are in place on both backends. Notably absent today: TLS (deploy behind a terminator), `100-continue`, conditional requests and `Range` evaluation, content negotiation, and connection timeouts. Pending work is tracked in [docs/HANDOFF.md](docs/HANDOFF.md).

## License

Apache License 2.0 — see [LICENSE](LICENSE).

