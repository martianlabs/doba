<a id="basic"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/auth/basic/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/auth/basic/title.svg" alt="basic">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/auth/basic/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/auth/basic/title-dark.svg" alt="basic">
</a>

**Header:** [include/protocol/http/common/auth/basic.h](../../../../../../include/protocol/http/common/auth/basic.h)

Validates HTTP Basic credentials through a user-supplied callback.

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::auth

[basic](#type-basic)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::auth::basic

[authenticate](#method-authenticate)

<a id="api"></a>
<a href="../../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-basic"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> basic

**TYPE** · *protocol::http::auth*

**Declaration synopsis:**

```cpp
class basic { ... };
```

Validates HTTP Basic credentials through a user-supplied callback.

**Source:** [include/protocol/http/common/auth/basic.h](../../../../../../include/protocol/http/common/auth/basic.h)

<a id="method-authenticate"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> authenticate

**METHOD** · *protocol::http::auth::basic*

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

**Source:** [include/protocol/http/common/auth/basic.h](../../../../../../include/protocol/http/common/auth/basic.h)

## Construction

```cpp
explicit basic(std::string_view realm,
               std::function<bool(std::string_view, std::string_view)> validator);
```

The realm is copied into the challenge, escaping quotes and backslashes. Only printable ASCII realm bytes are accepted. A missing validator or invalid realm throws `std::invalid_argument`; allocation failures may propagate.

The provider is shared by the router, so validators must support concurrent calls. Callback arguments are borrowed for the call; copy them if they must be retained. Validator exceptions propagate to the caller.

Duplicate `Authorization` fields produce `kInvalidRequest` without a challenge. Missing/wrong schemes, malformed Base64, nonzero padding bits, missing colon, control bytes, and rejected credentials produce `kUnauthenticated` with the configured Basic challenge. Scheme matching ignores ASCII case; spaces separate scheme and credential. The first decoded colon divides user from password; either side may be empty and later colons belong to the password. A validator result of true produces `kAccepted`.
