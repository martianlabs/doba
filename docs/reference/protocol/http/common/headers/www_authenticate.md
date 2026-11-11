<a id="www-authenticate"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/www_authenticate/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/www_authenticate/title.svg" alt="www_authenticate">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/www_authenticate/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/www_authenticate/title-dark.svg" alt="www_authenticate">
</a>

**Header:** [include/protocol/http/common/headers/www_authenticate.h](../../../../../../include/protocol/http/common/headers/www_authenticate.h)

Checks a comma-separated challenge list, tracking whether following `name=value` members belong to the preceding scheme. Schemes may stand alone or use token68/parameters. It does not implement a challenge exchange.

<a id="api"></a>
<a id="type-www-authenticate"></a>

## www_authenticate

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.

List parsing skips empty comma elements and respects quoted commas. Spaces/tabs are trimmed before commas and after separators, but not uniformly at the outer edges; pass the field value after HTTP field-level trimming.
