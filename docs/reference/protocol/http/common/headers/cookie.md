<a id="cookie"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/cookie/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/cookie/index.svg" alt="cookie">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/cookie/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/cookie/index-dark.svg" alt="cookie">
</a>

**Header:** [include/protocol/http/common/headers/cookie.h](../../../../../../include/protocol/http/common/headers/cookie.h)

|                                                                    cookie | | RFC 6265 S4.2 Cookie                                                      | | The "Cookie" header field is sent by a user agent in an HTTP request to   | | return stored cookies to the origin server. The field contains cookies    | | previously received from Set-Cookie

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[cookie](#type-cookie)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::cookie

[check](#method-check)

<a id="api-details"></a>
<a href="../../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-cookie"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> cookie

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class cookie { ... };
```

|                                                                    cookie | | RFC 6265 S4.2 Cookie                                                      | | The "Cookie" header field is sent by a user agent in an HTTP request to   | | return stored cookies to the origin server. The field contains cookies    | | previously received from Set-Cookie

**Source:** [include/protocol/http/common/headers/cookie.h:105](../../../../../../include/protocol/http/common/headers/cookie.h#L105)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::cookie*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP cookie header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/cookie.h:110](../../../../../../include/protocol/http/common/headers/cookie.h#L110)
