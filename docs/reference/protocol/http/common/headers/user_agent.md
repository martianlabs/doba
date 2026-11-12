<a id="user-agent"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/user_agent/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/user_agent/title.svg" alt="user_agent">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/user_agent/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/user_agent/title-dark.svg" alt="user_agent">
</a>

**Header:** [include/protocol/http/common/headers/user_agent.h](../../../../../../include/protocol/http/common/headers/user_agent.h)

Requires a product token with an optional `/version`, followed by whitespace-separated products or balanced comments. Trailing whitespace is rejected. It does not identify capabilities or establish trust.

<a id="api"></a>
<a id="type-user-agent"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> user_agent

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.
