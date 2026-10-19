<a id="etag-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/etag/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/etag/index.svg" alt="Index for etag.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/etag.h](../../../../../../include/protocol/http/common/headers/etag.h)

|                                                                      etag | | RFC 9110 S8.8.3 ETag                                                      | | The "ETag" response header field provides the current entity tag for the  | | selected representation, as determined at the conclusion of handling the  | | request.

**protocol::http::headers**

- [etag](#type-etag) · *type*

**protocol::http::headers::etag**

- [check](#method-check) · *method*

<a id="type-etag"></a>

## etag

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class etag { ... };
```

|                                                                      etag | | RFC 9110 S8.8.3 ETag                                                      | | The "ETag" response header field provides the current entity tag for the  | | selected representation, as determined at the conclusion of handling the  | | request.

**Source:** [include/protocol/http/common/headers/etag.h:100](../../../../../../include/protocol/http/common/headers/etag.h#L100)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::etag*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP etag header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/etag.h:105](../../../../../../include/protocol/http/common/headers/etag.h#L105)
