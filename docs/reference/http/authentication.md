<a name="authentication"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/h1-authentication-dark.svg">
    <img src="../../../resources/docs/reference/h1-authentication.svg" alt="Authentication">
  </picture>
</h1>

Pick an authentication provider for each endpoint. The
[Basic example](../../../examples/http/v11/basic_auth/README.md) and
[Bearer example](../../../examples/http/v11/bearer_auth/README.md) show complete
servers, including public routes and client commands.

All types below live in **martianlabs::doba::protocol::http::auth**.
The server owns the providers; routes select one by type. Registrations
without a provider remain public. See [HTTP server](server.md#authentication)
for construction and registration.

Checks run synchronously before every handler. Handlers return
*common::task<void>* and may complete immediately or suspend.
The engine builds rejection responses and keeps them in request order.
Authentication runs after receiving a complete request: it does not reject
a body before reading it or suppress **100 Continue**.

Use TLS to protect credentials in deployments. The
[HTTPS example](../../../examples/http/v11/https_hello_world/README.md) shows
certificate setup. Basic is Base64 encoding, not encryption; Bearer tokens
can be used by anyone who obtains them.

<a name="authentication-result"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/h2-authentication-result-dark.svg">
    <img src="../../../resources/docs/reference/h2-authentication-result.svg" alt="Authentication result">
  </picture>
</h2>

Include **protocol/http/common/auth/authentication_result.h**.

```cpp
enum class authentication_status {
  kAccepted,
  kInvalidRequest,
  kUnauthenticated,
  kForbidden
};

struct authentication_result {
  authentication_status status{authentication_status::kUnauthenticated};
  std::string challenge;
};
```

**Parameters:** Set **status** and, when needed, **challenge**. The challenge
owns its text and contains the complete **WWW-Authenticate** value without
the field name. It must follow the field's grammar.

**Returns:** **kAccepted** allows the handler to run. Other states skip it
and produce **400**, **401**, or **403**, respectively. Nonempty challenges
are included in rejections; an accepted result's challenge is ignored.

**Errors:** A **401** without a challenge, invalid rejection challenge, or
invalid status is a provider contract error. The engine returns **500**
and closes the connection. A default result needs a challenge before it
can be used as a rejection.

A **401** challenge is required by
[RFC 9110 S15.5.2](https://www.rfc-editor.org/rfc/rfc9110.html#section-15.5.2).
The result contains no response object or identity storage.

<a name="basic"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/h2-basic-dark.svg">
    <img src="../../../resources/docs/reference/h2-basic.svg" alt="Basic">
  </picture>
</h2>

Include **protocol/http/common/auth/basic.h**. See the
[Basic example](../../../examples/http/v11/basic_auth/README.md) and its
[source](../../../examples/http/v11/basic_auth/main.cpp).

<a name="basic-construction"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/h3-construction-dark.svg">
    <img src="../../../resources/docs/reference/h3-construction.svg" alt="Construction">
  </picture>
</h3>

```cpp
explicit basic(
    std::string_view realm,
    std::function<bool(std::string_view, std::string_view)> validator);
```

**Parameters:** **realm** identifies the protection space. It may be empty
and must contain printable ASCII. Quotes and backslashes are escaped in
the challenge. The provider owns **validator**; captured dependencies must
outlive all server requests. Credential views are valid only during the
validator call.

Basic passes user-id and password to the validator.

**Returns:** No value; constructs a provider that can be moved into the
server's provider tuple.

**Errors:** Throws **std::invalid_argument** for an empty callable or an
invalid realm.

<a name="basic-authenticate"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/h3-authenticate-dark.svg">
    <img src="../../../resources/docs/reference/h3-authenticate.svg" alt="Authenticate">
  </picture>
</h3>

```cpp
template <typename RQty>
authentication_result authenticate(const RQty& request) const;
```

**Parameters:** **request** supplies indexed headers through
**get_headers_length** and **get_header**. Field names and scheme names
are matched without case sensitivity. The validator returns a bool.

**Returns:** **kAccepted** when the validator accepts the decoded pair.
Missing credentials, another scheme, invalid Basic credentials, or a false
validator result produce **kUnauthenticated** with a Basic realm challenge.
Duplicate Authorization fields produce **kInvalidRequest** before validation.

The first colon separates user-id and password; later colons belong to the
password. Empty user-id and password are passed to the validator. Control
characters and invalid Base64, including nonzero padding bits, are rejected.
Credential octets are preserved without character conversion; the provider
does not advertise **charset**. These rules follow
[RFC 7617 S2](https://www.rfc-editor.org/rfc/rfc7617.html#section-2).

**Errors:** Validator exceptions propagate to direct callers. During
server processing, the engine uses its existing error path: **500** and
connection closure. Earlier decoder errors retain their current behavior.

<a name="bearer"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/h2-bearer-dark.svg">
    <img src="../../../resources/docs/reference/h2-bearer.svg" alt="Bearer">
  </picture>
</h2>

Include **protocol/http/common/auth/bearer.h**. See the
[Bearer example](../../../examples/http/v11/bearer_auth/README.md) and its
[source](../../../examples/http/v11/bearer_auth/main.cpp).

<a name="bearer-construction"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/h3-construction-dark.svg">
    <img src="../../../resources/docs/reference/h3-construction.svg" alt="Construction">
  </picture>
</h3>

```cpp
explicit bearer(std::string_view realm,
                std::function<bool(std::string_view)> validator);
```

**Parameters:** **realm** identifies the protection space. It may be empty
and must contain printable ASCII. Quotes and backslashes are escaped in
the challenge. The provider owns **validator**; captured dependencies must
outlive all server requests. Credential views are valid only during the
validator call.

Bearer passes the token unchanged. It does not verify JWT or issue tokens.

**Returns:** No value; constructs a provider that can be moved into the
server's provider tuple.

**Errors:** Throws **std::invalid_argument** for an empty callable or an
invalid realm.

<a name="bearer-authenticate"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/h3-authenticate-dark.svg">
    <img src="../../../resources/docs/reference/h3-authenticate.svg" alt="Authenticate">
  </picture>
</h3>

```cpp
template <typename RQty>
authentication_result authenticate(const RQty& request) const;
```

**Parameters:** **request** supplies indexed headers through
**get_headers_length** and **get_header**. Field names and scheme names
are matched without case sensitivity. The validator returns a bool.

**Returns:** **kAccepted** when the validator accepts the token. Missing
Bearer credentials produce **401** with a Bearer realm challenge and no
error attribute. A rejected token produces **401** with
**error="invalid_token"**. Malformed Bearer credentials or duplicate
Authorization fields produce **400** with **error="invalid_request"**.
The validator is not called for malformed credentials.

Tokens are read only from **Authorization**, following
[RFC 6750 S2.1](https://www.rfc-editor.org/rfc/rfc6750.html#section-2.1).
Query and body fields are not token sources. Errors follow
[RFC 6750 S3](https://www.rfc-editor.org/rfc/rfc6750.html#section-3).
There is no built-in scope or permissions check.

**Errors:** Validator exceptions propagate to direct callers. During
server processing, the engine uses its existing error path: **500** and
connection closure. Earlier decoder errors retain their current behavior.

<a name="custom-providers"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/h2-custom-providers-dark.svg">
    <img src="../../../resources/docs/reference/h2-custom-providers.svg" alt="Custom providers">
  </picture>
</h2>

Include **protocol/http/common/auth/contracts.h**. The
**auth::contracts::provider<Aty, RQty>** concept checks this method:

```cpp
auth::authentication_result authenticate(const request& req) const;
```

**Parameters:** **req** is the server's request type. The provider extracts
its own credentials and receives application dependencies at construction.

**Returns:** An owning **authentication_result**. The provider chooses the
rejection state and challenge; the engine writes the HTTP response.

**Errors:** Incorrect signatures fail compilation. Exceptions or invalid
results produce **500** and closure during server processing.

An application-specific header can use the same contract:

```cpp
struct company_auth {
  std::function<bool(std::string_view)> validator;

  auth::authentication_result authenticate(const request& req) const {
    if (req.exist_header("X-Company-Key") &&
        validator(req.get_header("X-Company-Key").second)) {
      return {auth::authentication_status::kAccepted, {}};
    }
    return {auth::authentication_status::kUnauthenticated,
            "Company realm=\"internal\""};
  }
};
```

List **company_auth** in the server's provider template arguments, pass its
instance in the construction tuple, and use **add_route<company_auth>**.
One instance of each provider type is shared across connections. Providers
and validators must support concurrent calls; const alone does not make
captured mutable state safe. Do not retain request views.

Registration remains forbidden while the server runs. Authentication is
selected through **add_route**; controller registrations remain public.
This feature adds no identity object, asynchronous validator, or policy
combining multiple providers.
