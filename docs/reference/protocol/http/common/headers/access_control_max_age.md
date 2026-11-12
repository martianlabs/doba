<a id="access-control-max-age"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_max_age/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_max_age/title.svg" alt="access_control_max_age">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_max_age/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_max_age/title-dark.svg" alt="access_control_max_age">
</a>

**Header:** [include/protocol/http/common/headers/access_control_max_age.h](../../../../../../include/protocol/http/common/headers/access_control_max_age.h)

Accepts a nonempty sequence of decimal digits. It does not convert the value or impose a numeric limit.

<a id="api"></a>
<a id="type-access-control-max-age"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> access_control_max_age

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.
