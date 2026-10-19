<a id="router-handler-static-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../resources/docs/reference/protocol/http/common/router_handler_static/index-dark.svg">
    <img src="../../../../../resources/docs/reference/protocol/http/common/router_handler_static/index.svg" alt="Index for router_handler_static.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/router_handler_static.h](../../../../../include/protocol/http/common/router_handler_static.h)

This header declares the types and operations listed below.

**protocol::http**

- [router_handler_static](#alias-router-handler-static) · *alias*

<a id="alias-router-handler-static"></a>

## router_handler_static

🟠 **ALIAS** · *protocol::http*

```cpp
template <typename RQty, typename RSty>
using router_handler_static = std::function<void(const RQty&, RSty&)>;
```

Alias for `std::function<void(const RQty&, RSty&)>`.

**Template parameters:**

- **RQty:** `typename RQty`.
- **RSty:** `typename RSty`.

**Source:** [include/protocol/http/common/router_handler_static.h:43](../../../../../include/protocol/http/common/router_handler_static.h#L43)
