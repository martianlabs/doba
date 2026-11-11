<a id="authentication-info"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/authentication_info/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/authentication_info/title.svg" alt="authentication_info">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/authentication_info/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/authentication_info/title-dark.svg" alt="authentication_info">
</a>

**Header:** [include/protocol/http/common/headers/authentication_info.h](../../../../../../include/protocol/http/common/headers/authentication_info.h)

Checks comma-separated `name=value` authentication parameters. Values may be tokens or quoted strings; spaces and tabs around `=` are accepted.

<a id="api"></a>
<a id="type-authentication-info"></a>

## authentication_info

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.

List parsing skips empty comma elements and respects quoted commas. Spaces/tabs are trimmed before commas and after separators, but not uniformly at the outer edges; pass the field value after HTTP field-level trimming.
