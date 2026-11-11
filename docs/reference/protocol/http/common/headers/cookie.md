<a id="cookie"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/cookie/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/cookie/title.svg" alt="cookie">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/cookie/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/cookie/title-dark.svg" alt="cookie">
</a>

**Header:** [include/protocol/http/common/headers/cookie.h](../../../../../../include/protocol/http/common/headers/cookie.h)

Requires cookie `name=value` pairs separated by exactly `; `. Values may be enclosed in double quotes, but still use cookie octets: quotes do not enable escapes or spaces. Empty values are allowed; an empty header is rejected.

<a id="api"></a>
<a id="type-cookie"></a>

## cookie

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.
