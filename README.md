<picture>
  <source media="(prefers-color-scheme: dark)" srcset="resources/doba-dark.png">
  <img src="resources/doba.png" alt="doba" width="640">
</picture>
<br><br>

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="resources/readme-hero-dark.svg">
  <img src="resources/readme-hero.svg" alt="You write the handler. doba moves the bytes." width="540">
</picture>
<br><br>

[![Build & Test](https://github.com/martianlabs/doba/actions/workflows/ci.yml/badge.svg?branch=main&event=push)](https://github.com/martianlabs/doba/actions/workflows/ci.yml?query=branch%3Amain)

Meet doba: a header-only C++20 server framework. Protocols and transports
mind their own business. HTTP/1.1 comes first; the architecture leaves room
for more.

Native IOCP on Windows. Native epoll on Linux. No external libraries to chase.

[Try the examples](examples/README.md) /
[Peek under the hood](docs/ARCHITECTURE.md)

## Okay, how fast?

Big throughput numbers are fun. Waiting for a response isn't. Let's look at both.

> Benchmark charts coming soon. Reproducible results, not imaginary bars.

## No magic. A few deliberate choices.

- **HTTP does HTTP. I/O does I/O.** Protocols handle message rules; transports
  move bytes. The same HTTP implementation runs on both native backends.

- **Copies should earn their keep.** Metadata views, explicit ownership, and
  a fixed buffer for small responses help avoid unnecessary copies.
  Not "zero-copy everything". Just deliberate data movement.

- **Not every handler needs a coroutine.** Sync handlers run directly.
  When work needs to wait, coroutine handlers support cancellation and keep
  responses in order.

## Hello, world. Let's not overcomplicate it.

```cpp
#include "common/signaler.h"
#include "protocol/http/v11/server.h"

using namespace martianlabs::doba::common;
using namespace martianlabs::doba::protocol::http::v11;

int main() {
  server srv;
  srv.add_route(
      "GET", "/hello",
      [](const request&) {
        response res;
        res.ok_200()
            .add_header("Content-Type", "text/plain")
            .set_body("hello from doba");
        return res;
      });
  srv.start("8080");
  signaler::wait();
}
```

Once running: `curl http://localhost:8080/hello`

[Build & integrate](docs/DEVELOPMENT.md) /
[See a coroutine handler](examples/http/v11/asynchronous_routes/main.cpp)

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

Set `CMAKE_PREFIX_PATH` to the installation prefix when it is not in a
standard system location. The exported target provides the include directory,
C++20 requirement, and system threading dependency.

For compiler setup, presets, build options, and test commands, see
[Development](docs/DEVELOPMENT.md).

</details>

## Fast is nice. Correct is non-negotiable.

We like fast code. We also like sleeping at night. HTTP/1.1 parsing and framing
follow RFC rules; CI checks GCC, Clang, and MSVC builds, strict warnings, and
ASan, UBSan, and TSan runs.

We're working toward 0.1. TLS and compression aren't in the first release,
and operational hardening is still in progress.
Here's [what's left to do](docs/BACKLOG.md).

---

Bring a compiler. Curiosity helps.

[Quality rules](docs/QUALITY.md) / [Apache 2.0](LICENSE)

