<a id="if-range"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/if_range/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/if_range/title.svg" alt="if_range">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/if_range/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/if_range/title-dark.svg" alt="if_range">
</a>

**Header:** [include/protocol/http/common/headers/if_range.h](../../../../../../include/protocol/http/common/headers/if_range.h)

Accepts an entity tag (including a weak tag) or a date accepted by `date::check`. It does not decide whether a range can be served.

<a id="api"></a>
<a id="type-if-range"></a>

## if_range

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.
