<a id="last-modified"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/last_modified/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/last_modified/title.svg" alt="last_modified">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/last_modified/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/last_modified/title-dark.svg" alt="last_modified">
</a>

**Header:** [include/protocol/http/common/headers/last_modified.h](../../../../../../include/protocol/http/common/headers/last_modified.h)

Uses `date::check` for the three supported date layouts, without validating calendar dates or time ranges.

<a id="api"></a>
<a id="type-last-modified"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> last_modified

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.
