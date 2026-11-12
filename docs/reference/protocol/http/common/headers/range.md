<a id="range"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/range/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/range/title.svg" alt="range">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/range/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/range/title-dark.svg" alt="range">
</a>

**Header:** [include/protocol/http/common/headers/range.h](../../../../../../include/protocol/http/common/headers/range.h)

Requires `unit=` followed by at least one nonempty comma-separated range. For case-insensitive `bytes`, accepts `first-last`, `first-`, or `-suffix`; other units also accept visible non-comma text. It does not convert numbers, check overflow/order, or compare ranges with a file size.

<a id="api"></a>
<a id="type-range"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> range

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.
