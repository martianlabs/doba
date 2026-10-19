<a id="if-modified-since-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/if_modified_since/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/if_modified_since/index.svg" alt="Index for if_modified_since.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/if_modified_since.h](../../../../../../include/protocol/http/common/headers/if_modified_since.h)

|                                                         if-modified-since | | RFC 9110 S13.1.3 If-Modified-Since                                        | | The "If-Modified-Since" header field makes a GET or HEAD request          | | conditional on the selected representation having been modified after     | | the date supplied in the field value

**protocol::http::headers**

- [if_modified_since](#type-if-modified-since) · *type*

**protocol::http::headers::if_modified_since**

- [check](#method-check) · *method*

<a id="type-if-modified-since"></a>

## if_modified_since

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class if_modified_since { ... };
```

|                                                         if-modified-since | | RFC 9110 S13.1.3 If-Modified-Since                                        | | The "If-Modified-Since" header field makes a GET or HEAD request          | | conditional on the selected representation having been modified after     | | the date supplied in the field value

**Source:** [include/protocol/http/common/headers/if_modified_since.h:123](../../../../../../include/protocol/http/common/headers/if_modified_since.h#L123)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::if_modified_since*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP if-modified-since header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/if_modified_since.h:128](../../../../../../include/protocol/http/common/headers/if_modified_since.h#L128)
