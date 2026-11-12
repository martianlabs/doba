<a id="content-location"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/content_location/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/content_location/title.svg" alt="content_location">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/content_location/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/content_location/title-dark.svg" alt="content_location">
</a>

**Header:** [include/protocol/http/common/headers/content_location.h](../../../../../../include/protocol/http/common/headers/content_location.h)

Checks a URI reference without a fragment. Empty and relative references are accepted; this does not resolve a URL or open a file.

<a id="api"></a>
<a id="type-content-location"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> content_location

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.
