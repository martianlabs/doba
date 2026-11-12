<a id="authorization"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/authorization/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/authorization/title.svg" alt="authorization">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/authorization/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/authorization/title-dark.svg" alt="authorization">
</a>

**Header:** [include/protocol/http/common/headers/authorization.h](../../../../../../include/protocol/http/common/headers/authorization.h)

Checks a scheme token, optionally followed by spaces and token68 credentials or an authentication-parameter list. A scheme alone is accepted. It does not authenticate credentials or decode Base64.

<a id="api"></a>
<a id="type-authorization"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> authorization

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.
