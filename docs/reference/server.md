<a id="server-h"></a>
<h1>
  <a href="../../resources/docs/reference/server/index.svg#gh-light-mode-only">
    <img src="../../resources/docs/reference/server/index.svg" alt="Index for server.h">
  </a>
  <a href="../../resources/docs/reference/server/index-dark.svg#gh-dark-mode-only">
    <img src="../../resources/docs/reference/server/index-dark.svg" alt="Index for server.h">
  </a>
</h1>

**Header:** [include/server.h](../../include/server.h)

Server class that manages the lifecycle of a transport server and its associated protocol engine.

**doba**

- [server](#type-server) · *type*

**server**

- [start](#method-start) · *method*
- [stop](#method-stop) · *method*

<a id="type-server"></a>

## <a href="../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> server

**TYPE** · *doba*

**Declaration synopsis:**

```cpp
template <protocol::contracts::engine ENty, typename TRty>
class server { ... };
```

Server class that manages the lifecycle of a transport server and its associated protocol engine.

**Template parameters:**

- **ENty:** `protocol::contracts::engine ENty`.
- **TRty:** `typename TRty`.

**Source:** [include/server.h:53](../../include/server.h#L53)

<a id="method-start"></a>

## <a href="../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> start

**METHOD** · *server*

```cpp
void start();
```

Starts the transport and begins accepting connections. A second call while running has no effect.

**Returns:** No value (`void`).

**Errors:** Throws `std::runtime_error` if transport construction failed on an earlier attempt. Construction, callbacks, and transport startup can propagate errors.

**Source:** [include/server.h:81](../../include/server.h#L81)

<a id="method-stop"></a>

## <a href="../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> stop

**METHOD** · *server*

```cpp
void stop();
```

Stops accepting connections and closes active connections. A call while stopped has no effect.

**Returns:** No value (`void`).

**Source:** [include/server.h:109](../../include/server.h#L109)
