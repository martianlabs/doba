<a id="router"></a>
<a href="../../../../../resources/docs/reference/protocol/http/common/router/index.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/common/router/index.svg" alt="router">
</a>
<a href="../../../../../resources/docs/reference/protocol/http/common/router/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/common/router/index-dark.svg" alt="router">
</a>

**Header:** [include/protocol/http/common/router.h](../../../../../include/protocol/http/common/router.h)

Provides the http 1.1 router implementation.

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http

[router](#type-router)

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::router

[add&lt;Hty&gt;(method, route, handler)](#method-add) ·
[add&lt;Hty&gt;(method, route, handler, authenticate)](#method-add-2) ·
[add_controller](#method-add-controller) ·
[match](#method-match) ·
[allowed_methods](#method-allowed-methods)

<a id="api-details"></a>
<a href="../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-router"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> router

**TYPE** · *protocol::http*

**Declaration synopsis:**

```cpp
template <typename RQty, typename RSty>
class router { ... };
```

Provides the http 1.1 router implementation.

**Template parameters:**

- **RQty:** `typename RQty`.
- **RSty:** `typename RSty`.

**Source:** [include/protocol/http/common/router.h:58](../../../../../include/protocol/http/common/router.h#L58)

<a id="method-add"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> add

**METHOD** · *protocol::http::router*

```cpp
template <typename Hty>
    requires router_handler_lambda<Hty>
  void add(std::string_view method, std::string_view route, Hty handler);
```

Adds a route to the router with the specified HTTP method, route path, and handler.

**Template parameters:**

- **Hty:** `typename Hty`.

**Parameters:**

- **method:** HTTP method to match. Declaration: `std::string_view method`.
- **route:** Route pattern to match. Declaration: `std::string_view route`.
- **handler:** Callable invoked for a matching route. Declaration: `Hty handler`.

**Returns:** No value (`void`).

**Source:** [include/protocol/http/common/router.h:80](../../../../../include/protocol/http/common/router.h#L80)

<a id="method-add-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> add

**METHOD** · *protocol::http::router*

```cpp
template <typename Hty>
    requires router_handler_lambda<Hty>
  void add(std::string_view method, std::string_view route, Hty handler,
           std::function<auth::authentication_result(const RQty&)>
               authenticate);
```

Adds the requested value.

**Template parameters:**

- **Hty:** `typename Hty`.

**Parameters:**

- **method:** HTTP method to match. Declaration: `std::string_view method`.
- **route:** Route pattern to match. Declaration: `std::string_view route`.
- **handler:** Callable invoked for a matching route. Declaration: `Hty handler`.
- **authenticate:** Authentication callback for the route. Declaration: `std::function<auth::authentication_result(const RQty&)> authenticate`.

**Returns:** No value (`void`).

**Errors:** Directly throws `std::invalid_argument`. Calls made by this operation may propagate additional errors.

**Source:** [include/protocol/http/common/router.h:89](../../../../../include/protocol/http/common/router.h#L89)

<a id="method-add-controller"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> add_controller

**METHOD** · *protocol::http::router*

```cpp
template <typename CTty, typename... Args>
  void add_controller(Args&&... args);
```

Adds a controller to the router. The controller must implement the 'register_routes' method to register its routes with the router.

**Template parameters:**

- **CTty:** `typename CTty`.
- **Args:** `typename... Args`.

**Parameters:**

- **args:** Arguments forwarded to the selected constructor or handler. Declaration: `Args&&... args`.

**Returns:** No value (`void`).

**Errors:** Directly throws `std::invalid_argument`. Calls made by this operation may propagate additional errors.

**Source:** [include/protocol/http/common/router.h:180](../../../../../include/protocol/http/common/router.h#L180)

<a id="method-match"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> match

**METHOD** · *protocol::http::router*

```cpp
[[nodiscard]]
  router_match_result<RQty, RSty> match(std::string_view method,
                                        std::string_view path) const;
```

Matches the given HTTP method and path against the registered routes.

**Parameters:**

- **method:** HTTP method to match. Declaration: `std::string_view method`.
- **path:** Filesystem path or route path, according to the call. Declaration: `std::string_view path`.

**Returns:** A value of type `router_match_result<RQty, RSty>`.

**Source:** [include/protocol/http/common/router.h:213](../../../../../include/protocol/http/common/router.h#L213)

<a id="method-allowed-methods"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> allowed_methods

**METHOD** · *protocol::http::router*

```cpp
[[nodiscard]]
  std::string allowed_methods(std::string_view path) const;
```

Returns a comma-separated string of allowed HTTP methods for the given path.

**Parameters:**

- **path:** Filesystem path or route path, according to the call. Declaration: `std::string_view path`.

**Returns:** A value of type `std::string`.

**Source:** [include/protocol/http/common/router.h:269](../../../../../include/protocol/http/common/router.h#L269)
