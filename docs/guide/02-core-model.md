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

<a name="protocol-contract"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h2-protocol-contract-dark.svg">
    <img src="../../resources/docs/guide/h2-protocol-contract.svg" alt="Protocol contract">
  </picture>
</h2>

A protocol engine understands the bytes for one connection. The
[engine contract](../../include/protocol/contracts.h) is a C++ concept that
checks four pieces:

- **Policies.** *policies_type* names the engine's policy type. It can be
  empty when the engine needs no options.
- **Output.** *set_on_send* receives a callback from the transport. The engine
  calls it when it has bytes to send.
- **Closure.** *set_on_close* receives a callback the engine can use to ask
  the transport to close the connection.
- **Input.** *on_bytes_received* gets a buffer pointer, the number of bytes
  available, and the buffer's total capacity. It returns the number of bytes
  consumed. TCP keeps the rest for the next read.

Here are the three calls as the concept checks them:

```cpp
{ engine.set_on_send(std::move(output)) } -> std::same_as<void>;
{ engine.set_on_close(std::move(close)) } -> std::same_as<void>;
{ engine.on_bytes_received(buf, sze, capacity) } -> std::same_as<std::size_t>;
```

The [send delegate](../../include/protocol/send_delegate.h) is that output
callback. The engine gives it three things:

- **Head and body:** two *std::string_view* chunks to send in that order.
- **Source:** an optional *[reader](../../include/common/reader.h)* for more
  body bytes. The transport takes ownership of the reader.

Its type is:

```cpp
using send_delegate = std::function<void(std::string_view, std::string_view,
                                         std::unique_ptr<common::reader>)>;
```

The transport owns and drains submitted output. The two views do not own
their bytes, so the transport must preserve their contents if it sends them
later.

The transport needs a fresh engine for each connection. An **engine factory**
is the callable you give it to create that engine. TCP calls the factory when
a connection opens, so you choose how to build the engine without putting
that setup in TCP. The [factory contract](../../include/protocol/contracts.h)
requires a move-constructible callable that takes no arguments even when const
and returns the engine type.

Input calls for a connection are serialized and stop when closing begins.
Output may be submitted from different threads, so the transport must accept
it safely.

<a name="transport-contract"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h2-transport-contract-dark.svg">
    <img src="../../resources/docs/guide/h2-transport-contract.svg" alt="Transport contract">
  </picture>
</h2>

The [transport contract](../../include/transport/server/contracts.h) checks
how a transport is created and controlled:

- **Policies.** *policies_type* names the transport's policy type. A value of
  that type is the first constructor argument.
- **Engine factory.** The factory is the second constructor argument and lets
  the transport create an engine for each connection.
- **Connection events.** *set_on_connection* and *set_on_disconnection* take
  callbacks to announce when a connection opens or closes.
- **Lifecycle.** *start* begins transport work; *stop* ends it.

These are the callback and lifecycle calls the concept checks:

```cpp
{ tr.set_on_connection(std::move(callback)) } -> std::same_as<void>;
{ tr.set_on_disconnection(std::move(callback)) } -> std::same_as<void>;
{ tr.start() } -> std::same_as<void>;
{ tr.stop() } -> std::same_as<void>;
```

A working transport creates an engine for each connection, wires up its send
and close callbacks, feeds it input, and handles its output. The concept
checks the required C++ shape; it cannot check that bytes stay alive long
enough or that those runtime steps happen in the right order.

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

The [custom_transport example][custom-transport] keeps doba's HTTP/1.1 engine
but trades TCP for an in-memory transport. Its
[source](../../examples/transport/server/custom_transport/main.cpp) supplies
that transport to the HTTP server:

[custom-transport]: ../../examples/transport/server/custom_transport/README.md

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
