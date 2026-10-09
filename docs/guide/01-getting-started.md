<a name="getting-started"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h1-getting-started-dark.svg">
    <img src="../../resources/docs/guide/h1-getting-started.svg" alt="Getting started">
  </picture>
</h1>

Let's get doba to say hello. We'll build the bundled **hello_world** example,
run it, and send it a request with **curl**. Start from the repository root.

Want the full example? Open the
[hello_world README](../../examples/http/v11/hello_world/README.md) or jump
straight to [main.cpp](../../examples/http/v11/hello_world/main.cpp).

<a name="before-you-start"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h2-before-you-start-dark.svg">
    <img src="../../resources/docs/guide/h2-before-you-start.svg" alt="Before you start">
  </picture>
</h2>

You only need:

- A C++20 compiler on Windows or Linux.
- CMake 3.20 or later.
- **curl** for the test request.

No OpenSSL setup this time - this example uses plain TCP.

<a name="build-it"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h2-build-it-dark.svg">
    <img src="../../resources/docs/guide/h2-build-it.svg" alt="Build it">
  </picture>
</h2>

Two commands:

```sh
cmake -S . -B out/build/guide
cmake --build out/build/guide --config Release --target hello_world
```

Examples are enabled by default when you build doba from the repository root.
The **hello_world** target keeps the build focused on this example.
The *--config Release* option applies to multi-configuration generators.

<a name="run-it"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h2-run-it-dark.svg">
    <img src="../../resources/docs/guide/h2-run-it.svg" alt="Run it">
  </picture>
</h2>

Start the executable from the build tree. With a single-configuration generator
on Linux, that looks like this:

```sh
./out/build/guide/examples/http/v11/hello_world/hello_world
```

With Visual Studio on Windows, the Release executable is normally here:

```powershell
.\out\build\guide\examples\http\v11\hello_world\Release\hello_world.exe
```

Keep it running and open another terminal. Then ask for **/hello**:

```sh
curl -i http://localhost:8080/hello
```

You should get **200 OK**, the **Server: doba.** and
**Content-Type: text/plain; charset=utf-8** headers, and the body **ok**.
Stop the server with Ctrl+C when you're done.

<a name="under-the-hood"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h2-under-the-hood-dark.svg">
    <img src="../../resources/docs/guide/h2-under-the-hood.svg" alt="Under the hood">
  </picture>
</h2>

Here is the bit doing the work in
[main.cpp](../../examples/http/v11/hello_world/main.cpp):

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

**server<>** pairs doba's HTTP/1.1 engine with its default TCP transport.
**add_route** matches **GET /hello**; the response sets its status, headers,
and body. **start()** opens the listener; **signaler::wait()** keeps it alive
until Ctrl+C.

That's your first doba server. Head back to the [guide index](README.md) when
you're ready for more.
