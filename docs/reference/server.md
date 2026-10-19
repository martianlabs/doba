<a id="server-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/reference/server/index-dark.svg">
    <img src="../../resources/docs/reference/server/index.svg" alt="Index for server.h">
  </picture>
</h1>

**Header:** [include/server.h](../../include/server.h)

Server class that manages the lifecycle of a transport server and its associated protocol engine.

**doba**

- [server](#type-server) · *type*

**server**

- [server&lt;EFy&gt;(configuration, factory)](#constructor-server) · *constructor*
- [server(server)](#constructor-server-2) · *constructor*
- [server(server)](#constructor-server-3) · *constructor*
- [~server](#destructor-destructor-server) · *destructor*
- [operator=(server)](#operator-operator) · *operator*
- [operator=(server)](#operator-operator-2) · *operator*
- [start](#method-start) · *method*
- [stop](#method-stop) · *method*

<a id="type-server"></a>

## server

🟠 **TYPE** · *doba*

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

<a id="constructor-server"></a>

## server

🟠 **CONSTRUCTOR** · *server*

```cpp
template <protocol::contracts::engine_factory<ENty> EFy>
    requires transport::server::contracts::transport<TRty, ENty, EFy>
  explicit server(typename TRty::policies_type configuration, EFy factory);
```

Constructs a server value.

**Template parameters:**

- **EFy:** `protocol::contracts::engine_factory<ENty> EFy`.

**Parameters:**

- **configuration:** Configuration for this component. Declaration: `typename TRty::policies_type configuration`.
- **factory:** Initial value supplied to the constructor. Declaration: `EFy factory`.

**Source:** [include/server.h:60](../../include/server.h#L60)

<a id="constructor-server-2"></a>

## server

🟠 **CONSTRUCTOR** · *server*

```cpp
server(const server&) = delete;
```

Disallows this form of server construction.

**Parameters:**

- **server:** Source object to copy or move. Declaration: `const server&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/server.h:68](../../include/server.h#L68)

<a id="constructor-server-3"></a>

## server

🟠 **CONSTRUCTOR** · *server*

```cpp
server(server&&) noexcept = delete;
```

Disallows this form of server construction.

**Parameters:**

- **server:** Source object to copy or move. Declaration: `server&&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/server.h:69](../../include/server.h#L69)

<a id="destructor-destructor-server"></a>

## ~server

🟠 **DESTRUCTOR** · *server*

```cpp
~server();
```

Destroys the server object.

**Source:** [include/server.h:70](../../include/server.h#L70)

<a id="operator-operator"></a>

## operator=

🟠 **OPERATOR** · *server*

```cpp
server& operator=(const server&) = delete;
```

Disallows this operator= operation.

**Parameters:**

- **server:** Source object to copy or move. Declaration: `const server&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/server.h:74](../../include/server.h#L74)

<a id="operator-operator-2"></a>

## operator=

🟠 **OPERATOR** · *server*

```cpp
server& operator=(server&&) noexcept = delete;
```

Disallows this operator= operation.

**Parameters:**

- **server:** Source object to copy or move. Declaration: `server&&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/server.h:75](../../include/server.h#L75)

<a id="method-start"></a>

## start

🟠 **METHOD** · *server*

```cpp
void start();
```

Starts the transport and begins accepting connections. A second call while running has no effect.

**Returns:** No value (`void`).

**Errors:** Throws `std::runtime_error` if transport construction failed on an earlier attempt. Construction, callbacks, and transport startup can propagate errors.

**Source:** [include/server.h:81](../../include/server.h#L81)

<a id="method-stop"></a>

## stop

🟠 **METHOD** · *server*

```cpp
void stop();
```

Stops accepting connections and closes active connections. A call while stopped has no effect.

**Returns:** No value (`void`).

**Source:** [include/server.h:109](../../include/server.h#L109)
