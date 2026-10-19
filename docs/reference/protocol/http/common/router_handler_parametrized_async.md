<a id="router-handler-parametrized-async-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../resources/docs/reference/protocol/http/common/router_handler_parametrized_async/index-dark.svg">
    <img src="../../../../../resources/docs/reference/protocol/http/common/router_handler_parametrized_async/index.svg" alt="Index for router_handler_parametrized_async.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/router_handler_parametrized_async.h](../../../../../include/protocol/http/common/router_handler_parametrized_async.h)

Provides the router handler parametrized async protocol component.

**protocol::http**

- [router_handler_parametrized_async](#type-router-handler-parametrized-async) · *type*

**protocol::http::router_handler_parametrized_async**

- [matcher_type](#alias-matcher-type) · *alias*
- [callback_type](#alias-callback-type) · *alias*
- [router_handler_parametrized_async](#constructor-router-handler-parametrized-async) · *constructor*
- [matches](#method-matches) · *method*
- [invoke](#method-invoke) · *method*

**protocol::http**

- [make_router_handler_parametrized_async](#function-make-router-handler-parametrized-async) · *function*

<a id="type-router-handler-parametrized-async"></a>

## router_handler_parametrized_async

🟠 **TYPE** · *protocol::http*

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

## matcher_type

🟠 **ALIAS** · *protocol::http::router_handler_parametrized_async*

```cpp
using matcher_type = bool (*)(std::string_view, std::string_view);
```

Alias for `bool (*)(std::string_view, std::string_view)`.

**Source:** [include/protocol/http/common/router_handler_parametrized_async.h:57](../../../../../include/protocol/http/common/router_handler_parametrized_async.h#L57)

<a id="alias-callback-type"></a>

## callback_type

🟠 **ALIAS** · *protocol::http::router_handler_parametrized_async*

```cpp
using callback_type = std::function<common::task<void>(
      const RQty&, RSty&, std::string_view, std::string_view,
      std::shared_ptr<void>&)>;
```

Alias for `std::function<common::task<void>( const RQty&, RSty&, std::string_view, std::string_view, std::shared_ptr<void>&)>`.

**Source:** [include/protocol/http/common/router_handler_parametrized_async.h:58](../../../../../include/protocol/http/common/router_handler_parametrized_async.h#L58)

<a id="constructor-router-handler-parametrized-async"></a>

## router_handler_parametrized_async

🟠 **CONSTRUCTOR** · *protocol::http::router_handler_parametrized_async*

```cpp
router_handler_parametrized_async(std::string pattern, matcher_type matcher,
                                    callback_type callback);
```

Constructs a router_handler_parametrized_async value.

**Parameters:**

- **pattern:** Pattern used for matching. Declaration: `std::string pattern`.
- **matcher:** Initial value supplied to the constructor. Declaration: `matcher_type matcher`.
- **callback:** Callable invoked when the corresponding event occurs. Declaration: `callback_type callback`.

**Source:** [include/protocol/http/common/router_handler_parametrized_async.h:64](../../../../../include/protocol/http/common/router_handler_parametrized_async.h#L64)

<a id="method-matches"></a>

## matches

🟠 **METHOD** · *protocol::http::router_handler_parametrized_async*

```cpp
[[nodiscard]] bool matches(std::string_view path) const;
```

Checks whether the value matches this pattern.

**Parameters:**

- **path:** Filesystem path or route path, according to the call. Declaration: `std::string_view path`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/router_handler_parametrized_async.h:72](../../../../../include/protocol/http/common/router_handler_parametrized_async.h#L72)

<a id="method-invoke"></a>

## invoke

🟠 **METHOD** · *protocol::http::router_handler_parametrized_async*

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

## make_router_handler_parametrized_async

🟠 **FUNCTION** · *protocol::http*

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
