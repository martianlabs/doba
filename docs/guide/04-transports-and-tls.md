<a name="transports-and-tls"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h1-transports-and-tls-dark.svg">
    <img src="../../resources/docs/guide/h1-transports-and-tls.svg" alt="Transports and TLS">
  </picture>
</h1>

Your HTTP handlers deal with requests and responses. The transport deals with
the socket underneath them. Start with TCP; when the connection needs
encryption, swap in TLS and keep the HTTP side as it is.

<a name="tcp-by-default"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h2-tcp-by-default-dark.svg">
    <img src="../../resources/docs/guide/h2-tcp-by-default.svg" alt="TCP by default">
  </picture>
</h2>

**server<>** uses the TCP transport unless you choose another one. The
[hello_world example](../../examples/http/v11/hello_world/README.md) gives
it an IPv4 address and port, then starts the listener. Here is the relevant
bit of [main.cpp](../../examples/http/v11/hello_world/main.cpp):

```cpp
server<> http_server({.ip = "0.0.0.0", .port = "8080"});
http_server.add_route("GET", "/hello", [](const request&, response& res) {
  res.ok_200();
  res.add_header("Server", "doba.")
      .add_header("Content-Type", "text/plain; charset=utf-8")
      .set_body("ok");
});
http_server.start();
```

**0.0.0.0** listens on all IPv4 interfaces; use **127.0.0.1** when the
server should accept local connections only. TCP moves bytes between each
connection and its protocol engine. On Windows, its workers use IOCP; on
Linux, they use epoll. Your handlers do not need a platform-specific path.

<a name="tcp-policies"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h2-tcp-policies-dark.svg">
    <img src="../../resources/docs/guide/h2-tcp-policies.svg" alt="TCP policies">
  </picture>
</h2>

The first **server<>** constructor argument is a *tcp_policies* value. It
sets up the listener and its workers. The
[policy definition](../../include/transport/server/tcp_policies.h) has the
full list; the settings you are most likely to touch are:

- **Address.** *ip* is the IPv4 bind address, and *port* is a string holding
  a port from **1** to **65535**. Set both before starting the server.
- **Workers.** *worker_count* chooses the worker count. **0** lets doba pick
  one based on hardware concurrency.
- **Queue.** *listen_backlog* requests the pending connection queue size.
  **0** leaves that choice to the system.
- **Buffers.** *recv_buffer_size* gives each connection its receive capacity;
  *max_send_buffer_size* caps its queued send reservations and active batch.
  Both must be positive.

You can pass a named policy value when a designated initializer starts to
feel crowded:

```cpp
martianlabs::doba::transport::server::tcp_policies configuration;
configuration.ip = "127.0.0.1";
configuration.port = "8080";
configuration.worker_count = 2;
martianlabs::doba::protocol::http::v11::server<> http_server(configuration);
```

Keep the default buffer sizes until you have a concrete reason to change
them; the limits chapter covers resource boundaries in more detail.

<a name="switch-on-tls"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h2-switch-on-tls-dark.svg">
    <img src="../../resources/docs/guide/h2-switch-on-tls.svg" alt="Switch on TLS">
  </picture>
</h2>

TLS is an optional transport backed by OpenSSL. The
[https_hello_world example](../../examples/http/v11/https_hello_world/README.md)
uses the same HTTP/1.1 request, response, and route APIs as the TCP example.
Its [main.cpp](../../examples/http/v11/https_hello_world/main.cpp) selects
the TLS transport explicitly:

```cpp
martianlabs::doba::transport::server::tls_policies configuration;
configuration.ip = "0.0.0.0";
configuration.port = "8443";
configuration.certificate_file = argv[1];
configuration.private_key_file = argv[2];
server<request, response,
       martianlabs::doba::protocol::http::router<request, response>,
       engine<request, response>,
       martianlabs::doba::transport::server::tls> http_server(configuration);
```

*tls_policies* includes the TCP settings plus paths to a PEM certificate
chain and its matching PEM private key. doba checks that both files load and
the key matches the certificate when it creates the TLS transport. A bad
pair fails server construction instead of waiting for the first request.

<a name="try-https"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h2-try-https-dark.svg">
    <img src="../../resources/docs/guide/h2-try-https.svg" alt="Try HTTPS">
  </picture>
</h2>

From the repository root, enable TLS and build the example:

```sh
cmake -S . -B out/build/tls -DDOBA_ENABLE_TLS=ON
cmake --build out/build/tls --config Release --target https_hello_world
```

CMake needs the OpenSSL development headers and libraries. On Windows with
vcpkg, pass its *CMAKE_TOOLCHAIN_FILE* path to the configure command. If you
consume an installed doba package, link *martianlabs::doba_tls* for this
transport.

Have a PEM certificate chain and matching key ready. On Linux with a
single-configuration generator, start the example like this:

```sh
./out/build/tls/examples/http/v11/https_hello_world/https_hello_world \
  server.crt server.key
```

With Visual Studio on Windows, the Release executable is normally here:

```powershell
$build = '.\out\build\tls\examples\http\v11\https_hello_world\Release'
& "$build\https_hello_world.exe" server.crt server.key
```

In another terminal, request the route:

```sh
curl -i https://localhost:8443/hello
```

You should get **200 OK** and **hello from doba**. For a self-signed
certificate used in local testing, add *--insecure* to that **curl** command.
Stop the server with Ctrl+C when you are done.

The HTTP code stays familiar whether the bytes arrive over TCP or TLS. Head
back to the [guide index](README.md) when you are ready to extend doba.
