<a id="basic"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/auth/basic/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/auth/basic/index.svg" alt="basic">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/auth/basic/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/auth/basic/index-dark.svg" alt="basic">
</a>

**Header:** [include/protocol/http/common/auth/basic.h](../../../../../../include/protocol/http/common/auth/basic.h)

Validates HTTP Basic credentials through a user-supplied callback.

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::auth

[basic](#type-basic)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::auth::basic

[authenticate](#method-authenticate)

<a id="type-basic"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> basic

**TYPE** · *protocol::http::auth*

**Declaration synopsis:**

```cpp
class basic { ... };
```

Validates HTTP Basic credentials through a user-supplied callback.

**Source:** [include/protocol/http/common/auth/basic.h:43](../../../../../../include/protocol/http/common/auth/basic.h#L43)

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

**Source:** [include/protocol/http/common/auth/basic.h:67](../../../../../../include/protocol/http/common/auth/basic.h#L67)
