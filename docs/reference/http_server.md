<a id="http-server"></a>
<a href="../../resources/docs/reference/http_server/title.svg#gh-light-mode-only">
  <img src="../../resources/docs/reference/http_server/title.svg" alt="http_server">
</a>
<a href="../../resources/docs/reference/http_server/title-dark.svg#gh-dark-mode-only">
  <img src="../../resources/docs/reference/http_server/title-dark.svg" alt="http_server">
</a>

**Header:** [include/http_server.h](../../include/http_server.h)

Combines an HTTP/1.1 router and protocol engine with a server transport. TCP is
the default transport.

### <a href="../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> doba

[http_server](#type-http-server)

### <a href="../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> http_server

[add_route&lt;Hty&gt;(method, route, handler)](#method-add-route) ·
[add_route&lt;Aty, Hty&gt;(method, route, handler)](#method-add-route-2) ·
[add_controller](#method-add-controller)

<a id="api"></a>
<a href="../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-http-server"></a>

## <a href="../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> http_server

**TYPE** · *doba*

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

<a id="method-add-route"></a>

## <a href="../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> add_route

**METHOD** · *http_server*

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

## <a href="../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> add_route

**METHOD** · *http_server*

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

## <a href="../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> add_controller

**METHOD** · *http_server*

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
