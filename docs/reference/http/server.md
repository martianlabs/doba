<a name="http-server"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/h1-http-server-dark.svg">
    <img src="../../../resources/docs/reference/h1-http-server.svg" alt="HTTP server">
  </picture>
</h1>

*server* composes an HTTP/1.1 router and protocol engine with a server
transport. The declaration lives in
[server.h](../../../include/protocol/http/v11/server.h), in the
*martianlabs::doba::protocol::http::v11* namespace:

```cpp
#include "protocol/http/v11/server.h"
```

For a runnable first use, open the
[hello_world example](../../../examples/http/v11/hello_world/README.md) and
its [source](../../../examples/http/v11/hello_world/main.cpp).

<a name="type-and-defaults"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/h2-type-and-defaults-dark.svg">
    <img src="../../../resources/docs/reference/h2-type-and-defaults.svg" alt="Type and defaults">
  </picture>
</h2>

The template parameters choose the request, response, router, engine, and
transport types:

```cpp
template <typename RQty = request, typename RSty = response,
          typename ROty = router<RQty, RSty>,
          protocol::contracts::engine ENty = engine<RQty, RSty, ROty>,
          template <typename, typename> class TRty = transport::server::tcp>
  requires contracts::server<ROty, ENty, TRty>
class server;
```

- **RQty.** Request type; defaults to *request*.
- **RSty.** Response type; defaults to *response*.
- **ROty.** Router type; defaults to *router<RQty, RSty>*.
- **ENty.** Protocol engine type; defaults to *engine<RQty, RSty, ROty>*.
- **TRty.** Transport template; defaults to TCP.

**server<>** therefore selects doba's HTTP/1.1 types and TCP transport.
The [core model](../../guide/02-core-model.md) explains how the engine and
transport meet; the [extending guide](../../guide/05-extending-doba.md)
shows how to replace either side.

<a name="construction"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/h2-construction-dark.svg">
    <img src="../../../resources/docs/reference/h2-construction.svg" alt="Construction">
  </picture>
</h2>

The constructor takes transport policies first and engine policies second:

```cpp
explicit server(
    typename TRty<ENty, engine_factory<ENty, ROty>>::policies_type
        transport_configuration = {},
    typename ENty::policies_type engine_configuration = {});
```

**Parameters:**

- **transport_configuration.** Policies for the selected transport; defaults
  to an empty policy object. With TCP, set an IPv4 bind address and port
  before calling *start*.
- **engine_configuration.** Policies passed to each new HTTP engine; also
  defaults to an empty policy object.

**Returns:** No value; construction creates the server object.

The [hello_world source][hello-source] uses:

[hello-source]: ../../../examples/http/v11/hello_world/main.cpp

```cpp
server<> http_server({.ip = "0.0.0.0", .port = "8080"});
```

The [request_limits example][request-limits] passes HTTP policies as the
second argument; its
[source](../../../examples/http/v11/request_limits/main.cpp) sets a body
limit of five bytes. The [limits guide](../../guide/06-limits-and-operation.md)
explains the two policy layers.

[request-limits]: ../../../examples/http/v11/request_limits/README.md

Invalid transport configuration, including TLS certificates, can make
construction fail. An engine configuration error may surface later, when a
connection creates its engine.

<a name="lifecycle"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/h2-lifecycle-dark.svg">
    <img src="../../../resources/docs/reference/h2-lifecycle.svg" alt="Lifecycle">
  </picture>
</h2>

<a name="start"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/h3-start-dark.svg">
    <img src="../../../resources/docs/reference/h3-start.svg" alt="Start">
  </picture>
</h3>

```cpp
void start();
```

**Parameters:** None.

**Returns:** Nothing (*void*).

Starts the date service and transport. Calling *start* again while the server
is running has no effect. If transport startup throws, the date service is
stopped and the exception reaches the caller. For TCP or TLS, this includes
listener startup failures.

<a name="stop"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/h3-stop-dark.svg">
    <img src="../../../resources/docs/reference/h3-stop.svg" alt="Stop">
  </picture>
</h3>

```cpp
void stop();
```

**Parameters:** None.

**Returns:** Nothing (*void*).

Returns immediately if already stopped. With TCP or TLS, *stop* quiesces the
transport, waits for pending async route handlers, then stops the transport
and date service. Call it from a control thread: the bundled transports
reject quiescing from one of their workers.

<a name="destructor"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/h3-destructor-dark.svg">
    <img src="../../../resources/docs/reference/h3-destructor.svg" alt="Destructor">
  </picture>
</h3>

```cpp
~server();
```

**Parameters:** None.

**Returns:** No value; destructors have no return type.

The destructor calls *stop*. Explicitly calling *stop* keeps the shutdown
point and any shutdown errors under your control.

The [HTTPS example](../../../examples/http/v11/https_hello_world/README.md)
shows *start*, a signal wait, and an explicit *stop*. See its
[source](../../../examples/http/v11/https_hello_world/main.cpp).

<a name="registration"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/h2-registration-dark.svg">
    <img src="../../../resources/docs/reference/h2-registration.svg" alt="Registration">
  </picture>
</h2>

Register routes and controllers before *start*:

<a name="add-route"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/h3-add-route-dark.svg">
    <img src="../../../resources/docs/reference/h3-add-route.svg" alt="Add route">
  </picture>
</h3>

```cpp
template <typename Hty>
  requires router_handler_lambda<Hty>
server& add_route(std::string_view method, std::string_view route,
                  Hty handler);
```

**Parameters and template arguments:**

- **Hty.** Deduced from *handler*; it must satisfy *router_handler_lambda*.
- **method.** The HTTP method to match, such as *GET*.
- **route.** The path pattern to match. See the
  [HTTP guide](../../guide/03-http-1-1.md) for supported route shapes.
- **handler.** A compatible handler; the router stores it by moving it from
  this argument.

**Returns:** A reference to this server, so registrations can be chained.

**Errors:** Throws *std::runtime_error* if the server is running. The router
throws *std::invalid_argument* for invalid wildcard placement or a mismatch
between route parameters and handler arguments.

<a name="add-controller"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/h3-add-controller-dark.svg">
    <img src="../../../resources/docs/reference/h3-add-controller.svg" alt="Add controller">
  </picture>
</h3>

```cpp
template <typename CTty, typename... Args>
server& add_controller(Args&&... args);
```

**Parameters and template arguments:**

- **CTty.** The controller type. It must provide *register_routes*.
- **Args and args.** The deduced argument types and values forwarded to the
  controller constructor.

The router owns the new controller instance, and bound handlers keep it
alive.

**Returns:** A reference to this server, so registrations can be chained.

**Errors:** Throws *std::runtime_error* if the server is running or
*std::invalid_argument* if the controller registers no routes. Exceptions
from construction or route registration propagate; failed registration rolls
back routes added by that call.

<a name="ownership-and-errors"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/h2-ownership-and-errors-dark.svg">
    <img src="../../../resources/docs/reference/h2-ownership-and-errors.svg" alt="Ownership and errors">
  </picture>
</h2>

The server owns its router and transport. It cannot be copied or moved.
Handlers stored by the router may still capture outside objects; keep any
captured references valid while requests can use them.

Setup failures reach the caller; the server does not convert them into HTTP
responses.

Back to the [reference index](../README.md), or follow the
[getting started guide](../../guide/01-getting-started.md) for a complete
server you can run.
