<a id="basic-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/auth/basic/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/auth/basic/index.svg" alt="Index for basic.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/auth/basic.h](../../../../../../include/protocol/http/common/auth/basic.h)

Validates HTTP Basic credentials through a user-supplied callback.

**protocol::http::auth**

- [basic](#type-basic) · *type*

**protocol::http::auth::basic**

- [basic](#constructor-basic) · *constructor*
- [authenticate](#method-authenticate) · *method*

<a id="type-basic"></a>

## basic

🟠 **TYPE** · *protocol::http::auth*

**Declaration synopsis:**

```cpp
class basic { ... };
```

Validates HTTP Basic credentials through a user-supplied callback.

**Source:** [include/protocol/http/common/auth/basic.h:43](../../../../../../include/protocol/http/common/auth/basic.h#L43)

<a id="constructor-basic"></a>

## basic

🟠 **CONSTRUCTOR** · *protocol::http::auth::basic*

```cpp
explicit basic(
      std::string_view realm,
      std::function<bool(std::string_view, std::string_view)> validator);
```

Stores the Basic validator and builds a quoted realm for the challenge header.

**Parameters:**

- **realm:** Initial value supplied to the constructor. Declaration: `std::string_view realm`.
- **validator:** Initial value supplied to the constructor. Declaration: `std::function<bool(std::string_view, std::string_view)> validator`.

**Errors:** Throws `std::invalid_argument` for a missing validator or a realm containing non-printable or non-ASCII bytes. Allocation failures may propagate.

**Source:** [include/protocol/http/common/auth/basic.h:48](../../../../../../include/protocol/http/common/auth/basic.h#L48)

<a id="method-authenticate"></a>

## authenticate

🟠 **METHOD** · *protocol::http::auth::basic*

```cpp
template <typename RQty>
  authentication_result authenticate(const RQty& request) const;
```

Checks the request's Authorization header using the Basic scheme and calls the configured validator for a well-formed credential.

**Template parameters:**

- **RQty:** `typename RQty`.

**Parameters:**

- **request:** Request being processed. Declaration: `const RQty& request`.

**Returns:** An `authentication_result` with the status and any challenge text.

**Source:** [include/protocol/http/common/auth/basic.h:67](../../../../../../include/protocol/http/common/auth/basic.h#L67)
