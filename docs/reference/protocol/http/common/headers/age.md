<a id="age"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/age/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/age/title.svg" alt="age">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/age/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/age/title-dark.svg" alt="age">
</a>

**Header:** [include/protocol/http/common/headers/age.h](../../../../../../include/protocol/http/common/headers/age.h)

Accepts a nonempty sequence of decimal digits. It does not convert seconds or impose a numeric limit.

<a id="api"></a>
<a id="type-age"></a>

## age

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.
