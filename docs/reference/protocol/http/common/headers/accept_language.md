<a id="accept-language"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_language/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_language/index.svg" alt="accept_language">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_language/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_language/index-dark.svg" alt="accept_language">
</a>

**Header:** [include/protocol/http/common/headers/accept_language.h](../../../../../../include/protocol/http/common/headers/accept_language.h)

|                                                           accept-language | | RFC 9110 S12.5.4 Accept-Language                                          | | The "Accept-Language" request header field allows a user agent to         | | indicate the natural languages preferred for response content.            | | Each language range can have an opti

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[accept_language](#type-accept-language)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::accept_language

[check](#method-check)

<a id="api-details"></a>
<a href="../../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-accept-language"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> accept_language

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class accept_language { ... };
```

|                                                           accept-language | | RFC 9110 S12.5.4 Accept-Language                                          | | The "Accept-Language" request header field allows a user agent to         | | indicate the natural languages preferred for response content.            | | Each language range can have an opti

**Source:** [include/protocol/http/common/headers/accept_language.h:123](../../../../../../include/protocol/http/common/headers/accept_language.h#L123)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::accept_language*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP accept-language header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/accept_language.h:128](../../../../../../include/protocol/http/common/headers/accept_language.h#L128)
