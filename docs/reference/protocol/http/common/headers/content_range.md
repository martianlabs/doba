<a id="content-range"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/content_range/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/content_range/title.svg" alt="content_range">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/content_range/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/content_range/title-dark.svg" alt="content_range">
</a>

**Header:** [include/protocol/http/common/headers/content_range.h](../../../../../../include/protocol/http/common/headers/content_range.h)

Checks `unit first-last/length`, `unit first-last/*`, or `unit */length`, with decimal numbers and a single space after the unit. It does not compare the numbers, check overflow, or limit the unit to `bytes`.

<a id="api"></a>
<a id="type-content-range"></a>

## content_range

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.
