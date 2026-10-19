<a id="bearer-h"></a>
<h1>
  <a href="../../../../../../resources/docs/reference/protocol/http/common/auth/bearer/index.svg#gh-light-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/auth/bearer/index.svg" alt="Index for bearer.h">
  </a>
  <a href="../../../../../../resources/docs/reference/protocol/http/common/auth/bearer/index-dark.svg#gh-dark-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/auth/bearer/index-dark.svg" alt="Index for bearer.h">
  </a>
</h1>

**Header:** [include/protocol/http/common/auth/bearer.h](../../../../../../include/protocol/http/common/auth/bearer.h)

Validates HTTP Bearer credentials through a user-supplied callback.

**protocol::http::auth**

- [bearer](#type-bearer) · *type*

**protocol::http::auth::bearer**

- [authenticate](#method-authenticate) · *method*

<a id="type-bearer"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> bearer

**TYPE** · *protocol::http::auth*

**Declaration synopsis:**

```cpp
class bearer { ... };
```

Validates HTTP Bearer credentials through a user-supplied callback.

**Source:** [include/protocol/http/common/auth/bearer.h:43](../../../../../../include/protocol/http/common/auth/bearer.h#L43)

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

**Source:** [include/protocol/http/common/auth/bearer.h:66](../../../../../../include/protocol/http/common/auth/bearer.h#L66)
