![doba](resources/doba.png)

**A protocol-agnostic, header-only C++20 server framework. Zero-copy. Zero-allocation. Zero excuses.**

---

## What is doba?

doba is a **generic server framework** built around one hard rule: transport and protocol are kept strictly separate. The transport layer never sees a header, a method name, or a status line — only a universal channel-lifecycle signal. HTTP/1.1 is the first protocol plugged into that contract; swap it, extend it, or run several side by side.

It ships as **pure headers**. No build step, no binary to link, no dependencies to fetch. `#include` it, point a C++20 compiler at it, and you have a server.

## Features

- 🔌 **Genuinely protocol-agnostic core.** Transport and protocol communicate through one narrow contract. Change protocols without touching transport code; change transports without touching protocol code.
- ⚡ **Single-pass, zero-copy parsing.** Every byte is read once. Semantic state is captured directly over the original buffer — no re-scans, no intermediate copies.
- 🚀 **Zero-allocation dispatch.** Routing runs through a hash map of raw function pointers — no `std::function`, no vtables, no per-request heap churn. O(1), fully inlinable.
- 🧩 **A memory model that fits the job.** `response` is allocation-free on a fixed buffer, built for a latency-critical write path. `request` uses container-backed storage because inbound data is variable-length and survives past parsing. Each shape earned by what the type has to do.
- 🛡️ **RFC-correct HTTP/1.1 out of the box.** All 5 request-target forms. 66 headers checked syntactically. 14 headers modeled semantically with dedicated interpreters. Cross-header correctness — framing, routing, connection directives — enforced in a single post-parse pass.
- 🎯 **Full body support.** `body_writer` and `body_reader` handle raw and chunked encoding over memory or file-backed storage, with configurable limits and lazy decoding.
- 🔀 **Sync or async, per route.** Each route independently chooses inline execution or hand-off to the built-in thread pool — no framework-wide trade-off forced on every endpoint.
- 🖥️ **Native async I/O on every platform.** IOCP on Windows, epoll on Linux — edge-triggered, one-shot, with response pipelining, reorder windowing, and carry-over of partial receives built in.
- ⚙️ **Fully parameterized.** `server<RQty, RSty, TRty, FNty, ROty>` — swap out the request, response, transport, function type, or router without modifying the framework.

## Quick look

```cpp
#include "protocol/http11/server.h"

using namespace martianlabs::doba::protocol::http11;

int main() {
  server srv;
  srv.add_route(
      "GET", "/hello",
      [](const request& req, response& res) {
        res.ok_200()
           .add_header("Content-Type", "text/plain; charset=utf-8")
           .set_body("hello from doba");
      },
      execution_policy::kSync)
     .start("8080");
}
```

## Build

doba is header-only — consuming it is just an include path. To build the bundled example:

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Requires CMake ≥ 3.20 and a C++20 compiler. MSVC/Ninja presets (`msvc-debug`, `msvc-release`) are provided in `CMakePresets.json`.

## License

Apache License 2.0 — see [LICENSE](LICENSE).

