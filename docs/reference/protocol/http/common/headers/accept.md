<a id="accept-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/accept/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/accept/index.svg" alt="Index for accept.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/accept.h](../../../../../../include/protocol/http/common/headers/accept.h)

|                                                                    accept | | RFC 9110 S12.5.1 Accept                                                   | | The "Accept" header field allows a user agent to specify its preferences  | | regarding the media types of the response representation. It can be used  | | to restrict the response to a set of

**protocol::http::headers**

- [accept](#type-accept) · *type*

**protocol::http::headers::accept**

- [check](#method-check) · *method*

<a id="type-accept"></a>

## accept

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class accept { ... };
```

|                                                                    accept | | RFC 9110 S12.5.1 Accept                                                   | | The "Accept" header field allows a user agent to specify its preferences  | | regarding the media types of the response representation. It can be used  | | to restrict the response to a set of

**Source:** [include/protocol/http/common/headers/accept.h:144](../../../../../../include/protocol/http/common/headers/accept.h#L144)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::accept*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP accept header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/accept.h:149](../../../../../../include/protocol/http/common/headers/accept.h#L149)
