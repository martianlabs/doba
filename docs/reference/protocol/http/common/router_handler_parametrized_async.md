<a id="router-handler-parametrized-async"></a>
<a href="../../../../../resources/docs/reference/protocol/http/common/router_handler_parametrized_async/index.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/common/router_handler_parametrized_async/index.svg" alt="router_handler_parametrized_async">
</a>
<a href="../../../../../resources/docs/reference/protocol/http/common/router_handler_parametrized_async/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/common/router_handler_parametrized_async/index-dark.svg" alt="router_handler_parametrized_async">
</a>

**Header:** [include/protocol/http/common/router_handler_parametrized_async.h](../../../../../include/protocol/http/common/router_handler_parametrized_async.h)

Provides the router handler parametrized async protocol component.

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http

[router_handler_parametrized_async](#type-router-handler-parametrized-async) ·
[make_router_handler_parametrized_async](#function-make-router-handler-parametrized-async)

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::router_handler_parametrized_async

[matcher_type](#alias-matcher-type) ·
[callback_type](#alias-callback-type) ·
[matches](#method-matches) ·
[invoke](#method-invoke)

<a id="api-details"></a>
<a href="../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-router-handler-parametrized-async"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> router_handler_parametrized_async

**TYPE** · *protocol::http*

**Declaration synopsis:**

```cpp
template <typename RQty, typename RSty>
class router_handler_parametrized_async { ... };
```

Provides the router handler parametrized async protocol component.

**Template parameters:**

- **RQty:** `typename RQty`.
- **RSty:** `typename RSty`.

**Source:** [include/protocol/http/common/router_handler_parametrized_async.h:52](../../../../../include/protocol/http/common/router_handler_parametrized_async.h#L52)

<a id="alias-matcher-type"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> matcher_type

**ALIAS** · *protocol::http::router_handler_parametrized_async*

```cpp
using matcher_type = bool (*)(std::string_view, std::string_view);
```

Alias for `bool (*)(std::string_view, std::string_view)`.

**Source:** [include/protocol/http/common/router_handler_parametrized_async.h:57](../../../../../include/protocol/http/common/router_handler_parametrized_async.h#L57)

<a id="alias-callback-type"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> callback_type

**ALIAS** · *protocol::http::router_handler_parametrized_async*

```cpp
using callback_type = std::function<common::task<void>(
      const RQty&, RSty&, std::string_view, std::string_view,
      std::shared_ptr<void>&)>;
```

Alias for `std::function<common::task<void>( const RQty&, RSty&, std::string_view, std::string_view, std::shared_ptr<void>&)>`.

**Source:** [include/protocol/http/common/router_handler_parametrized_async.h:58](../../../../../include/protocol/http/common/router_handler_parametrized_async.h#L58)

<a id="method-matches"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> matches

**METHOD** · *protocol::http::router_handler_parametrized_async*

```cpp
[[nodiscard]] bool matches(std::string_view path) const;
```

Checks whether the value matches this pattern.

**Parameters:**

- **path:** Filesystem path or route path, according to the call. Declaration: `std::string_view path`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/router_handler_parametrized_async.h:72](../../../../../include/protocol/http/common/router_handler_parametrized_async.h#L72)

<a id="method-invoke"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> invoke

**METHOD** · *protocol::http::router_handler_parametrized_async*

```cpp
common::task<void> invoke(const RQty& req, RSty& res, std::string_view path,
                            std::shared_ptr<void>& parameters) const;
```

Invokes the stored callable.

**Parameters:**

- **req:** HTTP request passed to the handler. Declaration: `const RQty& req`.
- **res:** HTTP response passed to the handler. Declaration: `RSty& res`.
- **path:** Filesystem path or route path, according to the call. Declaration: `std::string_view path`.
- **parameters:** Parameters to parse or pass to the handler. Declaration: `std::shared_ptr<void>& parameters`.

**Returns:** A value of type `common::task<void>`.

**Source:** [include/protocol/http/common/router_handler_parametrized_async.h:78](../../../../../include/protocol/http/common/router_handler_parametrized_async.h#L78)

<a id="function-make-router-handler-parametrized-async"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> make_router_handler_parametrized_async

**FUNCTION** · *protocol::http*

```cpp
template <typename RQty, typename RSty, typename... Args, typename Hty>
auto make_router_handler_parametrized_async(std::string_view pattern,
                                            Hty&& handler);
```

Creates router handler parametrized async.

**Template parameters:**

- **RQty:** `typename RQty`.
- **RSty:** `typename RSty`.
- **Args:** `typename... Args`.
- **Hty:** `typename Hty`.

**Parameters:**

- **pattern:** Pattern used for matching. Declaration: `std::string_view pattern`.
- **handler:** Callable invoked for a matching route. Declaration: `Hty&& handler`.

**Returns:** A `router_handler_parametrized_async<RQty, RSty>` wrapping the handler and route pattern.

**Errors:** Directly throws `std::runtime_error`. Calls made by this operation may propagate additional errors.

**Source:** [include/protocol/http/common/router_handler_parametrized_async.h:104](../../../../../include/protocol/http/common/router_handler_parametrized_async.h#L104)
