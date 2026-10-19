<a id="router-handler-parametrized"></a>
<a href="../../../../../resources/docs/reference/protocol/http/common/router_handler_parametrized/index.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/common/router_handler_parametrized/index.svg" alt="router_handler_parametrized">
</a>
<a href="../../../../../resources/docs/reference/protocol/http/common/router_handler_parametrized/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/common/router_handler_parametrized/index-dark.svg" alt="router_handler_parametrized">
</a>

**Header:** [include/protocol/http/common/router_handler_parametrized.h](../../../../../include/protocol/http/common/router_handler_parametrized.h)

Provides the router handler parametrized protocol component.

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::detail

[dependent_false_v](#constant-dependent-false-v) ·
[extract_route_parameters](#function-extract-route-parameters) ·
[parse_route_parameter](#function-parse-route-parameter) ·
[matches_route_parameter](#function-matches-route-parameter) ·
[match_route_parameters_](#function-match-route-parameters) ·
[match_route_parameters](#function-match-route-parameters-2) ·
[parse_route_parameters_](#function-parse-route-parameters) ·
[invoke_route_handler](#function-invoke-route-handler)

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http

[router_handler_parametrized](#type-router-handler-parametrized) ·
[make_router_handler_parametrized](#function-make-router-handler-parametrized)

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::router_handler_parametrized

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

<a id="constant-dependent-false-v"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> dependent_false_v

**CONSTANT** · *protocol::http::detail*

```cpp
template <typename>
inline constexpr bool dependent_false_v = false;
```

Defines the dependent false v constant.

**Value:** `false`

**Source:** [include/protocol/http/common/router_handler_parametrized.h:44](../../../../../include/protocol/http/common/router_handler_parametrized.h#L44)

<a id="function-extract-route-parameters"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> extract_route_parameters

**FUNCTION** · *protocol::http::detail*

```cpp
template <std::size_t N>
bool extract_route_parameters(std::string_view pattern, std::string_view path,
                              std::array<std::string_view, N>& parameters);
```

Extracts named values from a matching route.

**Template parameters:**

- **N:** `std::size_t N`.

**Parameters:**

- **pattern:** Pattern used for matching. Declaration: `std::string_view pattern`.
- **path:** Filesystem path or route path, according to the call. Declaration: `std::string_view path`.
- **parameters:** Parameters to parse or pass to the handler. Declaration: `std::array<std::string_view, N>& parameters`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/router_handler_parametrized.h:60](../../../../../include/protocol/http/common/router_handler_parametrized.h#L60)

<a id="function-parse-route-parameter"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> parse_route_parameter

**FUNCTION** · *protocol::http::detail*

```cpp
template <typename T>
bool parse_route_parameter(std::string_view input, T& output);
```

Parses route parameter.

**Template parameters:**

- **T:** `typename T`.

**Parameters:**

- **input:** Input data to process. Declaration: `std::string_view input`.
- **output:** Destination or output callback, as specified by its type. Declaration: `T& output`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/router_handler_parametrized.h:105](../../../../../include/protocol/http/common/router_handler_parametrized.h#L105)

<a id="function-matches-route-parameter"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> matches_route_parameter

**FUNCTION** · *protocol::http::detail*

```cpp
template <typename T>
bool matches_route_parameter(std::string_view input);
```

Checks whether one route parameter matches.

**Template parameters:**

- **T:** `typename T`.

**Parameters:**

- **input:** Input data to process. Declaration: `std::string_view input`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/router_handler_parametrized.h:146](../../../../../include/protocol/http/common/router_handler_parametrized.h#L146)

<a id="function-match-route-parameters"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> match_route_parameters_

**FUNCTION** · *protocol::http::detail*

```cpp
template <typename... Args, std::size_t... I>
bool match_route_parameters_(
    const std::array<std::string_view, sizeof...(Args)>& parameters,
    std::index_sequence<I...>);
```

Matches and converts route parameter values.

**Template parameters:**

- **Args:** `typename... Args`.
- **I:** `std::size_t... I`.

**Parameters:**

- **parameters:** Parameters to parse or pass to the handler. Declaration: `const std::array<std::string_view, sizeof...(Args)>& parameters`.
- **I:** Zero-based index of the requested element. Declaration: `std::index_sequence<I...>`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/router_handler_parametrized.h:169](../../../../../include/protocol/http/common/router_handler_parametrized.h#L169)

<a id="function-match-route-parameters-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> match_route_parameters

**FUNCTION** · *protocol::http::detail*

```cpp
template <typename... Args>
bool match_route_parameters(std::string_view pattern, std::string_view path);
```

Matches and converts route parameter values.

**Template parameters:**

- **Args:** `typename... Args`.

**Parameters:**

- **pattern:** Pattern used for matching. Declaration: `std::string_view pattern`.
- **path:** Filesystem path or route path, according to the call. Declaration: `std::string_view path`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/router_handler_parametrized.h:188](../../../../../include/protocol/http/common/router_handler_parametrized.h#L188)

<a id="function-parse-route-parameters"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> parse_route_parameters_

**FUNCTION** · *protocol::http::detail*

```cpp
template <typename... Args, std::size_t... I>
bool parse_route_parameters_(
    const std::array<std::string_view, sizeof...(Args)>& parameters,
    std::tuple<std::decay_t<Args>...>& values, std::index_sequence<I...>);
```

Parses route parameters .

**Template parameters:**

- **Args:** `typename... Args`.
- **I:** `std::size_t... I`.

**Parameters:**

- **parameters:** Parameters to parse or pass to the handler. Declaration: `const std::array<std::string_view, sizeof...(Args)>& parameters`.
- **values:** Tuple of parameter values. Declaration: `std::tuple<std::decay_t<Args>...>& values`.
- **I:** Zero-based index of the requested element. Declaration: `std::index_sequence<I...>`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/router_handler_parametrized.h:209](../../../../../include/protocol/http/common/router_handler_parametrized.h#L209)

<a id="function-invoke-route-handler"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> invoke_route_handler

**FUNCTION** · *protocol::http::detail*

```cpp
template <typename Hty, typename RQty, typename RSty, typename... Args>
void invoke_route_handler(Hty& handler, const RQty& req, RSty& res,
                          std::string_view pattern, std::string_view path);
```

Invokes the route handler with converted parameters.

**Template parameters:**

- **Hty:** `typename Hty`.
- **RQty:** `typename RQty`.
- **RSty:** `typename RSty`.
- **Args:** `typename... Args`.

**Parameters:**

- **handler:** Callable invoked for a matching route. Declaration: `Hty& handler`.
- **req:** HTTP request passed to the handler. Declaration: `const RQty& req`.
- **res:** HTTP response passed to the handler. Declaration: `RSty& res`.
- **pattern:** Pattern used for matching. Declaration: `std::string_view pattern`.
- **path:** Filesystem path or route path, according to the call. Declaration: `std::string_view path`.

**Returns:** No value (`void`).

**Errors:** Directly throws `std::runtime_error`. Calls made by this operation may propagate additional errors.

**Source:** [include/protocol/http/common/router_handler_parametrized.h:230](../../../../../include/protocol/http/common/router_handler_parametrized.h#L230)

<a id="type-router-handler-parametrized"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> router_handler_parametrized

**TYPE** · *protocol::http*

**Declaration synopsis:**

```cpp
template <typename RQty, typename RSty>
class router_handler_parametrized { ... };
```

Provides the router handler parametrized protocol component.

**Template parameters:**

- **RQty:** `typename RQty`.
- **RSty:** `typename RSty`.

**Source:** [include/protocol/http/common/router_handler_parametrized.h:258](../../../../../include/protocol/http/common/router_handler_parametrized.h#L258)

<a id="alias-matcher-type"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> matcher_type

**ALIAS** · *protocol::http::router_handler_parametrized*

```cpp
using matcher_type = bool (*)(std::string_view, std::string_view);
```

Alias for `bool (*)(std::string_view, std::string_view)`.

**Source:** [include/protocol/http/common/router_handler_parametrized.h:263](../../../../../include/protocol/http/common/router_handler_parametrized.h#L263)

<a id="alias-callback-type"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> callback_type

**ALIAS** · *protocol::http::router_handler_parametrized*

```cpp
using callback_type = std::function<void(const RQty&, RSty&, std::string_view,
                                           std::string_view)>;
```

Alias for `std::function<void(const RQty&, RSty&, std::string_view, std::string_view)>`.

**Source:** [include/protocol/http/common/router_handler_parametrized.h:264](../../../../../include/protocol/http/common/router_handler_parametrized.h#L264)

<a id="method-matches"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> matches

**METHOD** · *protocol::http::router_handler_parametrized*

```cpp
[[nodiscard]] bool matches(std::string_view path) const;
```

Checks whether the value matches this pattern.

**Parameters:**

- **path:** Filesystem path or route path, according to the call. Declaration: `std::string_view path`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/router_handler_parametrized.h:277](../../../../../include/protocol/http/common/router_handler_parametrized.h#L277)

<a id="method-invoke"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> invoke

**METHOD** · *protocol::http::router_handler_parametrized*

```cpp
void invoke(const RQty& req, RSty& res, std::string_view path) const;
```

Invokes the stored callable.

**Parameters:**

- **req:** HTTP request passed to the handler. Declaration: `const RQty& req`.
- **res:** HTTP response passed to the handler. Declaration: `RSty& res`.
- **path:** Filesystem path or route path, according to the call. Declaration: `std::string_view path`.

**Returns:** No value (`void`).

**Source:** [include/protocol/http/common/router_handler_parametrized.h:283](../../../../../include/protocol/http/common/router_handler_parametrized.h#L283)

<a id="function-make-router-handler-parametrized"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> make_router_handler_parametrized

**FUNCTION** · *protocol::http*

```cpp
template <typename RQty, typename RSty, typename... Args, typename Hty>
auto make_router_handler_parametrized(std::string_view pattern, Hty&& handler);
```

Creates router handler parametrized.

**Template parameters:**

- **RQty:** `typename RQty`.
- **RSty:** `typename RSty`.
- **Args:** `typename... Args`.
- **Hty:** `typename Hty`.

**Parameters:**

- **pattern:** Pattern used for matching. Declaration: `std::string_view pattern`.
- **handler:** Callable invoked for a matching route. Declaration: `Hty&& handler`.

**Returns:** A `router_handler_parametrized<RQty, RSty>` wrapping the handler and route pattern.

**Source:** [include/protocol/http/common/router_handler_parametrized.h:313](../../../../../include/protocol/http/common/router_handler_parametrized.h#L313)
