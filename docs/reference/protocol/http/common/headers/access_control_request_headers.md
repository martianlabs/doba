<a id="access-control-request-headers"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_request_headers/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_request_headers/title.svg" alt="access_control_request_headers">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_request_headers/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_request_headers/title-dark.svg" alt="access_control_request_headers">
</a>

**Header:** [include/protocol/http/common/headers/access_control_request_headers.h](../../../../../../include/protocol/http/common/headers/access_control_request_headers.h)

Requires at least one header-name token in a comma-separated list. It does not apply a CORS policy.

<a id="api"></a>
<a id="type-access-control-request-headers"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> access_control_request_headers

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.

List parsing skips empty comma elements and respects quoted commas. Spaces/tabs are trimmed before commas and after separators, but not uniformly at the outer edges; pass the field value after HTTP field-level trimming.
