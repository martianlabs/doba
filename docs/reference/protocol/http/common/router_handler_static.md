<a id="router-handler-static"></a>
<a href="../../../../../resources/docs/reference/protocol/http/common/router_handler_static/title.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/common/router_handler_static/title.svg" alt="router_handler_static">
</a>
<a href="../../../../../resources/docs/reference/protocol/http/common/router_handler_static/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/common/router_handler_static/title-dark.svg" alt="router_handler_static">
</a>

**Header:** [include/protocol/http/common/router_handler_static.h](../../../../../include/protocol/http/common/router_handler_static.h)

This header declares the types and operations listed below.

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http

[router_handler_static](#alias-router-handler-static)

<a id="api"></a>
<a href="../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="alias-router-handler-static"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> router_handler_static

**ALIAS** · *protocol::http*

```cpp
template <typename RQty, typename RSty>
using router_handler_static = std::function<void(const RQty&, RSty&)>;
```

Alias for `std::function<void(const RQty&, RSty&)>`.

**Template parameters:**

- **RQty:** `typename RQty`.
- **RSty:** `typename RSty`.

**Source:** [include/protocol/http/common/router_handler_static.h:43](../../../../../include/protocol/http/common/router_handler_static.h#L43)
