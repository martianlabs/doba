<a id="www-authenticate-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/www_authenticate/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/www_authenticate/index.svg" alt="Index for www_authenticate.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/www_authenticate.h](../../../../../../include/protocol/http/common/headers/www_authenticate.h)

|                                                          www-authenticate | | RFC 9110 S11.6.1 WWW-Authenticate                                         | | The "WWW-Authenticate" response header field indicates the authentication | | scheme(s) and parameters applicable to the target resource.               | | A server generating a 401 (Unauthori

**protocol::http::headers**

- [www_authenticate](#type-www-authenticate) · *type*

**protocol::http::headers::www_authenticate**

- [check](#method-check) · *method*

<a id="type-www-authenticate"></a>

## www_authenticate

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class www_authenticate { ... };
```

|                                                          www-authenticate | | RFC 9110 S11.6.1 WWW-Authenticate                                         | | The "WWW-Authenticate" response header field indicates the authentication | | scheme(s) and parameters applicable to the target resource.               | | A server generating a 401 (Unauthori

**Source:** [include/protocol/http/common/headers/www_authenticate.h:133](../../../../../../include/protocol/http/common/headers/www_authenticate.h#L133)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::www_authenticate*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP www-authenticate header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/www_authenticate.h:138](../../../../../../include/protocol/http/common/headers/www_authenticate.h#L138)
