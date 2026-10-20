<a id="content-language"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/content_language/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/content_language/title.svg" alt="content_language">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/content_language/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/content_language/title-dark.svg" alt="content_language">
</a>

**Header:** [include/protocol/http/common/headers/content_language.h](../../../../../../include/protocol/http/common/headers/content_language.h)

|                                                          content-language | | RFC 9110 S8.5 Content-Language                                            | | The "Content-Language" header field identifies the natural language(s) of | | the intended audience for a representation. It does not necessarily list  | | every language that appears within t

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[content_language](#type-content-language)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::content_language

[check](#method-check)

<a id="api"></a>
<a href="../../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-content-language"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> content_language

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class content_language { ... };
```

|                                                          content-language | | RFC 9110 S8.5 Content-Language                                            | | The "Content-Language" header field identifies the natural language(s) of | | the intended audience for a representation. It does not necessarily list  | | every language that appears within t

**Source:** [include/protocol/http/common/headers/content_language.h:129](../../../../../../include/protocol/http/common/headers/content_language.h#L129)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::content_language*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP content-language header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/content_language.h:134](../../../../../../include/protocol/http/common/headers/content_language.h#L134)
