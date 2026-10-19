<a id="authorization-h"></a>
<h1>
  <a href="../../../../../../resources/docs/reference/protocol/http/common/headers/authorization/index.svg#gh-light-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/authorization/index.svg" alt="Index for authorization.h">
  </a>
  <a href="../../../../../../resources/docs/reference/protocol/http/common/headers/authorization/index-dark.svg#gh-dark-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/authorization/index-dark.svg" alt="Index for authorization.h">
  </a>
</h1>

**Header:** [include/protocol/http/common/headers/authorization.h](../../../../../../include/protocol/http/common/headers/authorization.h)

|                                                             authorization | | RFC 9110 S11.6.2 Authorization                                            | | The "Authorization" header field allows a user agent to authenticate      | | itself with an origin server, usually after receiving a 401               | | (Unauthorized) response containing o

**protocol::http::headers**

- [authorization](#type-authorization) · *type*

**protocol::http::headers::authorization**

- [check](#method-check) · *method*

<a id="type-authorization"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> authorization

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class authorization { ... };
```

|                                                             authorization | | RFC 9110 S11.6.2 Authorization                                            | | The "Authorization" header field allows a user agent to authenticate      | | itself with an origin server, usually after receiving a 401               | | (Unauthorized) response containing o

**Source:** [include/protocol/http/common/headers/authorization.h:109](../../../../../../include/protocol/http/common/headers/authorization.h#L109)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::authorization*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP authorization header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/authorization.h:114](../../../../../../include/protocol/http/common/headers/authorization.h#L114)
