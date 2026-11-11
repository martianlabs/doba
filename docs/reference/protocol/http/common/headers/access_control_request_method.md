<a id="access-control-request-method"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_request_method/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_request_method/title.svg" alt="access_control_request_method">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_request_method/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_request_method/title-dark.svg" alt="access_control_request_method">
</a>

**Header:** [include/protocol/http/common/headers/access_control_request_method.h](../../../../../../include/protocol/http/common/headers/access_control_request_method.h)

Accepts a single nonempty method token, including extension methods. It does not check registered routes.

<a id="api"></a>
<a id="type-access-control-request-method"></a>

## access_control_request_method

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.
