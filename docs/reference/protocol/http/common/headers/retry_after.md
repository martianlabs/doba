<a id="retry-after"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/retry_after/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/retry_after/index.svg" alt="retry_after">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/retry_after/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/retry_after/index-dark.svg" alt="retry_after">
</a>

**Header:** [include/protocol/http/common/headers/retry_after.h](../../../../../../include/protocol/http/common/headers/retry_after.h)

|                                                               retry-after | | RFC 9110 S10.2.3 Retry-After                                              | | The "Retry-After" response header field indicates how long the user agent | | ought to wait before making a follow-up request.                          | | The field value can be either an HTT

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[retry_after](#type-retry-after)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::retry_after

[check](#method-check)

<a id="type-retry-after"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> retry_after

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class retry_after { ... };
```

|                                                               retry-after | | RFC 9110 S10.2.3 Retry-After                                              | | The "Retry-After" response header field indicates how long the user agent | | ought to wait before making a follow-up request.                          | | The field value can be either an HTT

**Source:** [include/protocol/http/common/headers/retry_after.h:112](../../../../../../include/protocol/http/common/headers/retry_after.h#L112)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::retry_after*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP retry-after header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/retry_after.h:117](../../../../../../include/protocol/http/common/headers/retry_after.h#L117)
