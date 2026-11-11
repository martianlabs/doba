<a id="if-unmodified-since"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/if_unmodified_since/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/if_unmodified_since/title.svg" alt="if_unmodified_since">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/if_unmodified_since/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/if_unmodified_since/title-dark.svg" alt="if_unmodified_since">
</a>

**Header:** [include/protocol/http/common/headers/if_unmodified_since.h](../../../../../../include/protocol/http/common/headers/if_unmodified_since.h)

Uses `date::check` for the three supported date layouts. It does not validate calendar ranges or evaluate a resource precondition.

<a id="api"></a>
<a id="type-if-unmodified-since"></a>

## if_unmodified_since

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.
