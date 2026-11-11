<a id="accept-encoding"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_encoding/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_encoding/title.svg" alt="accept_encoding">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_encoding/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_encoding/title-dark.svg" alt="accept_encoding">
</a>

**Header:** [include/protocol/http/common/headers/accept_encoding.h](../../../../../../include/protocol/http/common/headers/accept_encoding.h)

Checks coding tokens with an optional `;q=` weight. It does not select a coding or check whether doba implements it.

<a id="api"></a>
<a id="type-accept-encoding"></a>

## accept_encoding

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.

List parsing skips empty comma elements and respects quoted commas. Spaces/tabs are trimmed before commas and after separators, but not uniformly at the outer edges; pass the field value after HTTP field-level trimming.
