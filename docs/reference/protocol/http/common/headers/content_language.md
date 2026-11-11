<a id="content-language"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/content_language/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/content_language/title.svg" alt="content_language">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/content_language/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/content_language/title-dark.svg" alt="content_language">
</a>

**Header:** [include/protocol/http/common/headers/content_language.h](../../../../../../include/protocol/http/common/headers/content_language.h)

Checks language-tag structure, including private-use tags, extensions, and the built-in irregular grandfathered tags. It does not consult a language registry or reject repeated variant/extension names.

<a id="api"></a>
<a id="type-content-language"></a>

## content_language

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.

List parsing skips empty comma elements and respects quoted commas. Spaces/tabs are trimmed before commas and after separators, but not uniformly at the outer edges; pass the field value after HTTP field-level trimming.
