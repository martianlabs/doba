<a id="accept-encoding-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_encoding/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_encoding/index.svg" alt="Index for accept_encoding.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/accept_encoding.h](../../../../../../include/protocol/http/common/headers/accept_encoding.h)

|                                                           accept-encoding | | RFC 9110 S12.5.3 Accept-Encoding                                          | | The "Accept-Encoding" header field indicates preferences regarding the    | | use of content codings.                                                   | | In a request, it identifies the cont

**protocol::http::headers**

- [accept_encoding](#type-accept-encoding) · *type*

**protocol::http::headers::accept_encoding**

- [check](#method-check) · *method*

<a id="type-accept-encoding"></a>

## accept_encoding

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class accept_encoding { ... };
```

|                                                           accept-encoding | | RFC 9110 S12.5.3 Accept-Encoding                                          | | The "Accept-Encoding" header field indicates preferences regarding the    | | use of content codings.                                                   | | In a request, it identifies the cont

**Source:** [include/protocol/http/common/headers/accept_encoding.h:111](../../../../../../include/protocol/http/common/headers/accept_encoding.h#L111)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::accept_encoding*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP accept-encoding header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/accept_encoding.h:116](../../../../../../include/protocol/http/common/headers/accept_encoding.h#L116)
