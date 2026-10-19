<a id="accept-charset"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_charset/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_charset/index.svg" alt="accept_charset">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_charset/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_charset/index-dark.svg" alt="accept_charset">
</a>

**Header:** [include/protocol/http/common/headers/accept_charset.h](../../../../../../include/protocol/http/common/headers/accept_charset.h)

|                                                            accept-charset | | RFC 9110 S12.5.2 Accept-Charset                                           | | The "Accept-Charset" header field can be sent by a user agent to indicate | | what charsets are preferred for textual response content.                 | | Each member of the list identifies e

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[accept_charset](#type-accept-charset)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::accept_charset

[check](#method-check)

<a id="api-details"></a>
<a href="../../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-accept-charset"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> accept_charset

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class accept_charset { ... };
```

|                                                            accept-charset | | RFC 9110 S12.5.2 Accept-Charset                                           | | The "Accept-Charset" header field can be sent by a user agent to indicate | | what charsets are preferred for textual response content.                 | | Each member of the list identifies e

**Source:** [include/protocol/http/common/headers/accept_charset.h:98](../../../../../../include/protocol/http/common/headers/accept_charset.h#L98)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::accept_charset*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP accept-charset header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/accept_charset.h:103](../../../../../../include/protocol/http/common/headers/accept_charset.h#L103)
