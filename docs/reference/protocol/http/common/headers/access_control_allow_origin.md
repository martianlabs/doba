<a id="access-control-allow-origin"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_allow_origin/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_allow_origin/title.svg" alt="access_control_allow_origin">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_allow_origin/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_allow_origin/title-dark.svg" alt="access_control_allow_origin">
</a>

**Header:** [include/protocol/http/common/headers/access_control_allow_origin.h](../../../../../../include/protocol/http/common/headers/access_control_allow_origin.h)

Accepts exactly `*`, exactly `null`, or a serialized `scheme://host[:port]` origin. The host/port helper accepts empty hosts and empty or arbitrarily large decimal ports; this is a syntax check, not an origin allowlist.

<a id="api"></a>
<a id="type-access-control-allow-origin"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> access_control_allow_origin

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.
