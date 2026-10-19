These are layout samples. In the finished reference, each entry belongs to
the page that mirrors its own header under *include/*.

<a name="add-route"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/reference/heading-options/option-a-method-dark.svg">
    <img src="../../resources/docs/reference/heading-options/option-a-method.svg" alt="Method: add_route">
  </picture>
</h2>

**Header:** [include/http_server.h](../../include/http_server.h)

```cpp
template <typename Hty>
  requires protocol::http::router_handler_lambda<Hty>
http_server& add_route(std::string_view method, std::string_view route,
                       Hty handler);
```

Registers a route in the server's router. The handler is moved into the
router. Registration is unavailable while the server is running.

**Parameters:**

- **Hty:** Handler type, deduced from *handler*.
- **method:** HTTP method to match, such as *GET*.
- **route:** Path pattern to match.
- **handler:** Callable to run when the route matches.

**Returns:** A reference to the same *http_server*, so calls can be chained.

**Errors:** Throws *std::runtime_error* when registration is closed. The
router can throw *std::invalid_argument* for an invalid wildcard or a
mismatch between route parameters and handler arguments.

<a name="default-receive-buffer-capacity"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/reference/heading-options/option-a-constant-dark.svg">
    <img src="../../resources/docs/reference/heading-options/option-a-constant.svg" alt="Constant: kDefaultReceiveBufferCapacity">
  </picture>
</h2>

**Header:** [include/transport/server/tcp_policies.h](../../include/transport/server/tcp_policies.h)

```cpp
static constexpr std::size_t kDefaultReceiveBufferCapacity = 4 * 1024;
```

**Value:** 4096 bytes.

**Purpose:** Initializes *tcp_policies::receive_buffer_capacity*. This is
the receive buffer capacity per connection, not an HTTP body size limit.

<a name="authentication-status"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/reference/heading-options/option-a-enum-dark.svg">
    <img src="../../resources/docs/reference/heading-options/option-a-enum.svg" alt="Enum: authentication_status">
  </picture>
</h2>

**Header:** [include/protocol/http/common/auth/authentication_result.h](../../include/protocol/http/common/auth/authentication_result.h)

```cpp
enum class authentication_status {
  kAccepted,
  kInvalidRequest,
  kUnauthenticated,
  kForbidden
};
```

Selects the outcome of an authentication provider. The HTTP engine uses it
to decide whether to run the handler or send a rejection.

**Values:**

- **kAccepted:** Run the handler.
- **kInvalidRequest:** Reject with HTTP 400.
- **kUnauthenticated:** Reject with HTTP 401 and a valid challenge.
- **kForbidden:** Reject with HTTP 403.

**Default:** *authentication_result::status* starts as *kUnauthenticated*.
