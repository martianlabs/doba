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
template <protocol::contracts::engine_factory<ENty> EFy>
  requires transport::server::contracts::transport<TRty, ENty, EFy>
explicit server(typename TRty::policies_type configuration,
                EFy factory);
```

**Parameters and template arguments:**

- **EFy.** The deduced type of the engine factory. It must be move
  constructible, callable without arguments when const, and return *ENty*.
- **configuration.** Policies for the chosen transport.
- **factory.** Factory retained until the first *start*, then moved into
  the transport. It must remain able to create engines while the server
  accepts connections.

**Returns:** No value; construction creates the server object.

**Errors:** Storing the configuration and factory may throw. Transport
construction and configuration errors arise on the first *start*.

Construction does not create the transport or start accepting connections.

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

**Errors:** Transport construction and startup exceptions reach the caller.
After a transport startup failure, the server permits another *start* call,
but does not clean up the failed transport. A custom transport must restore
its stopped state for a successful retry. If transport construction throws,
this server instance cannot retry it.

A derived server subscribes during construction. *starting* fires before the
transport is constructed or started. *start_failed* fires if that callback,
transport construction, or transport startup throws. The transport is
constructed only on the first *start* and retained across *stop* and restart.
Repeated *start* calls while running emit no events.

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

**Errors:** Transport and lifecycle callback exceptions reach the caller.

The server quiesces transports that support it before stopping them. Call
*stop* from a control thread: TCP and TLS reject quiescing from a worker.
*stopping* fires after quiescing, or after *stop* when the transport cannot
quiesce. *stopped* fires after the transport stops. Lifecycle callbacks run
under the server's mutex and must not call *start* or *stop* again.

Derived classes register lifecycle callbacks through these protected
methods. Register them during construction, before *start*. Each method
stores one callback; another call replaces it, and an empty callback clears
it. The setters are not synchronized with lifecycle calls. If a callback
captures derived state, stop the server before that state is destroyed.

<a name="set-on-starting"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/reference/h3-set-on-starting-dark.svg">
    <img src="../../resources/docs/reference/h3-set-on-starting.svg" alt="Set on starting">
  </picture>
</h3>

```cpp
void set_on_starting(std::function<void()> callback);
```

**Parameters:** **callback.** The function called when this event occurs.

**Returns:** Nothing (*void*).

**Errors:** Callback exceptions propagate from *start*.

Registers the callback invoked before transport construction. If it throws,
*start* invokes *start_failed* and propagates the exception.

<a name="set-on-start-failed"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/reference/h3-set-on-start-failed-dark.svg">
    <img src="../../resources/docs/reference/h3-set-on-start-failed.svg" alt="Set on start failed">
  </picture>
</h3>

```cpp
void set_on_start_failed(std::function<void()> callback);
```

**Parameters:** **callback.** The function called when this event occurs.

**Returns:** Nothing (*void*).

**Errors:** Callback exceptions propagate from *start* and replace the original
startup exception.

Registers the callback invoked when *starting*, transport construction, or
transport startup throws. The server remains stopped. It can be started again
after a failure in *starting* or transport startup.

<a name="set-on-stopping"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/reference/h3-set-on-stopping-dark.svg">
    <img src="../../resources/docs/reference/h3-set-on-stopping.svg" alt="Set on stopping">
  </picture>
</h3>

```cpp
void set_on_stopping(std::function<void()> callback);
```

**Parameters:** **callback.** The function called when this event occurs.

**Returns:** Nothing (*void*).

**Errors:** Callback exceptions propagate from *stop* and may prevent shutdown
from completing.

Registers the callback invoked after transport quiescing, before the
transport stops. If the transport has no *quiesce* method, it runs after
the transport stops.

<a name="set-on-stopped"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/reference/h3-set-on-stopped-dark.svg">
    <img src="../../resources/docs/reference/h3-set-on-stopped.svg" alt="Set on stopped">
  </picture>
</h3>

```cpp
void set_on_stopped(std::function<void()> callback);
```

**Parameters:** **callback.** The function called when this event occurs.

**Returns:** Nothing (*void*).

**Errors:** Callback exceptions propagate from *stop*.

Registers the callback invoked after the transport stops and the
server is marked stopped.

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
