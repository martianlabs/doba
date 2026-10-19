<a id="tcp"></a>
<a href="../../../../resources/docs/reference/transport/server/tcp/index.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp/index.svg" alt="tcp">
</a>
<a href="../../../../resources/docs/reference/transport/server/tcp/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp/index-dark.svg" alt="tcp">
</a>

**Header:** [include/transport/server/tcp.h](../../../../include/transport/server/tcp.h)

Platform-independent transport delegate types.

### <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transport::server

[types](#type-types) ·
[tcp](#type-tcp)

### <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transport::server::types

[on_client_connected_delegate](#alias-on-client-connected-delegate) ·
[on_client_disconnected_delegate](#alias-on-client-disconnected-delegate)

<a id="api-details"></a>
<a href="../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-types"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> types

**TYPE** · *transport::server*

**Declaration synopsis:**

```cpp
struct types { ... };
```

Platform-independent transport delegate types.

**Source:** [include/transport/server/tcp.h:44](../../../../include/transport/server/tcp.h#L44)

<a id="alias-on-client-connected-delegate"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> on_client_connected_delegate

**ALIAS** · *transport::server::types*

```cpp
using on_client_connected_delegate = std::function<void()>;
```

On client connected delegate type

**Source:** [include/transport/server/tcp.h:46](../../../../include/transport/server/tcp.h#L46)

<a id="alias-on-client-disconnected-delegate"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> on_client_disconnected_delegate

**ALIAS** · *transport::server::types*

```cpp
using on_client_disconnected_delegate = std::function<void()>;
```

On client disconnected delegate type

**Source:** [include/transport/server/tcp.h:48](../../../../include/transport/server/tcp.h#L48)

<a id="type-tcp"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> tcp

**TYPE** · *transport::server*

**Declaration synopsis:**

```cpp
template <protocol::contracts::engine ENty,
          protocol::contracts::engine_factory<ENty> FAty>
class tcp
    : public basic_transport<ENty, FAty, tcp_connection<ENty>, tcp_policies> { ... };
```

This class provides a TCP transport for an engine. It inherits from `basic_transport` and takes a `tcp_policies` configuration and an engine factory when it is created.

**Template parameters:**

- **ENty:** `protocol::contracts::engine ENty`.
- **FAty:** `protocol::contracts::engine_factory<ENty> FAty`.

**Source:** [include/transport/server/tcp.h:79](../../../../include/transport/server/tcp.h#L79)
