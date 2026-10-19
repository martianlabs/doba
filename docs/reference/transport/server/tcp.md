<a id="tcp-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../resources/docs/reference/transport/server/tcp/index-dark.svg">
    <img src="../../../../resources/docs/reference/transport/server/tcp/index.svg" alt="Index for tcp.h">
  </picture>
</h1>

**Header:** [include/transport/server/tcp.h](../../../../include/transport/server/tcp.h)

Platform-independent transport delegate types.

**transport::server**

- [types](#type-types) · *type*

**transport::server::types**

- [on_client_connected_delegate](#alias-on-client-connected-delegate) · *alias*
- [on_client_disconnected_delegate](#alias-on-client-disconnected-delegate) · *alias*

**transport::server**

- [tcp](#type-tcp) · *type*

**transport::server::tcp**

- [tcp](#constructor-tcp) · *constructor*

<a id="type-types"></a>

## types

🟠 **TYPE** · *transport::server*

**Declaration synopsis:**

```cpp
struct types { ... };
```

Platform-independent transport delegate types.

**Source:** [include/transport/server/tcp.h:44](../../../../include/transport/server/tcp.h#L44)

<a id="alias-on-client-connected-delegate"></a>

## on_client_connected_delegate

🟠 **ALIAS** · *transport::server::types*

```cpp
using on_client_connected_delegate = std::function<void()>;
```

On client connected delegate type

**Source:** [include/transport/server/tcp.h:46](../../../../include/transport/server/tcp.h#L46)

<a id="alias-on-client-disconnected-delegate"></a>

## on_client_disconnected_delegate

🟠 **ALIAS** · *transport::server::types*

```cpp
using on_client_disconnected_delegate = std::function<void()>;
```

On client disconnected delegate type

**Source:** [include/transport/server/tcp.h:48](../../../../include/transport/server/tcp.h#L48)

<a id="type-tcp"></a>

## tcp

🟠 **TYPE** · *transport::server*

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

<a id="constructor-tcp"></a>

## tcp

🟠 **CONSTRUCTOR** · *transport::server::tcp*

```cpp
explicit tcp(tcp_policies configuration, FAty create_engine);
```

Constructs a tcp value.

**Parameters:**

- **configuration:** Configuration for this component. Declaration: `tcp_policies configuration`.
- **create_engine:** Initial value supplied to the constructor. Declaration: `FAty create_engine`.

**Source:** [include/transport/server/tcp.h:85](../../../../include/transport/server/tcp.h#L85)
