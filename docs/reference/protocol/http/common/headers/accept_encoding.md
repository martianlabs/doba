<a id="accept-encoding"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_encoding/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_encoding/index.svg" alt="accept_encoding">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_encoding/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_encoding/index-dark.svg" alt="accept_encoding">
</a>

**Header:** [include/protocol/http/common/headers/accept_encoding.h](../../../../../../include/protocol/http/common/headers/accept_encoding.h)

|                                                           accept-encoding | | RFC 9110 S12.5.3 Accept-Encoding                                          | | The "Accept-Encoding" header field indicates preferences regarding the    | | use of content codings.                                                   | | In a request, it identifies the cont

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[accept_encoding](#type-accept-encoding)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::accept_encoding

[check](#method-check)

<a id="api-details"></a>
<a href="../../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-accept-encoding"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> accept_encoding

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class accept_encoding { ... };
```

|                                                           accept-encoding | | RFC 9110 S12.5.3 Accept-Encoding                                          | | The "Accept-Encoding" header field indicates preferences regarding the    | | use of content codings.                                                   | | In a request, it identifies the cont

**Source:** [include/protocol/http/common/headers/accept_encoding.h:111](../../../../../../include/protocol/http/common/headers/accept_encoding.h#L111)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::accept_encoding*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP accept-encoding header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/accept_encoding.h:116](../../../../../../include/protocol/http/common/headers/accept_encoding.h#L116)
