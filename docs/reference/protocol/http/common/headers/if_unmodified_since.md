<a id="if-unmodified-since-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/if_unmodified_since/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/if_unmodified_since/index.svg" alt="Index for if_unmodified_since.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/if_unmodified_since.h](../../../../../../include/protocol/http/common/headers/if_unmodified_since.h)

|                                                       if-unmodified-since | | RFC 9110 S13.1.4 If-Unmodified-Since                                      | | The "If-Unmodified-Since" header field makes a request conditional on the | | selected representation's last modification date being earlier than or    | | equal to the date provided in the fi

**protocol::http::headers**

- [if_unmodified_since](#type-if-unmodified-since) · *type*

**protocol::http::headers::if_unmodified_since**

- [check](#method-check) · *method*

<a id="type-if-unmodified-since"></a>

## if_unmodified_since

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class if_unmodified_since { ... };
```

|                                                       if-unmodified-since | | RFC 9110 S13.1.4 If-Unmodified-Since                                      | | The "If-Unmodified-Since" header field makes a request conditional on the | | selected representation's last modification date being earlier than or    | | equal to the date provided in the fi

**Source:** [include/protocol/http/common/headers/if_unmodified_since.h:131](../../../../../../include/protocol/http/common/headers/if_unmodified_since.h#L131)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::if_unmodified_since*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP if-unmodified-since header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/if_unmodified_since.h:136](../../../../../../include/protocol/http/common/headers/if_unmodified_since.h#L136)
