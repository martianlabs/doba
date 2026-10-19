<a id="if-unmodified-since"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/if_unmodified_since/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/if_unmodified_since/index.svg" alt="if_unmodified_since">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/if_unmodified_since/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/if_unmodified_since/index-dark.svg" alt="if_unmodified_since">
</a>

**Header:** [include/protocol/http/common/headers/if_unmodified_since.h](../../../../../../include/protocol/http/common/headers/if_unmodified_since.h)

|                                                       if-unmodified-since | | RFC 9110 S13.1.4 If-Unmodified-Since                                      | | The "If-Unmodified-Since" header field makes a request conditional on the | | selected representation's last modification date being earlier than or    | | equal to the date provided in the fi

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[if_unmodified_since](#type-if-unmodified-since)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::if_unmodified_since

[check](#method-check)

<a id="type-if-unmodified-since"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> if_unmodified_since

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class if_unmodified_since { ... };
```

|                                                       if-unmodified-since | | RFC 9110 S13.1.4 If-Unmodified-Since                                      | | The "If-Unmodified-Since" header field makes a request conditional on the | | selected representation's last modification date being earlier than or    | | equal to the date provided in the fi

**Source:** [include/protocol/http/common/headers/if_unmodified_since.h:131](../../../../../../include/protocol/http/common/headers/if_unmodified_since.h#L131)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::if_unmodified_since*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP if-unmodified-since header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/if_unmodified_since.h:136](../../../../../../include/protocol/http/common/headers/if_unmodified_since.h#L136)
