<a id="authentication-result"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/auth/authentication_result/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/auth/authentication_result/title.svg" alt="authentication_result">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/auth/authentication_result/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/auth/authentication_result/title-dark.svg" alt="authentication_result">
</a>

**Header:** [include/protocol/http/common/auth/authentication_result.h](../../../../../../include/protocol/http/common/auth/authentication_result.h)

Provides the authentication result protocol component.

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::auth

[authentication_status](#enum-authentication-status) ·
[authentication_result](#type-authentication-result)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::auth::authentication_result

[status](#field-status) ·
[challenge](#field-challenge)

<a id="api"></a>
<a href="../../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="enum-authentication-status"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> authentication_status

**ENUM** · *protocol::http::auth*

```cpp
enum class authentication_status {
  kAccepted,
  kInvalidRequest,
  kUnauthenticated,
  kForbidden
};
```

Enumerates authentication status values.

**Values:**

- **kAccepted:** The credential was accepted. Declaration: `kAccepted`.
- **kInvalidRequest:** The request's authentication syntax is invalid. Declaration: `kInvalidRequest`.
- **kUnauthenticated:** Authentication is required or the credential was rejected. Declaration: `kUnauthenticated`.
- **kForbidden:** The authenticated caller is not permitted to access the resource. Declaration: `kForbidden`.

**Source:** [include/protocol/http/common/auth/authentication_result.h:36](../../../../../../include/protocol/http/common/auth/authentication_result.h#L36)

<a id="type-authentication-result"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> authentication_result

**TYPE** · *protocol::http::auth*

**Declaration synopsis:**

```cpp
struct authentication_result { ... };
```

Provides the authentication result protocol component.

**Source:** [include/protocol/http/common/auth/authentication_result.h:48](../../../../../../include/protocol/http/common/auth/authentication_result.h#L48)

<a id="field-status"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> status

**FIELD** · *protocol::http::auth::authentication_result*

```cpp
authentication_status status{authentication_status::kUnauthenticated};
```

Stores the status value.

**Value:** `{authentication_status::kUnauthenticated}`

**Source:** [include/protocol/http/common/auth/authentication_result.h:52](../../../../../../include/protocol/http/common/auth/authentication_result.h#L52)

<a id="field-challenge"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> challenge

**FIELD** · *protocol::http::auth::authentication_result*

```cpp
std::string challenge;
```

Stores the challenge value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/common/auth/authentication_result.h:53](../../../../../../include/protocol/http/common/auth/authentication_result.h#L53)
