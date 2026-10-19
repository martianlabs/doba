<a id="if-modified-since"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/if_modified_since/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/if_modified_since/index.svg" alt="if_modified_since">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/if_modified_since/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/if_modified_since/index-dark.svg" alt="if_modified_since">
</a>

**Header:** [include/protocol/http/common/headers/if_modified_since.h](../../../../../../include/protocol/http/common/headers/if_modified_since.h)

|                                                         if-modified-since | | RFC 9110 S13.1.3 If-Modified-Since                                        | | The "If-Modified-Since" header field makes a GET or HEAD request          | | conditional on the selected representation having been modified after     | | the date supplied in the field value

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[if_modified_since](#type-if-modified-since)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::if_modified_since

[check](#method-check)

<a id="type-if-modified-since"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> if_modified_since

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class if_modified_since { ... };
```

|                                                         if-modified-since | | RFC 9110 S13.1.3 If-Modified-Since                                        | | The "If-Modified-Since" header field makes a GET or HEAD request          | | conditional on the selected representation having been modified after     | | the date supplied in the field value

**Source:** [include/protocol/http/common/headers/if_modified_since.h:123](../../../../../../include/protocol/http/common/headers/if_modified_since.h#L123)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::if_modified_since*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP if-modified-since header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/if_modified_since.h:128](../../../../../../include/protocol/http/common/headers/if_modified_since.h#L128)
