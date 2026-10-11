<a name="server"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/reference/h1-server-dark.svg">
    <img src="../../resources/docs/reference/h1-server.svg" alt="Server">
  </picture>
</h1>

*server* joins a protocol engine to a concrete server transport. It has no
HTTP types. The declaration is in [server.h](../../include/server.h), in the
*martianlabs::doba* namespace:

```cpp
#include "server.h"
```

The [custom_protocol example](../../examples/protocol/custom_protocol/README.md)
shows a complete engine and its
[source](../../examples/protocol/custom_protocol/main.cpp) runs it over TCP.

<a name="type-and-defaults"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/reference/h2-type-and-defaults-dark.svg">
    <img src="../../resources/docs/reference/h2-type-and-defaults.svg" alt="Type and defaults">
  </picture>
</h2>

```cpp
template <protocol::contracts::engine ENty, typename TRty>
class server;
```

- **ENty.** The engine type for one connection. It must satisfy the
  [engine contract](../../include/protocol/contracts.h).
- **TRty.** A concrete transport type, already instantiated with *ENty* and
  its factory type. Its constructor must satisfy the
  [transport contract](../../include/transport/server/contracts.h).

The engine factory creates each connection's engine. For TCP, its type is the
second template argument of *transport::server::tcp*.

<a name="construction"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/reference/h2-construction-dark.svg">
    <img src="../../resources/docs/reference/h2-construction.svg" alt="Construction">
  </picture>
</h2>

```cpp
template <protocol::contracts::engine_factory<ENty> FNty>
  requires transport::server::contracts::transport<TRty, ENty, FNty>
explicit server(typename TRty::policies_type configuration,
                FNty create_engine);
```

**Parameters and template arguments:**

- **FNty.** The deduced type of the engine factory. It must be move
  constructible, callable without arguments when const, and return *ENty*.
- **configuration.** Policies for the chosen transport.
- **create_engine.** Factory moved into the transport. It must remain able to
  create engines while the server accepts connections.

**Returns:** No value; construction creates the server object.

**Errors:** Transport construction errors reach the caller. Configuration
errors may arise here or when the transport starts.

The example creates a transport type from its engine and factory, then gives
both to *server*:

```cpp
auto create_engine = []() { return custom_protocol{}; };
using transport_type = martianlabs::doba::transport::server::tcp<
    custom_protocol, decltype(create_engine)>;
martianlabs::doba::server<custom_protocol, transport_type> server(
    {.ip = "0.0.0.0", .port = "8080"}, create_engine);
```

<a name="lifecycle"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/reference/h2-lifecycle-dark.svg">
    <img src="../../resources/docs/reference/h2-lifecycle.svg" alt="Lifecycle">
  </picture>
</h2>

<a name="start"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/reference/h3-start-dark.svg">
    <img src="../../resources/docs/reference/h3-start.svg" alt="Start">
  </picture>
</h3>

```cpp
void start();
```

**Parameters:** None.

**Returns:** Nothing (*void*). A repeated call while running has no effect.

**Errors:** Transport startup exceptions reach the caller. After a failed
start, the server can be started again.

<a name="stop"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/reference/h3-stop-dark.svg">
    <img src="../../resources/docs/reference/h3-stop.svg" alt="Stop">
  </picture>
</h3>

```cpp
void stop();
```

**Parameters:** None.

**Returns:** Nothing (*void*). A repeated call while stopped has no effect.

The server quiesces transports that support it before stopping them. Call
*stop* from a control thread: TCP and TLS reject quiescing from a worker.

<a name="destructor"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/reference/h3-destructor-dark.svg">
    <img src="../../resources/docs/reference/h3-destructor.svg" alt="Destructor">
  </picture>
</h3>

```cpp
~server();
```

**Parameters:** None.

**Returns:** No value; destructors have no return type.

The destructor calls *stop*. The server owns its transport and cannot be
copied or moved. Keep any storage captured by the engine factory alive until
the server stops.

Back to the [reference index](README.md), or see the
[HTTP server](http/server.md) for routing and HTTPS.
