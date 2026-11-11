<a id="content-type"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/content_type/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/content_type/title.svg" alt="content_type">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/content_type/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/content_type/title-dark.svg" alt="content_type">
</a>

**Header:** [include/protocol/http/common/headers/content_type.h](../../../../../../include/protocol/http/common/headers/content_type.h)

Checks token `type/subtype` with optional semicolon-separated `name=value` parameters. Empty semicolon slots are accepted; spaces around `=` are rejected. It does not look up registered media types or decode a body.

<a id="api"></a>
<a id="type-content-type"></a>

## content_type

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.
