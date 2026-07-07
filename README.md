![doba](resources/doba.png)

**A protocol-agnostic, header-only C++20 server framework. Zero-copy. Zero-allocation. Zero excuses.**

---

## What is doba?

doba is a **generic server framework**, not an HTTP library that pretends to be one. It draws a hard line between **transport** (how bytes move) and **protocol** (what the bytes mean): the transport only ever sees a closed, universal vocabulary for a channel's lifecycle, never a header, a status line, or a method name. HTTP/1.1 is simply the first protocol plugged into that contract — swap it, extend it, or run several side by side, and the rest of the stack doesn't need to know.

It ships as **pure headers**. No build step, no binary to link, no third-party dependencies to fetch or version. `#include` it, point a C++20 compiler at it, and you have a server.

## Why use doba?

Most "generic" server frameworks are generic in name only — scratch the surface and there's an HTTP-shaped hole where the transport should be. doba is built the other way around, and it shows in every layer:

- 🔌 **A genuinely protocol-agnostic core.** Transport and protocol talk through one narrow contract — `channel_intent` (`kKeep` / `kClose` / `kUpgrade`) — the entire set of things a transport can physically do with a connection. Change protocols without touching transport code; change transports without touching protocol code.
- 🧠 **Single-pass, zero-copy parsing.** Every unit of protocol syntax is inspected exactly once — a header, in HTTP/1.1's case. Semantic state is captured as `std::string_view`s over the original buffer as it's parsed — no re-scans, no intermediate copies just to answer a second question about data you already read.
- ⚡ **Zero-allocation dispatch.** Protocol-level routing — HTTP/1.1's header dispatch today — runs through a hash map of raw function pointers — no `std::function`, no vtables, no per-request heap churn — resolved in O(1) and fully inlinable.
- 🧩 **A memory model that fits the job, not a compromise.** HTTP/1.1's `response` is allocation-free with a fixed-footprint buffer, built for a latency-critical write path; its `request` uses flexible, container-backed storage, because inbound data is variable-length and has to survive past parsing. Same discipline, two shapes — each one earned by what the type actually has to do.
- 🛡️ **Policy-driven configurability.** Every inbound limit is plain data you set, not behavior you fork — HTTP/1.1's `policies` covers content-length ceilings, forwarding-hop limits, and chunked/upgrade toggles.
- 🧵 **Sync or async, per route.** Each route independently chooses inline execution or hand-off to the built-in thread pool — no framework-wide trade-off imposed on every endpoint.
- 🖥️ **Async I/O transport, done properly.** The reference TCP/IP transport drives every connection through native asynchronous I/O (IOCP on Windows) instead of thread-per-connection or busy-polling.
- 🎯 **No dangling edge cases.** In HTTP/1.1, every recognized request shape resolves to a definitive, well-formed response — including the ones most frameworks quietly get wrong.

## HTTP/1.1: the reference protocol, done rigorously

doba's HTTP/1.1 implementation isn't "good enough" — it's built to track **RFC 9110 / RFC 9112** to the letter, without ever leaving the single-pass, zero-copy path:

- **All 5 request-target forms recognized and validated** — origin, absolute, authority, and asterisk — not just the origin-form most minimal parsers stop at. A `CONNECT` request gets a clean, deliberate response instead of a hung connection; `OPTIONS *` is acknowledged server-wide without a fake route lookup.
- **66 headers checked syntactically, 14 modeled semantically** with dedicated interpreters, all wired through a single dispatch table so nothing is ever parsed twice — and a compile-time `static_assert` guards the generic dispatch bridge so a context-dependent checker can never be mis-wired into it.
- **Cross-header correctness enforced exactly where the RFC demands it** — framing, host/routing, connection-directives, and policy are all evaluated together, once, right after the single header pass completes:
  - Rejects duplicate `Content-Length` outright — the classic CL.CL request-smuggling vector.
  - Rejects `Transfer-Encoding` and `Content-Length` appearing together, and rejects `chunked` unless it's the final coding.
  - Requires exactly one `Host`, and reconciles it against the request-target authority with **scheme-aware default-port normalization** (`Host: example.com` correctly matches `http://example.com:80/...`).
  - Rejects `Connection` options that try to hop-by-hop a control header (`Host`, `Content-Length`, `Transfer-Encoding`, …), and rejects an `upgrade` option with no matching `Upgrade` offer.

The result is an HTTP/1.1 layer that is strict about correctness *because* of the same pass that makes it fast — validation and extraction happen together, once, not as two separate costs.

## Quick look

```cpp
#include "protocol/http11/server.h"

using namespace martianlabs::doba::protocol::http11;

int main() {
  server srv;
  srv.add_route(
      "GET", "/pipeline",
      [](const request& req, response& res) {
        res.ok_200()
           .add_header("Content-Type", "text/plain; charset=utf-8")
           .set_body("ok");
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
