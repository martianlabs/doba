<a id="cache-control-h"></a>
<h1>
  <a href="../../../../../../resources/docs/reference/protocol/http/common/headers/cache_control/index.svg#gh-light-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/cache_control/index.svg" alt="Index for cache_control.h">
  </a>
  <a href="../../../../../../resources/docs/reference/protocol/http/common/headers/cache_control/index-dark.svg#gh-dark-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/cache_control/index-dark.svg" alt="Index for cache_control.h">
  </a>
</h1>

**Header:** [include/protocol/http/common/headers/cache_control.h](../../../../../../include/protocol/http/common/headers/cache_control.h)

|                                                             cache-control | | RFC 9111 S5.2 Cache-Control                                               | | The "Cache-Control" header field carries a comma-separated list of cache  | | directives along a request/response chain.                                | | Cache directives are unidirectional:

**protocol::http::headers**

- [cache_control](#type-cache-control) · *type*

**protocol::http::headers::cache_control**

- [check](#method-check) · *method*

<a id="type-cache-control"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> cache_control

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class cache_control { ... };
```

|                                                             cache-control | | RFC 9111 S5.2 Cache-Control                                               | | The "Cache-Control" header field carries a comma-separated list of cache  | | directives along a request/response chain.                                | | Cache directives are unidirectional:

**Source:** [include/protocol/http/common/headers/cache_control.h:128](../../../../../../include/protocol/http/common/headers/cache_control.h#L128)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::cache_control*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP cache-control header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/cache_control.h:133](../../../../../../include/protocol/http/common/headers/cache_control.h#L133)
