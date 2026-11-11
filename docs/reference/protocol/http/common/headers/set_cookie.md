<a id="set-cookie"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/set_cookie/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/set_cookie/title.svg" alt="set_cookie">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/set_cookie/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/set_cookie/title-dark.svg" alt="set_cookie">
</a>

**Header:** [include/protocol/http/common/headers/set_cookie.h](../../../../../../include/protocol/http/common/headers/set_cookie.h)

Requires a cookie pair followed by optional attributes separated by exactly `; `. Attributes are checked as nonempty attribute text only: names, dates, flags, and numeric values are not interpreted. The checker does not store cookies.

<a id="api"></a>
<a id="type-set-cookie"></a>

## set_cookie

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.
