<a name="core-model"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h1-core-model-dark.svg">
    <img src="../../resources/docs/guide/h1-core-model.svg" alt="Core model">
  </picture>
</h1>

doba has two pieces that work together without knowing each other's internals:
a **protocol engine** and a **transport**. Swap one, keep the other. That is
how the same TCP transport can carry a new protocol, or the same HTTP/1.1
engine can run without a network socket.

<a name="protocol-plus-transport"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h2-protocol-plus-transport-dark.svg">
    <img src="../../resources/docs/guide/h2-protocol-plus-transport.svg" alt="Protocol plus transport">
  </picture>
</h2>

The **transport** handles connections and moves bytes. It passes incoming
bytes to an engine and sends the output that engine produces. The
**protocol engine** decides what those bytes mean: it consumes input, submits
output, and can ask to close the connection.

For TCP, each connection gets its own engine from a factory. Input callbacks
for a connection are serialized; output can be submitted from different
threads. The transport owns submitted output while it sends it. The required
methods on each side are checked by C++ concepts, so a missing piece shows up
at compile time.

<a name="the-default-pair"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h2-the-default-pair-dark.svg">
    <img src="../../resources/docs/guide/h2-the-default-pair.svg" alt="The default pair">
  </picture>
</h2>

You already used this pairing in the
[hello_world example](../../examples/http/v11/hello_world/README.md). Its
[source](../../examples/http/v11/hello_world/main.cpp) uses this line:

```cpp
server<> http_server({.ip = "0.0.0.0", .port = "8080"});
```

The empty template argument list picks the HTTP/1.1 engine and TCP transport.
The server ties them together. You register routes and start the listener. You
write the handler; the engine speaks HTTP, and TCP moves the bytes.

<a name="bring-a-protocol"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h2-bring-a-protocol-dark.svg">
    <img src="../../resources/docs/guide/h2-bring-a-protocol.svg" alt="Bring a protocol">
  </picture>
</h2>

The [custom_protocol example](../../examples/protocol/custom_protocol/README.md)
keeps TCP and replaces HTTP with a tiny line-based echo engine. Its
[source](../../examples/protocol/custom_protocol/main.cpp) creates one engine
for each connection:

```cpp
auto create_engine = []() { return custom_protocol{}; };
transport::tcp<custom_protocol, decltype(create_engine)> server(
    {.ip = "0.0.0.0", .port = "8080"}, create_engine);
```

The engine reads complete lines and submits an **echo:** response. TCP still
accepts clients, buffers incomplete input, and sends the output. The engine
only implements the protocol contract; it does not need socket code.

<a name="bring-a-transport"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h2-bring-a-transport-dark.svg">
    <img src="../../resources/docs/guide/h2-bring-a-transport.svg" alt="Bring a transport">
  </picture>
</h2>

The **custom_transport** demo keeps doba's HTTP/1.1 engine but trades TCP for
an in-memory transport. Start with its
[README](../../examples/transport/server/custom_transport/README.md); the
[source](../../examples/transport/server/custom_transport/main.cpp) supplies
that transport to the HTTP server:

```cpp
using routes_type =
    martianlabs::doba::protocol::http::router<http::request, http::response>;
using engine_type = http::engine<http::request, http::response>;

std::string output;
http::server<http::request, http::response, routes_type, engine_type,
             custom_transport>
    server(&output);
```

This transport creates one engine, feeds it a fixed HTTP request, and copies
the response into a string. No network listener opens. It is a focused demo
of the boundary, not a general HTTP middleware API.

That is the core idea: **protocols decide what bytes mean; transports move
them.** The two examples show you can change either side without rebuilding
the other.

Head back to the [guide index](README.md) when you're ready for more.
