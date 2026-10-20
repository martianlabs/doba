<a id="set-cookie"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/set_cookie/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/set_cookie/title.svg" alt="set_cookie">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/set_cookie/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/set_cookie/title-dark.svg" alt="set_cookie">
</a>

**Header:** [include/protocol/http/common/headers/set_cookie.h](../../../../../../include/protocol/http/common/headers/set_cookie.h)

|                                                               set-cookie  | | RFC 6265 S4.1 Set-Cookie                                                  | | The "Set-Cookie" response header field is used to send a cookie from the  | | server to the user agent. The user agent stores the cookie together with  | | its attributes and can return the co

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[set_cookie](#type-set-cookie)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::set_cookie

[check](#method-check)

<a id="api"></a>
<a href="../../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-set-cookie"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> set_cookie

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class set_cookie { ... };
```

|                                                               set-cookie  | | RFC 6265 S4.1 Set-Cookie                                                  | | The "Set-Cookie" response header field is used to send a cookie from the  | | server to the user agent. The user agent stores the cookie together with  | | its attributes and can return the co

**Source:** [include/protocol/http/common/headers/set_cookie.h:121](../../../../../../include/protocol/http/common/headers/set_cookie.h#L121)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::set_cookie*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP set-cookie header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/set_cookie.h:126](../../../../../../include/protocol/http/common/headers/set_cookie.h#L126)
