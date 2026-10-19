<a id="origin-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/origin/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/origin/index.svg" alt="Index for origin.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/origin.h](../../../../../../include/protocol/http/common/headers/origin.h)

|                                                                    origin | | RFC 6454 S7 Origin                                                        | | The "Origin" header field indicates the origin or origins that caused the | | user agent to issue an HTTP request. It is mainly used by servers to make | | access-control decisions based on th

**protocol::http::headers**

- [origin](#type-origin) · *type*

**protocol::http::headers::origin**

- [check](#method-check) · *method*

<a id="type-origin"></a>

## origin

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class origin { ... };
```

|                                                                    origin | | RFC 6454 S7 Origin                                                        | | The "Origin" header field indicates the origin or origins that caused the | | user agent to issue an HTTP request. It is mainly used by servers to make | | access-control decisions based on th

**Source:** [include/protocol/http/common/headers/origin.h:103](../../../../../../include/protocol/http/common/headers/origin.h#L103)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::origin*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP origin header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/origin.h:108](../../../../../../include/protocol/http/common/headers/origin.h#L108)
