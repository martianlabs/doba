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

Both arguments have defaults. With TCP, set an IPv4 bind address and port
before calling *start*. The [hello_world source][hello-source] uses:

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

<a name="lifecycle"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/h2-lifecycle-dark.svg">
    <img src="../../../resources/docs/reference/h2-lifecycle.svg" alt="Lifecycle">
  </picture>
</h2>

```cpp
void start();
void stop();
~server();
```

- **start.** Starts the date service and transport. Calling it again while
  the server is running has no effect. A transport startup failure is
  propagated after the date service is stopped.
- **stop.** Returns immediately if already stopped. With TCP or TLS, it
  quiesces the transport, waits for pending async route handlers, then stops
  the transport and date service. Call it from a control thread, since the
  bundled transports reject quiescing from one of their workers.
- **Destructor.** Calls *stop*.

The [HTTPS example](../../../examples/http/v11/https_hello_world/README.md)
shows *start*, a signal wait, and an explicit *stop*. Its
[source](../../../examples/http/v11/https_hello_world/main.cpp) keeps the
shutdown point visible.

<a name="registration"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/h2-registration-dark.svg">
    <img src="../../../resources/docs/reference/h2-registration.svg" alt="Registration">
  </picture>
</h2>

Register routes and controllers before *start*:

```cpp
template <typename Hty>
server& add_route(std::string_view method, std::string_view route,
                  Hty handler);

template <typename CTty, typename... Args>
server& add_controller(Args&&... args);
```

- **add_route.** Registers a method, path, and handler with the router.
  The handler is moved into the router. The call returns *server&* so
  registrations can be chained.
- **add_controller.** Constructs a controller from forwarded arguments and
  asks it to register routes. Bound handlers keep that instance alive. The
  call also returns *server&*.

Both calls throw *std::runtime_error* while the server is running. The
router can reject an invalid route or controller registration; see the
[HTTP guide](../../guide/03-http-1-1.md) for supported route shapes.

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

Construction can fail for transport configuration, including TLS
certificates. *start* can fail when the listener cannot open. An engine
configuration error may surface when a connection creates its engine.
Route and controller registration may throw for invalid inputs, and both
reject registration after *start*. The server does not convert these setup
failures into HTTP responses.

Back to the [reference index](../README.md), or follow the
[getting started guide](../../guide/01-getting-started.md) for a complete
server you can run.
