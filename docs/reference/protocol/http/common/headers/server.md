<a id="server"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/server/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/server/title.svg" alt="server">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/server/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/server/title-dark.svg" alt="server">
</a>

**Header:** [include/protocol/http/common/headers/server.h](../../../../../../include/protocol/http/common/headers/server.h)

Requires a product token with an optional `/version`, followed by whitespace-separated products or balanced comments. Trailing whitespace is rejected. It does not identify or configure the running server.

<a id="api"></a>
<a id="type-server"></a>

## server

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.
