<a id="bearer"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/auth/bearer/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/auth/bearer/title.svg" alt="bearer">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/auth/bearer/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/auth/bearer/title-dark.svg" alt="bearer">
</a>

**Header:** [include/protocol/http/common/auth/bearer.h](../../../../../../include/protocol/http/common/auth/bearer.h)

Validates HTTP Bearer credentials through a user-supplied callback.

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::auth

[bearer](#type-bearer)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::auth::bearer

[authenticate](#method-authenticate)

<a id="api"></a>
<a href="../../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-bearer"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> bearer

**TYPE** · *protocol::http::auth*

**Declaration synopsis:**

```cpp
class bearer { ... };
```

Validates HTTP Bearer credentials through a user-supplied callback.

**Source:** [include/protocol/http/common/auth/bearer.h](../../../../../../include/protocol/http/common/auth/bearer.h)

<a id="method-authenticate"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> authenticate

**METHOD** · *protocol::http::auth::bearer*

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

**Source:** [include/protocol/http/common/auth/bearer.h](../../../../../../include/protocol/http/common/auth/bearer.h)

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> Construction

```cpp
explicit bearer(std::string_view realm,
               std::function<bool(std::string_view)> validator);
```

The realm is copied into the challenge, escaping quotes and backslashes. Only printable ASCII realm bytes are accepted. A missing validator or invalid realm throws `std::invalid_argument`; allocation failures may propagate.

The provider is shared by the router, so validators must support concurrent calls. Callback arguments are borrowed for the call; copy them if they must be retained. Validator exceptions propagate to the caller.

Duplicate `Authorization` fields or a Bearer credential that is not token68 produce `kInvalidRequest` and an `invalid_request` challenge. A missing/wrong scheme produces `kUnauthenticated` with the plain challenge. A false validator result adds `invalid_token`; true produces `kAccepted`. Scheme matching ignores ASCII case; spaces separate scheme and token. The token is passed unchanged, without decoding or JWT validation.
