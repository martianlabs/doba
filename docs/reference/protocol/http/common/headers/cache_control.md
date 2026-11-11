<a id="cache-control"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/cache_control/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/cache_control/title.svg" alt="cache_control">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/cache_control/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/cache_control/title-dark.svg" alt="cache_control">
</a>

**Header:** [include/protocol/http/common/headers/cache_control.h](../../../../../../include/protocol/http/common/headers/cache_control.h)

Checks comma-separated directive tokens with optional `=token` or `="quoted value"`. It does not interpret directive names, detect conflicting directives, or configure caching.

<a id="api"></a>
<a id="type-cache-control"></a>

## cache_control

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.

List parsing skips empty comma elements and respects quoted commas. Spaces/tabs are trimmed before commas and after separators, but not uniformly at the outer edges; pass the field value after HTTP field-level trimming.
