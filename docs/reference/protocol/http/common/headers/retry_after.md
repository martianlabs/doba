<a id="retry-after-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/retry_after/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/retry_after/index.svg" alt="Index for retry_after.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/retry_after.h](../../../../../../include/protocol/http/common/headers/retry_after.h)

|                                                               retry-after | | RFC 9110 S10.2.3 Retry-After                                              | | The "Retry-After" response header field indicates how long the user agent | | ought to wait before making a follow-up request.                          | | The field value can be either an HTT

**protocol::http::headers**

- [retry_after](#type-retry-after) · *type*

**protocol::http::headers::retry_after**

- [check](#method-check) · *method*

<a id="type-retry-after"></a>

## retry_after

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class retry_after { ... };
```

|                                                               retry-after | | RFC 9110 S10.2.3 Retry-After                                              | | The "Retry-After" response header field indicates how long the user agent | | ought to wait before making a follow-up request.                          | | The field value can be either an HTT

**Source:** [include/protocol/http/common/headers/retry_after.h:112](../../../../../../include/protocol/http/common/headers/retry_after.h#L112)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::retry_after*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP retry-after header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/retry_after.h:117](../../../../../../include/protocol/http/common/headers/retry_after.h#L117)
