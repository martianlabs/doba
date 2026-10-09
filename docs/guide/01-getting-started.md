<a name="getting-started"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h1-getting-started-dark.svg">
    <img src="../../resources/docs/guide/h1-getting-started.svg" alt="Getting started">
  </picture>
</h1>

This chapter builds the bundled `hello_world` example and sends a request to
it. Run the commands from the repository root. Its source is
[main.cpp](../../examples/http/v11/hello_world/main.cpp).

## Requirements

- Windows or Linux with a C++20 compiler.
- CMake 3.20 or later.
- `curl` to make the test request.

The TCP example does not require OpenSSL.

## Build the example

Configure a build directory and build only the `hello_world` target:

```sh
cmake -S . -B out/build/guide
cmake --build out/build/guide --config Release --target hello_world
```

The top-level build enables examples by default. Building this target does
not build the other examples or tests. `--config Release` applies to
multi-configuration generators.

## Run the server

Run the `hello_world` executable produced in the build tree. For example, with
a single-configuration generator on Linux:

```sh
./out/build/guide/examples/http/v11/hello_world/hello_world
```

With Visual Studio on Windows, the Release executable is normally at
`out\build\guide\examples\http\v11\hello_world\Release\hello_world.exe`.
Leave the server running while you make the request in another terminal.

```sh
curl -i http://localhost:8080/hello
```

The route returns `200 OK`, the `Server: doba.` and
`Content-Type: text/plain; charset=utf-8` headers, and the body `ok`.
Stop the server with Ctrl+C.

## Understand the example

The core of [main.cpp](../../examples/http/v11/hello_world/main.cpp) is short:

```cpp
server<> http_server({.ip = "0.0.0.0", .port = "8080"});
http_server.add_route("GET", "/hello", [](const request&, response& res) {
  res.ok_200();
  res.add_header("Server", "doba.")
      .add_header("Content-Type", "text/plain; charset=utf-8")
      .set_body("ok");
});
http_server.start();
signaler::wait();
```

`server<>` combines doba's HTTP/1.1 engine with its default TCP transport.
The route matches `GET /hello`. `start()` opens the listener; `signaler::wait()`
keeps this example running until a termination signal arrives.

Return to the [guide index](README.md) for the remaining chapters.
