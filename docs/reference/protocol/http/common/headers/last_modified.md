<a id="last-modified-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/last_modified/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/last_modified/index.svg" alt="Index for last_modified.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/last_modified.h](../../../../../../include/protocol/http/common/headers/last_modified.h)

|                                                             last-modified | | RFC 9110 S8.8.2 Last-Modified                                             | | The "Last-Modified" header field in a response provides the date and time | | at which the origin server believes the selected representation was last  | | modified, as determined when request

**protocol::http::headers**

- [last_modified](#type-last-modified) · *type*

**protocol::http::headers::last_modified**

- [check](#method-check) · *method*

<a id="type-last-modified"></a>

## last_modified

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class last_modified { ... };
```

|                                                             last-modified | | RFC 9110 S8.8.2 Last-Modified                                             | | The "Last-Modified" header field in a response provides the date and time | | at which the origin server believes the selected representation was last  | | modified, as determined when request

**Source:** [include/protocol/http/common/headers/last_modified.h:132](../../../../../../include/protocol/http/common/headers/last_modified.h#L132)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::last_modified*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP last-modified header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/last_modified.h:137](../../../../../../include/protocol/http/common/headers/last_modified.h#L137)
