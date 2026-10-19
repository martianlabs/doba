<a id="router-handler-static-async-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../resources/docs/reference/protocol/http/common/router_handler_static_async/index-dark.svg">
    <img src="../../../../../resources/docs/reference/protocol/http/common/router_handler_static_async/index.svg" alt="Index for router_handler_static_async.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/router_handler_static_async.h](../../../../../include/protocol/http/common/router_handler_static_async.h)

This header declares the types and operations listed below.

**protocol::http**

- [router_handler_static_async](#alias-router-handler-static-async) · *alias*

<a id="alias-router-handler-static-async"></a>

## router_handler_static_async

🟠 **ALIAS** · *protocol::http*

```cpp
template <typename RQty, typename RSty>
using router_handler_static_async =
    std::function<common::task<void>(const RQty&, RSty&)>;
```

Alias for `std::function<common::task<void>(const RQty&, RSty&)>`.

**Template parameters:**

- **RQty:** `typename RQty`.
- **RSty:** `typename RSty`.

**Source:** [include/protocol/http/common/router_handler_static_async.h:43](../../../../../include/protocol/http/common/router_handler_static_async.h#L43)
