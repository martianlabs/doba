<a id="tcp-windows-transport-h"></a>
<h1>
  <a href="../../../../resources/docs/reference/transport/server/tcp_windows_transport/index.svg#gh-light-mode-only">
    <img src="../../../../resources/docs/reference/transport/server/tcp_windows_transport/index.svg" alt="Index for tcp_windows_transport.h">
  </a>
  <a href="../../../../resources/docs/reference/transport/server/tcp_windows_transport/index-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/docs/reference/transport/server/tcp_windows_transport/index-dark.svg" alt="Index for tcp_windows_transport.h">
  </a>
</h1>

**Header:** [include/transport/server/tcp_windows_transport.h](../../../../include/transport/server/tcp_windows_transport.h)

This class provides a basic transport for an engine. It’s a template that takes the engine, engine factory, connection, and policies types as parameters. It manages the worker that handles the transport, including starting, stopping, and quiescing it. You can also set callbacks for connection and disconnection events.

**transport::server**

- [basic_transport](#type-basic-transport) · *type*

**transport::server::basic_transport**

- [policies_type](#alias-policies-type) · *alias*
- [start](#method-start) · *method*
- [stop](#method-stop) · *method*
- [quiesce](#method-quiesce) · *method*
- [set_on_connection](#method-set-on-connection) · *method*
- [set_on_disconnection](#method-set-on-disconnection) · *method*

<a id="type-basic-transport"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> basic_transport

**TYPE** · *transport::server*

**Declaration synopsis:**

```cpp
template <protocol::contracts::engine ENty,
          protocol::contracts::engine_factory<ENty> FAty, typename CNty,
          typename POty>
class basic_transport { ... };
```

This class provides a basic transport for an engine. It’s a template that takes the engine, engine factory, connection, and policies types as parameters. It manages the worker that handles the transport, including starting, stopping, and quiescing it. You can also set callbacks for connection and disconnection events.

**Template parameters:**

- **ENty:** `protocol::contracts::engine ENty`.
- **FAty:** `protocol::contracts::engine_factory<ENty> FAty`.
- **CNty:** `typename CNty`.
- **POty:** `typename POty`.

**Source:** [include/transport/server/tcp_windows_transport.h:53](../../../../include/transport/server/tcp_windows_transport.h#L53)

<a id="alias-policies-type"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> policies_type

**ALIAS** · *transport::server::basic_transport*

```cpp
using policies_type = POty;
```

Alias for `POty`.

**Source:** [include/transport/server/tcp_windows_transport.h:58](../../../../include/transport/server/tcp_windows_transport.h#L58)

<a id="method-start"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> start

**METHOD** · *transport::server::basic_transport*

```cpp
void start();
```

Starts the basic_transport lifecycle.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_windows_transport.h:77](../../../../include/transport/server/tcp_windows_transport.h#L77)

<a id="method-stop"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> stop

**METHOD** · *transport::server::basic_transport*

```cpp
void stop();
```

Stops the basic_transport lifecycle.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_windows_transport.h:81](../../../../include/transport/server/tcp_windows_transport.h#L81)

<a id="method-quiesce"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> quiesce

**METHOD** · *transport::server::basic_transport*

```cpp
void quiesce();
```

Stops accepting new connections before shutdown.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_windows_transport.h:85](../../../../include/transport/server/tcp_windows_transport.h#L85)

<a id="method-set-on-connection"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> set_on_connection

**METHOD** · *transport::server::basic_transport*

```cpp
template <typename FNty>
  void set_on_connection(FNty&& fn);
```

Sets on connection.

**Template parameters:**

- **FNty:** `typename FNty`.

**Parameters:**

- **fn:** Callable to invoke. Declaration: `FNty&& fn`.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_windows_transport.h:90](../../../../include/transport/server/tcp_windows_transport.h#L90)

<a id="method-set-on-disconnection"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> set_on_disconnection

**METHOD** · *transport::server::basic_transport*

```cpp
template <typename FNty>
  void set_on_disconnection(FNty&& fn);
```

Sets on disconnection.

**Template parameters:**

- **FNty:** `typename FNty`.

**Parameters:**

- **fn:** Callable to invoke. Declaration: `FNty&& fn`.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_windows_transport.h:97](../../../../include/transport/server/tcp_windows_transport.h#L97)
