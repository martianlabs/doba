<a id="router-match-result-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../resources/docs/reference/protocol/http/common/router_match_result/index-dark.svg">
    <img src="../../../../../resources/docs/reference/protocol/http/common/router_match_result/index.svg" alt="Index for router_match_result.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/router_match_result.h](../../../../../include/protocol/http/common/router_match_result.h)

Provides the router match result protocol component.

**protocol::http**

- [router_match_result](#type-router-match-result) · *type*

**protocol::http::router_match_result**

- [handler](#field-handler) · *field*
- [parametrized_handler](#field-parametrized-handler) · *field*
- [async_handler](#field-async-handler) · *field*
- [async_parametrized_handler](#field-async-parametrized-handler) · *field*
- [authenticate](#field-authenticate) · *field*
- [operator bool](#operator-operator-bool) · *operator*

<a id="type-router-match-result"></a>

## router_match_result

🟠 **TYPE** · *protocol::http*

**Declaration synopsis:**

```cpp
template <typename RQty, typename RSty>
struct router_match_result { ... };
```

Provides the router match result protocol component.

**Template parameters:**

- **RQty:** `typename RQty`.
- **RSty:** `typename RSty`.

**Source:** [include/protocol/http/common/router_match_result.h:43](../../../../../include/protocol/http/common/router_match_result.h#L43)

<a id="field-handler"></a>

## handler

🟠 **FIELD** · *protocol::http::router_match_result*

```cpp
const router_handler_static<RQty, RSty>* handler{nullptr};
```

Stores the handler value.

**Value:** `{nullptr}`

**Source:** [include/protocol/http/common/router_match_result.h:44](../../../../../include/protocol/http/common/router_match_result.h#L44)

<a id="field-parametrized-handler"></a>

## parametrized_handler

🟠 **FIELD** · *protocol::http::router_match_result*

```cpp
const router_handler_parametrized<RQty, RSty>* parametrized_handler{nullptr};
```

Stores the parametrized handler value.

**Value:** `{nullptr}`

**Source:** [include/protocol/http/common/router_match_result.h:45](../../../../../include/protocol/http/common/router_match_result.h#L45)

<a id="field-async-handler"></a>

## async_handler

🟠 **FIELD** · *protocol::http::router_match_result*

```cpp
const router_handler_static_async<RQty, RSty>* async_handler{nullptr};
```

Stores the async handler value.

**Value:** `{nullptr}`

**Source:** [include/protocol/http/common/router_match_result.h:46](../../../../../include/protocol/http/common/router_match_result.h#L46)

<a id="field-async-parametrized-handler"></a>

## async_parametrized_handler

🟠 **FIELD** · *protocol::http::router_match_result*

```cpp
const router_handler_parametrized_async<RQty, RSty>*
      async_parametrized_handler{nullptr};
```

Stores the async parametrized handler value.

**Value:** `{nullptr}`

**Source:** [include/protocol/http/common/router_match_result.h:47](../../../../../include/protocol/http/common/router_match_result.h#L47)

<a id="field-authenticate"></a>

## authenticate

🟠 **FIELD** · *protocol::http::router_match_result*

```cpp
const std::function<auth::authentication_result(const RQty&)>*
      authenticate{nullptr};
```

Stores the authenticate value.

**Value:** `{nullptr}`

**Source:** [include/protocol/http/common/router_match_result.h:49](../../../../../include/protocol/http/common/router_match_result.h#L49)

<a id="operator-operator-bool"></a>

## operator bool

🟠 **OPERATOR** · *protocol::http::router_match_result*

```cpp
[[nodiscard]] explicit operator bool() const;
```

Implements operator bool for router_match_result.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/router_match_result.h:51](../../../../../include/protocol/http/common/router_match_result.h#L51)
