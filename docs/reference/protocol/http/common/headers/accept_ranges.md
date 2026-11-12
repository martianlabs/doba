<a id="accept-ranges"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_ranges/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_ranges/title.svg" alt="accept_ranges">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_ranges/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_ranges/title-dark.svg" alt="accept_ranges">
</a>

**Header:** [include/protocol/http/common/headers/accept_ranges.h](../../../../../../include/protocol/http/common/headers/accept_ranges.h)

Requires at least one range-unit token. Tokens such as `none` are accepted without checking which units the server supports.

<a id="api"></a>
<a id="type-accept-ranges"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> accept_ranges

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.

List parsing skips empty comma elements and respects quoted commas. Spaces/tabs are trimmed before commas and after separators, but not uniformly at the outer edges; pass the field value after HTTP field-level trimming.
