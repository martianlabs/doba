<a id="if-match"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/if_match/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/if_match/title.svg" alt="if_match">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/if_match/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/if_match/title-dark.svg" alt="if_match">
</a>

**Header:** [include/protocol/http/common/headers/if_match.h](../../../../../../include/protocol/http/common/headers/if_match.h)

Accepts exactly `*` or a comma-separated entity-tag list, including weak tags. Empty list elements are skipped and an empty value is accepted. This checks syntax only; it does not perform the strong comparison required by a resource precondition.

<a id="api"></a>
<a id="type-if-match"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> if_match

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.
