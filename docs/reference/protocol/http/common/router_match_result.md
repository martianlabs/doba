<a id="router-match-result"></a>
<a href="../../../../../resources/docs/reference/protocol/http/common/router_match_result/title.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/common/router_match_result/title.svg" alt="router_match_result">
</a>
<a href="../../../../../resources/docs/reference/protocol/http/common/router_match_result/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/common/router_match_result/title-dark.svg" alt="router_match_result">
</a>

**Header:** [include/protocol/http/common/router_match_result.h](../../../../../include/protocol/http/common/router_match_result.h)

Provides the router match result protocol component.

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http

[router_match_result](#type-router-match-result)

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::router_match_result

[handler](#field-handler) ·
[parametrized_handler](#field-parametrized-handler) ·
[async_handler](#field-async-handler) ·
[async_parametrized_handler](#field-async-parametrized-handler) ·
[authenticate](#field-authenticate)

<a id="api"></a>
<a href="../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-router-match-result"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> router_match_result

**TYPE** · *protocol::http*

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

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> handler

**FIELD** · *protocol::http::router_match_result*

```cpp
const router_handler_static<RQty, RSty>* handler{nullptr};
```

Stores the handler value.

**Value:** `{nullptr}`

**Source:** [include/protocol/http/common/router_match_result.h:44](../../../../../include/protocol/http/common/router_match_result.h#L44)

<a id="field-parametrized-handler"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> parametrized_handler

**FIELD** · *protocol::http::router_match_result*

```cpp
const router_handler_parametrized<RQty, RSty>* parametrized_handler{nullptr};
```

Stores the parametrized handler value.

**Value:** `{nullptr}`

**Source:** [include/protocol/http/common/router_match_result.h:45](../../../../../include/protocol/http/common/router_match_result.h#L45)

<a id="field-async-handler"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> async_handler

**FIELD** · *protocol::http::router_match_result*

```cpp
const router_handler_static_async<RQty, RSty>* async_handler{nullptr};
```

Stores the async handler value.

**Value:** `{nullptr}`

**Source:** [include/protocol/http/common/router_match_result.h:46](../../../../../include/protocol/http/common/router_match_result.h#L46)

<a id="field-async-parametrized-handler"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> async_parametrized_handler

**FIELD** · *protocol::http::router_match_result*

```cpp
const router_handler_parametrized_async<RQty, RSty>*
      async_parametrized_handler{nullptr};
```

Stores the async parametrized handler value.

**Value:** `{nullptr}`

**Source:** [include/protocol/http/common/router_match_result.h:47](../../../../../include/protocol/http/common/router_match_result.h#L47)

<a id="field-authenticate"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> authenticate

**FIELD** · *protocol::http::router_match_result*

```cpp
const std::function<auth::authentication_result(const RQty&)>*
      authenticate{nullptr};
```

Stores the authenticate value.

**Value:** `{nullptr}`

**Source:** [include/protocol/http/common/router_match_result.h:49](../../../../../include/protocol/http/common/router_match_result.h#L49)
