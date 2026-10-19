<a id="expires-h"></a>
<h1>
  <a href="../../../../../../resources/docs/reference/protocol/http/common/headers/expires/index.svg#gh-light-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/expires/index.svg" alt="Index for expires.h">
  </a>
  <a href="../../../../../../resources/docs/reference/protocol/http/common/headers/expires/index-dark.svg#gh-dark-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/expires/index-dark.svg" alt="Index for expires.h">
  </a>
</h1>

**Header:** [include/protocol/http/common/headers/expires.h](../../../../../../include/protocol/http/common/headers/expires.h)

|                                                                   expires | | RFC 9111 S5.3 Expires                                                     | | The "Expires" response header field gives the date/time after which the   | | response is considered stale. It is part of the HTTP caching model and is | | used to compute an explicit freshnes

**protocol::http::headers**

- [expires](#type-expires) · *type*

**protocol::http::headers::expires**

- [check](#method-check) · *method*

<a id="type-expires"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> expires

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class expires { ... };
```

|                                                                   expires | | RFC 9111 S5.3 Expires                                                     | | The "Expires" response header field gives the date/time after which the   | | response is considered stale. It is part of the HTTP caching model and is | | used to compute an explicit freshnes

**Source:** [include/protocol/http/common/headers/expires.h:122](../../../../../../include/protocol/http/common/headers/expires.h#L122)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::expires*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP expires header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/expires.h:127](../../../../../../include/protocol/http/common/headers/expires.h#L127)
