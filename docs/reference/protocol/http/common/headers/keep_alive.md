<a id="keep-alive"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/keep_alive/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/keep_alive/title.svg" alt="keep_alive">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/keep_alive/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/keep_alive/title-dark.svg" alt="keep_alive">
</a>

**Header:** [include/protocol/http/common/headers/keep_alive.h](../../../../../../include/protocol/http/common/headers/keep_alive.h)

Checks comma-separated `name=value` parameters using the authentication-parameter helper, including quoted values and whitespace around `=`. The class is named `x_keep_alive`. It does not configure connection timeouts.

<a id="api"></a>
<a id="type-x-keep-alive"></a>

## x_keep_alive

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.

List parsing skips empty comma elements and respects quoted commas. Spaces/tabs are trimmed before commas and after separators, but not uniformly at the outer edges; pass the field value after HTTP field-level trimming.
