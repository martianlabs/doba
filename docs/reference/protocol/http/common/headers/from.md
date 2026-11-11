<a id="from"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/from/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/from/title.svg" alt="from">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/from/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/from/title-dark.svg" alt="from">
</a>

**Header:** [include/protocol/http/common/headers/from.h](../../../../../../include/protocol/http/common/headers/from.h)

Checks one mailbox, either an address or a display name with an angle-bracketed address. Comments and supported quoted strings are accepted. It does not resolve the domain or verify delivery.

<a id="api"></a>
<a id="type-from"></a>

## from

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.
