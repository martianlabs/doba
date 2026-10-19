<a id="accept-charset-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_charset/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_charset/index.svg" alt="Index for accept_charset.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/accept_charset.h](../../../../../../include/protocol/http/common/headers/accept_charset.h)

|                                                            accept-charset | | RFC 9110 S12.5.2 Accept-Charset                                           | | The "Accept-Charset" header field can be sent by a user agent to indicate | | what charsets are preferred for textual response content.                 | | Each member of the list identifies e

**protocol::http::headers**

- [accept_charset](#type-accept-charset) · *type*

**protocol::http::headers::accept_charset**

- [check](#method-check) · *method*

<a id="type-accept-charset"></a>

## accept_charset

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class accept_charset { ... };
```

|                                                            accept-charset | | RFC 9110 S12.5.2 Accept-Charset                                           | | The "Accept-Charset" header field can be sent by a user agent to indicate | | what charsets are preferred for textual response content.                 | | Each member of the list identifies e

**Source:** [include/protocol/http/common/headers/accept_charset.h:98](../../../../../../include/protocol/http/common/headers/accept_charset.h#L98)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::accept_charset*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP accept-charset header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/accept_charset.h:103](../../../../../../include/protocol/http/common/headers/accept_charset.h#L103)
