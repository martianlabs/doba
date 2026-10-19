<a id="bearer-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/auth/bearer/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/auth/bearer/index.svg" alt="Index for bearer.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/auth/bearer.h](../../../../../../include/protocol/http/common/auth/bearer.h)

Validates HTTP Bearer credentials through a user-supplied callback.

**protocol::http::auth**

- [bearer](#type-bearer) · *type*

**protocol::http::auth::bearer**

- [bearer](#constructor-bearer) · *constructor*
- [authenticate](#method-authenticate) · *method*

<a id="type-bearer"></a>

## bearer

🟠 **TYPE** · *protocol::http::auth*

**Declaration synopsis:**

```cpp
class bearer { ... };
```

Validates HTTP Bearer credentials through a user-supplied callback.

**Source:** [include/protocol/http/common/auth/bearer.h:43](../../../../../../include/protocol/http/common/auth/bearer.h#L43)

<a id="constructor-bearer"></a>

## bearer

🟠 **CONSTRUCTOR** · *protocol::http::auth::bearer*

```cpp
explicit bearer(std::string_view realm,
                  std::function<bool(std::string_view)> validator);
```

Stores the Bearer validator and builds a quoted realm for the challenge header.

**Parameters:**

- **realm:** Initial value supplied to the constructor. Declaration: `std::string_view realm`.
- **validator:** Initial value supplied to the constructor. Declaration: `std::function<bool(std::string_view)> validator`.

**Errors:** Throws `std::invalid_argument` for a missing validator or a realm containing non-printable or non-ASCII bytes. Allocation failures may propagate.

**Source:** [include/protocol/http/common/auth/bearer.h:48](../../../../../../include/protocol/http/common/auth/bearer.h#L48)

<a id="method-authenticate"></a>

## authenticate

🟠 **METHOD** · *protocol::http::auth::bearer*

```cpp
template <typename RQty>
  authentication_result authenticate(const RQty& request) const;
```

Checks the request's Authorization header using the Bearer scheme and calls the configured validator for a well-formed credential.

**Template parameters:**

- **RQty:** `typename RQty`.

**Parameters:**

- **request:** Request being processed. Declaration: `const RQty& request`.

**Returns:** An `authentication_result` with the status and any challenge text.

**Source:** [include/protocol/http/common/auth/bearer.h:66](../../../../../../include/protocol/http/common/auth/bearer.h#L66)
