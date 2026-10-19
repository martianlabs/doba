<a id="http-server-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/reference/http_server/index-dark.svg">
    <img src="../../resources/docs/reference/http_server/index.svg" alt="Index for http_server.h">
  </picture>
</h1>

**Header:** [include/http_server.h](../../include/http_server.h)

Combines an HTTP/1.1 router and protocol engine with a server transport. TCP is
the default transport.

**doba**

- [http_server](#type-http-server) · *type*

**http_server**

- [http_server(transport_configuration, engine_configuration)](#constructor-http-server) · *constructor*
- [http_server(transport_configuration, engine_configuration, authentication)](#constructor-http-server-2) · *constructor*
- [~http_server](#destructor-destructor-http-server) · *destructor*
- [add_route&lt;Hty&gt;(method, route, handler)](#method-add-route) · *method*
- [add_route&lt;Aty, Hty&gt;(method, route, handler)](#method-add-route-2) · *method*
- [add_controller](#method-add-controller) · *method*

<a id="type-http-server"></a>

## http_server

🟠 **TYPE** · *doba*

**Declaration synopsis:**

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
class http_server
    : public server<
          ENty, TRty<ENty, protocol::http::v11::engine_factory<ENty, ROty>>> { ... };
```

Combines routing, the HTTP/1.1 engine, and the selected transport in one server.

**Template parameters:**

- **RQty:** `typename RQty = protocol::http::v11::request`.
- **RSty:** `typename RSty = protocol::http::v11::response`.
- **ROty:** `typename ROty = protocol::http::router<RQty, RSty>`.
- **ENty:** `protocol::contracts::engine ENty = protocol::http::v11::engine<RQty, RSty, ROty>`.
- **TRty:** `template <typename, typename> class TRty = transport::server::tcp`.
- **AUty:** `typename... AUty`.

**Source:** [include/http_server.h:62](../../include/http_server.h#L62)

<a id="constructor-http-server"></a>

## http_server

🟠 **CONSTRUCTOR** · *http_server*

```cpp
explicit http_server(
      typename TRty<ENty, protocol::http::v11::engine_factory<ENty, ROty>>::
          policies_type transport_configuration = {},
      typename ENty::policies_type engine_configuration = {})
    requires(sizeof...(AUty) == 0);
```

Builds an HTTP/1.1 server without authentication providers. Both configuration arguments default to their policy defaults.

**Parameters:**

- **transport_configuration:** Transport policy values used to build the server. Declaration: `typename TRty<ENty, protocol::http::v11::engine_factory<ENty, ROty>>:: policies_type transport_configuration = {}`.
- **engine_configuration:** Protocol-engine policy values used to build the server. Declaration: `typename ENty::policies_type engine_configuration = {}`.

**Source:** [include/http_server.h:69](../../include/http_server.h#L69)

<a id="constructor-http-server-2"></a>

## http_server

🟠 **CONSTRUCTOR** · *http_server*

```cpp
explicit http_server(
      typename TRty<ENty, protocol::http::v11::engine_factory<ENty, ROty>>::
          policies_type transport_configuration,
      typename ENty::policies_type engine_configuration,
      std::tuple<AUty...> authentication);
```

Builds the transport and HTTP/1.1 engine configuration and stores the authentication providers. Route registration remains open until the server starts.

**Parameters:**

- **transport_configuration:** Transport policy values used to build the server. Declaration: `typename TRty<ENty, protocol::http::v11::engine_factory<ENty, ROty>>:: policies_type transport_configuration`.
- **engine_configuration:** Protocol-engine policy values used to build the server. Declaration: `typename ENty::policies_type engine_configuration`.
- **authentication:** Authentication providers stored by the server. Declaration: `std::tuple<AUty...> authentication`.

**Source:** [include/http_server.h:77](../../include/http_server.h#L77)

<a id="destructor-destructor-http-server"></a>

## ~http_server

🟠 **DESTRUCTOR** · *http_server*

```cpp
~http_server();
```

Stops the server before destroying it.

**Source:** [include/http_server.h:118](../../include/http_server.h#L118)

<a id="method-add-route"></a>

## add_route

🟠 **METHOD** · *http_server*

```cpp
template <typename Hty>
    requires protocol::http::router_handler_lambda<Hty>
  http_server& add_route(std::string_view method, std::string_view route,
                         Hty handler);
```

Registers an HTTP method and route pattern with a handler.

**Template parameters:**

- **Hty:** `typename Hty`.

**Parameters:**

- **method:** HTTP method to match. Declaration: `std::string_view method`.
- **route:** Route pattern to match. Declaration: `std::string_view route`.
- **handler:** Callable invoked for a matching route. Declaration: `Hty handler`.

**Returns:** A reference to this object, for chaining calls.

**Errors:** Throws `std::runtime_error` if registration has closed because the server is running. Router or controller construction errors may propagate.

**Source:** [include/http_server.h:126](../../include/http_server.h#L126)

<a id="method-add-route-2"></a>

## add_route

🟠 **METHOD** · *http_server*

```cpp
template <typename Aty, typename Hty>
    requires protocol::http::router_handler_lambda<Hty> &&
             (std::same_as<Aty, AUty> || ...)
  http_server& add_route(std::string_view method, std::string_view route,
                         Hty handler);
```

Registers a route that authenticates requests with the provider selected by `Aty`. That provider type must occur exactly once in `AUty...`.

**Template parameters:**

- **Aty:** `typename Aty`.
- **Hty:** `typename Hty`.

**Parameters:**

- **method:** HTTP method to match. Declaration: `std::string_view method`.
- **route:** Route pattern to match. Declaration: `std::string_view route`.
- **handler:** Callable invoked for a matching route. Declaration: `Hty handler`.

**Returns:** A reference to this object, for chaining calls.

**Errors:** Throws `std::runtime_error` if registration has closed because the server is running. Router or controller construction errors may propagate.

**Source:** [include/http_server.h:141](../../include/http_server.h#L141)

<a id="method-add-controller"></a>

## add_controller

🟠 **METHOD** · *http_server*

```cpp
template <typename CTty, typename... Args>
  http_server& add_controller(Args&&... args);
```

Constructs and registers controller `CTty` with the forwarded arguments.

**Template parameters:**

- **CTty:** `typename CTty`.
- **Args:** `typename... Args`.

**Parameters:**

- **args:** Arguments forwarded to the selected constructor or handler. Declaration: `Args&&... args`.

**Returns:** A reference to this object, for chaining calls.

**Errors:** Throws `std::runtime_error` if registration has closed because the server is running. Router or controller construction errors may propagate.

**Source:** [include/http_server.h:163](../../include/http_server.h#L163)
