<a id="access-control-expose-headers"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_expose_headers/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_expose_headers/title.svg" alt="access_control_expose_headers">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_expose_headers/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_expose_headers/title-dark.svg" alt="access_control_expose_headers">
</a>

**Header:** [include/protocol/http/common/headers/access_control_expose_headers.h](../../../../../../include/protocol/http/common/headers/access_control_expose_headers.h)

Checks a comma-separated list of header-name tokens. It does not apply a CORS policy.

<a id="api"></a>
<a id="type-access-control-expose-headers"></a>

## access_control_expose_headers

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.

List parsing skips empty comma elements and respects quoted commas. Spaces/tabs are trimmed before commas and after separators, but not uniformly at the outer edges; pass the field value after HTTP field-level trimming.
