<a name="http-server"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/h1-http-server-dark.svg">
    <img src="../../../resources/docs/reference/h1-http-server.svg" alt="HTTP server">
  </picture>
</h1>

*http_server* composes an HTTP/1.1 router and engine with TCP by default.
*https_server* uses the same HTTP behavior with TLS. Both live in the
*martianlabs::doba* namespace and build on the
[core server](../server.md):

```cpp
#include "http_server.h"
#include "https_server.h"  // When TLS is enabled.
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

The template parameters choose the request, response, router, and engine.
*http_server* also accepts a transport template for custom transports:

```cpp
template <typename RQty = protocol::http::v11::request,
          typename RSty = protocol::http::v11::response,
          typename ROty = protocol::http::router<RQty, RSty>,
          protocol::contracts::engine ENty =
              protocol::http::v11::engine<RQty, RSty, ROty>,
          template <typename, typename> class TRty = transport::server::tcp,
          typename... AUty>
  requires contracts::http_server<RQty, RSty, ROty, ENty, TRty> &&
           (protocol::http::auth::contracts::provider<AUty, RQty> && ...)
class http_server;

template <typename RQty = protocol::http::v11::request,
          typename RSty = protocol::http::v11::response,
          typename ROty = protocol::http::router<RQty, RSty>,
          protocol::contracts::engine ENty =
              protocol::http::v11::engine<RQty, RSty, ROty>,
          typename... AUty>
class https_server;
```

- **RQty.** Request type; defaults to *request*.
- **RSty.** Response type; defaults to *response*.
- **ROty.** Router type; defaults to *router<RQty, RSty>*.
- **ENty.** Protocol engine type; defaults to *engine<RQty, RSty, ROty>*.
- **TRty.** Transport template; defaults to TCP.
- **AUty.** Authentication provider types; empty by default. Each satisfies
  **auth::contracts::provider<AUty, RQty>**.

The [HTTP server contract](../../../include/contracts.h) checks that the
request, response, and router meet their HTTP/1.1 contracts and that the
engine and transport can be composed.

**http_server<>** selects doba's HTTP/1.1 types and TCP transport.
**https_server<>** selects the same HTTP types and the TLS transport. Both
accept custom request, response, and router types.
Those types must satisfy the
[HTTP/1.1 contracts](../../../include/protocol/http/v11/contracts.h)
for request, response, and router.
Requests and responses provide *from* factories; the router registers and
selects routes. Its *match* method returns
[*router_match_result<RQty, RSty>*](../../../include/protocol/http/common/router_match_result.h).
The result holds non-owning handler and authentication callback pointers. Keep selected handlers alive
while the engine may use them, including pending async work. Each contract
checks the public API of the corresponding doba type. These requirements also
apply when you provide a custom engine. A custom engine used by *http_server*
accepts its policies and router. If it leaves work pending after a connection
closes, it must also implement the optional
[*completion_engine* contract](../../../include/protocol/contracts.h) so the
transport retains that connection until the work finishes.

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

Both server types take transport policies first and engine policies second.
The existing constructor applies when the provider pack is empty:

```cpp
explicit http_server(
    typename TRty<ENty,
                  protocol::http::v11::engine_factory<ENty, ROty>>::policies_type
        transport_configuration = {},
    typename ENty::policies_type engine_configuration = {})
  requires(sizeof...(AUty) == 0);

// https_server inherits the constructor with TRty fixed to TLS.
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
http_server<> http_server({.ip = "0.0.0.0", .port = "8080"});
```

The [HTTPS example](../../../examples/http/v11/https_hello_world/README.md)
passes TLS policies to *https_server<>*; its
[source](../../../examples/http/v11/https_hello_world/main.cpp) supplies the
certificate and private key.

The [request_limits example][request-limits] passes HTTP policies as the
second argument; its
[source](../../../examples/http/v11/request_limits/main.cpp) sets a body
limit of five bytes. The [limits guide](../../guide/06-limits-and-operation.md)
explains the two policy layers.

[request-limits]: ../../../examples/http/v11/request_limits/README.md

The constructor creates the router but not the transport. Invalid transport
configuration, including TLS certificates, can make the first *start* fail.
An engine configuration error may surface later, when a connection creates
its engine.

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

Starts the date service, creates the transport on the first call, and starts
it. Calling *start* again while the server is running has no effect. If
transport construction or startup throws, the date service is stopped and the
exception reaches the caller. For TCP or TLS, this includes listener startup
failures. A transport construction failure cannot be retried on that server
instance.

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
transport, waits for engines with pending work to complete, then stops the
transport and date service. Call it from a control thread: the bundled
transports reject quiescing from one of their workers.

<a name="destructor"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/h3-destructor-dark.svg">
    <img src="../../../resources/docs/reference/h3-destructor.svg" alt="Destructor">
  </picture>
</h3>

```cpp
~http_server();
```

**Parameters:** None.

**Returns:** No value; destructors have no return type.

The *http_server* destructor calls *stop* before destroying the router.
*https_server* inherits this shutdown. Explicitly calling *stop* keeps the
shutdown point and any shutdown errors under your control.

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
http_server& add_route(std::string_view method, std::string_view route,
                       Hty handler);

template <typename Aty, typename Hty>
  requires router_handler_lambda<Hty> && (std::same_as<Aty, AUty> || ...)
http_server& add_route(std::string_view method, std::string_view route,
                       Hty handler);
```

**Parameters and template arguments:**

- **Hty.** Deduced from *handler*; it must satisfy *router_handler_lambda*.
- **Aty.** Optional explicit provider type, occurring exactly once in **AUty**.
  Omit it for a public route.
- **method.** The HTTP method to match, such as *GET*.
- **route.** The path pattern to match. See the
  [HTTP guide](../../guide/03-http-1-1.md) for supported route shapes.
- **handler.** A compatible handler; the router stores it by moving it from
  this argument.

The authenticated overload calls the provider once before the handler.
Only an accepted result runs it. Rejections use the engine's normal response
ordering. Static, parametrized, and wildcard routes support authentication
with synchronous or coroutine handlers; route precedence is unchanged.

**Returns:** A reference to this server, so registrations can be chained.

**Errors:** Throws *std::runtime_error* if the server is running. The router
throws *std::invalid_argument* for invalid wildcard placement or a mismatch
between route parameters and handler arguments.

<a name="authentication"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/h2-authentication-dark.svg">
    <img src="../../../resources/docs/reference/h2-authentication.svg" alt="Authentication">
  </picture>
</h2>

The [authentication reference](authentication.md) describes Basic, Bearer,
custom providers, rejection results, and concurrency requirements. The
[Basic example](../../../examples/http/v11/basic_auth/README.md) and
[Bearer example](../../../examples/http/v11/bearer_auth/README.md) run complete
servers.

The additional constructor accepts provider instances. HTTPS inherits it
with the transport fixed to TLS:

```cpp
explicit http_server(
    typename TRty<ENty,
                  protocol::http::v11::engine_factory<ENty, ROty>>::policies_type
        transport_configuration,
    typename ENty::policies_type engine_configuration,
    std::tuple<AUty...> authentication);
```

**Parameters:** Transport and engine policies follow the existing
constructor. **authentication** supplies provider instances in template
order, moved into the server. This constructor has no default arguments.

**Returns:** No value. The server owns providers until it has stopped all
pending requests. Captured dependencies must remain valid for that period.

**Errors:** Providers must satisfy the method contract and be movable.
Selecting an absent or repeated provider type for a route fails compilation.

A server with Basic uses:

```cpp
#include <tuple>

#include "http_server.h"
#include "protocol/http/common/auth/basic.h"

namespace doba = martianlabs::doba;
namespace auth = doba::protocol::http::auth;
using namespace doba::protocol::http::v11;
using routes_type = doba::protocol::http::router<request, response>;
using engine_type = engine<request, response>;
using server_type = doba::http_server<request, response, routes_type,
    engine_type, doba::transport::server::tcp, auth::basic>;

server_type server({.ip = "127.0.0.1", .port = "8080"}, {},
    std::tuple{auth::basic{"demo", [](auto user, auto password) {
      return user == "alice" && password == "secret";
    }}});
```

For HTTPS the pack follows the engine argument:
**https_server<request, response, routes_type, engine_type, auth::basic>**.
A custom router used for authenticated routes must also support
**add(method, route, handler, authenticate)** and return a pointer to the
stored callback in **router_match_result**. Existing custom routers used
only for public routes retain their contract.

<a name="add-controller"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/h3-add-controller-dark.svg">
    <img src="../../../resources/docs/reference/h3-add-controller.svg" alt="Add controller">
  </picture>
</h3>

```cpp
template <typename CTty, typename... Args>
http_server& add_controller(Args&&... args);
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
