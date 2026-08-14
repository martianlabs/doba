![doba](resources/doba-small.png)

[![Build & Test](https://github.com/martianlabs/doba/actions/workflows/ci.yml/badge.svg?branch=main&event=push)](https://github.com/martianlabs/doba/actions/workflows/ci.yml?query=branch%3Amain)

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

- **Native asynchronous I/O on both platforms.** Windows (IOCP) and Linux (epoll) are both fully implemented and selected at compile time by `transport/server/tcpip.h` — there is no reference or fallback backend. Each is written against the platform primitive directly, yet both expose the same public contract to the protocol layer and share the same response-ordering model: pipelined responses are tracked by monotonic response identifiers, so completions from asynchronous route handlers are still delivered in request order.

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

Requires CMake ≥ 3.20 and a C++20 compiler. The MSVC/Ninja presets
(`msvc-debug`, `msvc-release`) provided in `CMakePresets.json` require
CMake ≥ 3.25.

## Current scope

doba is under active development. The transport/protocol contract, HTTP/1.1
request decoding, routing, `Expect: 100-continue`, and body handling are in
place, and both the Windows (IOCP) and Linux (epoll) transports are implemented
and unified behind the same contract.

Deliberately **out of scope for the first release**: TLS (deploy behind a
terminator) and content compression — both are product decisions, the latter
because it would introduce an external dependency and break the "no
dependencies" rule above.

Still pending for compliance: connection timeouts, conditional request and
`Range` evaluation, automatic resource-specific `OPTIONS` responses, protocol
upgrade handling, outbound trailers, and effective connection limits.
`OPTIONS *` and `Allow` on `405 Method Not Allowed` are already implemented.
Pending work is tracked in [docs/HANDOFF.md](docs/HANDOFF.md).

## License

Apache License 2.0 — see [LICENSE](LICENSE).

