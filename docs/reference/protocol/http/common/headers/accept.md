<a id="accept"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/accept/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/accept/title.svg" alt="accept">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/accept/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/accept/title-dark.svg" alt="accept">
</a>

**Header:** [include/protocol/http/common/headers/accept.h](../../../../../../include/protocol/http/common/headers/accept.h)

Checks comma-separated media ranges (`type/subtype`, `type/*`, or `*/*`) and parameters. Each range permits at most one case-insensitive `q` parameter, with a value from 0 to 1 and at most three fractional digits. Other parameters require `name=value`; empty semicolon slots are rejected.

<a id="api"></a>
<a id="type-accept"></a>

## accept

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.

List parsing skips empty comma elements and respects quoted commas. Spaces/tabs are trimmed before commas and after separators, but not uniformly at the outer edges; pass the field value after HTTP field-level trimming.
