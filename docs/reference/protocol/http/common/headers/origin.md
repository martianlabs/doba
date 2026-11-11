<a id="origin"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/origin/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/origin/title.svg" alt="origin">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/origin/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/origin/title-dark.svg" alt="origin">
</a>

**Header:** [include/protocol/http/common/headers/origin.h](../../../../../../include/protocol/http/common/headers/origin.h)

Accepts exactly `null` or serialized origins separated by single spaces. Leading, trailing, or repeated spaces are rejected. Host/port checks permit empty hosts and empty or arbitrarily large decimal ports; this does not apply an origin policy.

<a id="api"></a>
<a id="type-origin"></a>

## origin

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.
