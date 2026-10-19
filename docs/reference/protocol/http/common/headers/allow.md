<a id="allow-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/allow/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/allow/index.svg" alt="Index for allow.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/allow.h](../../../../../../include/protocol/http/common/headers/allow.h)

|                                                                     allow | | RFC 9110 S10.2.1 Allow                                                    | | The "Allow" header field lists the set of methods advertised as supported | | by the target resource. Its purpose is strictly to inform the recipient   | | of valid request methods associated

**protocol::http::headers**

- [allow](#type-allow) · *type*

**protocol::http::headers::allow**

- [check](#method-check) · *method*

<a id="type-allow"></a>

## allow

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class allow { ... };
```

|                                                                     allow | | RFC 9110 S10.2.1 Allow                                                    | | The "Allow" header field lists the set of methods advertised as supported | | by the target resource. Its purpose is strictly to inform the recipient   | | of valid request methods associated

**Source:** [include/protocol/http/common/headers/allow.h:94](../../../../../../include/protocol/http/common/headers/allow.h#L94)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::allow*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP allow header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/allow.h:99](../../../../../../include/protocol/http/common/headers/allow.h#L99)
