<a id="pragma-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/pragma/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/pragma/index.svg" alt="Index for pragma.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/pragma.h](../../../../../../include/protocol/http/common/headers/pragma.h)

|                                                                    pragma | | RFC 9111 S5.4 Pragma                                                      | | The "Pragma" request header field was originally defined for HTTP/1.0     | | caches, allowing clients to send a "no-cache" request before              | | Cache-Control existed.

**protocol::http::headers**

- [pragma](#type-pragma) · *type*

**protocol::http::headers::pragma**

- [check](#method-check) · *method*

<a id="type-pragma"></a>

## pragma

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class pragma { ... };
```

|                                                                    pragma | | RFC 9111 S5.4 Pragma                                                      | | The "Pragma" request header field was originally defined for HTTP/1.0     | | caches, allowing clients to send a "no-cache" request before              | | Cache-Control existed.

**Source:** [include/protocol/http/common/headers/pragma.h:107](../../../../../../include/protocol/http/common/headers/pragma.h#L107)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::pragma*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP pragma header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/pragma.h:112](../../../../../../include/protocol/http/common/headers/pragma.h#L112)
