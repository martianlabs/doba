<a id="proxy-connection"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/proxy_connection/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/proxy_connection/title.svg" alt="proxy_connection">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/proxy_connection/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/proxy_connection/title-dark.svg" alt="proxy_connection">
</a>

**Header:** [include/protocol/http/common/headers/proxy_connection.h](../../../../../../include/protocol/http/common/headers/proxy_connection.h)

Checks a comma-separated list of connection-option tokens. The class is named `x_proxy_connection`; this checker does not change connection lifetime.

<a id="api"></a>
<a id="type-x-proxy-connection"></a>

## x_proxy_connection

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.

List parsing skips empty comma elements and respects quoted commas. Spaces/tabs are trimmed before commas and after separators, but not uniformly at the outer edges; pass the field value after HTTP field-level trimming.
