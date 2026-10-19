<a id="proxy-connection-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/proxy_connection/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/proxy_connection/index.svg" alt="Index for proxy_connection.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/proxy_connection.h](../../../../../../include/protocol/http/common/headers/proxy_connection.h)

|                                                          proxy-connection | | Non-standard / legacy Proxy-Connection                                    | | The "Proxy-Connection" header field is a non-standard, legacy field used  | | by some clients when communicating with an explicit HTTP proxy. It was    | | intended to provide semantics simila

**protocol::http::headers**

- [x_proxy_connection](#type-x-proxy-connection) · *type*

**protocol::http::headers::x_proxy_connection**

- [check](#method-check) · *method*

<a id="type-x-proxy-connection"></a>

## x_proxy_connection

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class x_proxy_connection { ... };
```

|                                                          proxy-connection | | Non-standard / legacy Proxy-Connection                                    | | The "Proxy-Connection" header field is a non-standard, legacy field used  | | by some clients when communicating with an explicit HTTP proxy. It was    | | intended to provide semantics simila

**Source:** [include/protocol/http/common/headers/proxy_connection.h:100](../../../../../../include/protocol/http/common/headers/proxy_connection.h#L100)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::x_proxy_connection*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP proxy-connection header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/proxy_connection.h:105](../../../../../../include/protocol/http/common/headers/proxy_connection.h#L105)
