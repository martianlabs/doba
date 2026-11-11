<a id="retry-after"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/retry_after/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/retry_after/title.svg" alt="retry_after">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/retry_after/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/retry_after/title-dark.svg" alt="retry_after">
</a>

**Header:** [include/protocol/http/common/headers/retry_after.h](../../../../../../include/protocol/http/common/headers/retry_after.h)

Accepts nonempty decimal digits or a date accepted by `date::check`. It does not convert seconds, validate calendar ranges, or schedule a retry.

<a id="api"></a>
<a id="type-retry-after"></a>

## retry_after

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.
