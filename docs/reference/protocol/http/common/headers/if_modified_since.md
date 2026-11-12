<a id="if-modified-since"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/if_modified_since/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/if_modified_since/title.svg" alt="if_modified_since">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/if_modified_since/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/if_modified_since/title-dark.svg" alt="if_modified_since">
</a>

**Header:** [include/protocol/http/common/headers/if_modified_since.h](../../../../../../include/protocol/http/common/headers/if_modified_since.h)

Uses `date::check` for the three supported date layouts. It does not validate calendar ranges or compare a resource modification time.

<a id="api"></a>
<a id="type-if-modified-since"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> if_modified_since

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.
