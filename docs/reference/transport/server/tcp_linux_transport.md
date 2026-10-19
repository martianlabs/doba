<a id="tcp-linux-transport-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../resources/docs/reference/transport/server/tcp_linux_transport/index-dark.svg">
    <img src="../../../../resources/docs/reference/transport/server/tcp_linux_transport/index.svg" alt="Index for tcp_linux_transport.h">
  </picture>
</h1>

**Header:** [include/transport/server/tcp_linux_transport.h](../../../../include/transport/server/tcp_linux_transport.h)

This class provides a basic TCP transport for Linux. It manages the worker threads that handle incoming and outgoing connections, and lets you set callbacks for connection and disconnection events. Before starting the transport, it also sets up the buffer sizes and listen backlog.

**transport::server**

- [basic_transport](#type-basic-transport) · *type*

**transport::server::basic_transport**

- [policies_type](#alias-policies-type) · *alias*
- [basic_transport(configuration, create_engine, shared_state)](#constructor-basic-transport) · *constructor*
- [basic_transport(basic_transport)](#constructor-basic-transport-2) · *constructor*
- [basic_transport(basic_transport)](#constructor-basic-transport-3) · *constructor*
- [~basic_transport](#destructor-destructor-basic-transport) · *destructor*
- [operator=(basic_transport)](#operator-operator) · *operator*
- [operator=(basic_transport)](#operator-operator-2) · *operator*
- [start](#method-start) · *method*
- [stop](#method-stop) · *method*
- [quiesce](#method-quiesce) · *method*
- [set_on_connection](#method-set-on-connection) · *method*
- [set_on_disconnection](#method-set-on-disconnection) · *method*

<a id="type-basic-transport"></a>

## basic_transport

🟠 **TYPE** · *transport::server*

**Declaration synopsis:**

```cpp
template <protocol::contracts::engine ENty,
          protocol::contracts::engine_factory<ENty> FAty, typename CNty,
          typename POty>
class basic_transport { ... };
```

This class provides a basic TCP transport for Linux. It manages the worker threads that handle incoming and outgoing connections, and lets you set callbacks for connection and disconnection events. Before starting the transport, it also sets up the buffer sizes and listen backlog.

**Template parameters:**

- **ENty:** `protocol::contracts::engine ENty`.
- **FAty:** `protocol::contracts::engine_factory<ENty> FAty`.
- **CNty:** `typename CNty`.
- **POty:** `typename POty`.

**Source:** [include/transport/server/tcp_linux_transport.h:68](../../../../include/transport/server/tcp_linux_transport.h#L68)

<a id="alias-policies-type"></a>

## policies_type

🟠 **ALIAS** · *transport::server::basic_transport*

```cpp
using policies_type = POty;
```

Alias for `POty`.

**Source:** [include/transport/server/tcp_linux_transport.h:73](../../../../include/transport/server/tcp_linux_transport.h#L73)

<a id="constructor-basic-transport"></a>

## basic_transport

🟠 **CONSTRUCTOR** · *transport::server::basic_transport*

```cpp
explicit basic_transport(policies_type configuration, FAty create_engine,
                           typename CNty::shared_state shared_state);
```

Constructs a basic_transport value.

**Parameters:**

- **configuration:** Configuration for this component. Declaration: `policies_type configuration`.
- **create_engine:** Initial value supplied to the constructor. Declaration: `FAty create_engine`.
- **shared_state:** Initial value supplied to the constructor. Declaration: `typename CNty::shared_state shared_state`.

**Errors:** Directly throws `std::runtime_error`. Calls made by this operation may propagate additional errors.

**Source:** [include/transport/server/tcp_linux_transport.h:77](../../../../include/transport/server/tcp_linux_transport.h#L77)

<a id="constructor-basic-transport-2"></a>

## basic_transport

🟠 **CONSTRUCTOR** · *transport::server::basic_transport*

```cpp
basic_transport(const basic_transport&) = delete;
```

Disallows this form of transport::server::basic_transport construction.

**Parameters:**

- **basic_transport:** Source object to copy or move. Declaration: `const basic_transport&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/transport/server/tcp_linux_transport.h:94](../../../../include/transport/server/tcp_linux_transport.h#L94)

<a id="constructor-basic-transport-3"></a>

## basic_transport

🟠 **CONSTRUCTOR** · *transport::server::basic_transport*

```cpp
basic_transport(basic_transport&&) noexcept = delete;
```

Disallows this form of transport::server::basic_transport construction.

**Parameters:**

- **basic_transport:** Source object to copy or move. Declaration: `basic_transport&&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/transport/server/tcp_linux_transport.h:95](../../../../include/transport/server/tcp_linux_transport.h#L95)

<a id="destructor-destructor-basic-transport"></a>

## ~basic_transport

🟠 **DESTRUCTOR** · *transport::server::basic_transport*

```cpp
~basic_transport();
```

Destroys the basic_transport object.

**Source:** [include/transport/server/tcp_linux_transport.h:96](../../../../include/transport/server/tcp_linux_transport.h#L96)

<a id="operator-operator"></a>

## operator=

🟠 **OPERATOR** · *transport::server::basic_transport*

```cpp
basic_transport& operator=(const basic_transport&) = delete;
```

Disallows this operator= operation.

**Parameters:**

- **basic_transport:** Source object to copy or move. Declaration: `const basic_transport&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/transport/server/tcp_linux_transport.h:100](../../../../include/transport/server/tcp_linux_transport.h#L100)

<a id="operator-operator-2"></a>

## operator=

🟠 **OPERATOR** · *transport::server::basic_transport*

```cpp
basic_transport& operator=(basic_transport&&) noexcept = delete;
```

Disallows this operator= operation.

**Parameters:**

- **basic_transport:** Source object to copy or move. Declaration: `basic_transport&&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/transport/server/tcp_linux_transport.h:101](../../../../include/transport/server/tcp_linux_transport.h#L101)

<a id="method-start"></a>

## start

🟠 **METHOD** · *transport::server::basic_transport*

```cpp
void start();
```

Starts the transport, sets up the listening socket, and starts the worker threads. It also checks that the IP address and port are valid. If it’s already starting or stopping, or has active workers, it returns without doing anything. Once everything is up and running, it lets any waiting threads know that startup is complete. If setup fails, it

**Returns:** No value (`void`).

**Errors:** Directly throws `std::runtime_error`. Calls made by this operation may propagate additional errors.

**Source:** [include/transport/server/tcp_linux_transport.h:112](../../../../include/transport/server/tcp_linux_transport.h#L112)

<a id="method-stop"></a>

## stop

🟠 **METHOD** · *transport::server::basic_transport*

```cpp
void stop();
```

Stops the basic_transport lifecycle.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_linux_transport.h:153](../../../../include/transport/server/tcp_linux_transport.h#L153)

<a id="method-quiesce"></a>

## quiesce

🟠 **METHOD** · *transport::server::basic_transport*

```cpp
void quiesce();
```

Asks all workers to quiesce: they stop accepting new connections and finish handling the ones they already have. The method waits until all workers are done. If called from a worker thread, it throws an exception to avoid a deadlock.

**Returns:** No value (`void`).

**Errors:** Directly throws `std::runtime_error`. Calls made by this operation may propagate additional errors.

**Source:** [include/transport/server/tcp_linux_transport.h:162](../../../../include/transport/server/tcp_linux_transport.h#L162)

<a id="method-set-on-connection"></a>

## set_on_connection

🟠 **METHOD** · *transport::server::basic_transport*

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

**Source:** [include/transport/server/tcp_linux_transport.h:183](../../../../include/transport/server/tcp_linux_transport.h#L183)

<a id="method-set-on-disconnection"></a>

## set_on_disconnection

🟠 **METHOD** · *transport::server::basic_transport*

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

**Source:** [include/transport/server/tcp_linux_transport.h:192](../../../../include/transport/server/tcp_linux_transport.h#L192)
