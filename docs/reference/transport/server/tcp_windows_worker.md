<a id="tcp-windows-worker-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../resources/docs/reference/transport/server/tcp_windows_worker/index-dark.svg">
    <img src="../../../../resources/docs/reference/transport/server/tcp_windows_worker/index.svg" alt="Index for tcp_windows_worker.h">
  </picture>
</h1>

**Header:** [include/transport/server/tcp_windows_worker.h](../../../../include/transport/server/tcp_windows_worker.h)

This struct manages a pool of worker threads that handle TCP connections and data. It sets up a listener, accepts new connections, and creates a context for each one. It also lets you start, stop, or quiesce the workers and set callbacks for when connections are established or disconnected.

**transport::server**

- [worker](#type-worker) · *type*

**transport::server::worker**

- [policies_type](#alias-policies-type) · *alias*
- [worker(configuration, create_engine, shared_state)](#constructor-worker) · *constructor*
- [worker(worker)](#constructor-worker-2) · *constructor*
- [worker(worker)](#constructor-worker-3) · *constructor*
- [~worker](#destructor-destructor-worker) · *destructor*
- [operator=(worker)](#operator-operator) · *operator*
- [operator=(worker)](#operator-operator-2) · *operator*
- [start](#method-start) · *method*
- [stop](#method-stop) · *method*
- [quiesce](#method-quiesce) · *method*
- [set_on_connection](#method-set-on-connection) · *method*
- [set_on_disconnection](#method-set-on-disconnection) · *method*

<a id="type-worker"></a>

## worker

🟠 **TYPE** · *transport::server*

**Declaration synopsis:**

```cpp
template <protocol::contracts::engine ENty,
          protocol::contracts::engine_factory<ENty> FAty, typename CNty,
          typename POty>
struct worker { ... };
```

This struct manages a pool of worker threads that handle TCP connections and data. It sets up a listener, accepts new connections, and creates a context for each one. It also lets you start, stop, or quiesce the workers and set callbacks for when connections are established or disconnected.

**Template parameters:**

- **ENty:** `protocol::contracts::engine ENty`.
- **FAty:** `protocol::contracts::engine_factory<ENty> FAty`.
- **CNty:** `typename CNty`.
- **POty:** `typename POty`.

**Source:** [include/transport/server/tcp_windows_worker.h:76](../../../../include/transport/server/tcp_windows_worker.h#L76)

<a id="alias-policies-type"></a>

## policies_type

🟠 **ALIAS** · *transport::server::worker*

```cpp
using policies_type = POty;
```

Alias for `POty`.

**Source:** [include/transport/server/tcp_windows_worker.h:81](../../../../include/transport/server/tcp_windows_worker.h#L81)

<a id="constructor-worker"></a>

## worker

🟠 **CONSTRUCTOR** · *transport::server::worker*

```cpp
explicit worker(policies_type configuration, FAty create_engine,
                  typename CNty::shared_state shared_state);
```

Constructs a worker value.

**Parameters:**

- **configuration:** Configuration for this component. Declaration: `policies_type configuration`.
- **create_engine:** Initial value supplied to the constructor. Declaration: `FAty create_engine`.
- **shared_state:** Initial value supplied to the constructor. Declaration: `typename CNty::shared_state shared_state`.

**Errors:** Directly throws `std::runtime_error`. Calls made by this operation may propagate additional errors.

**Source:** [include/transport/server/tcp_windows_worker.h:85](../../../../include/transport/server/tcp_windows_worker.h#L85)

<a id="constructor-worker-2"></a>

## worker

🟠 **CONSTRUCTOR** · *transport::server::worker*

```cpp
worker(const worker&) = delete;
```

Disallows this form of transport::server::worker construction.

**Parameters:**

- **worker:** Source object to copy or move. Declaration: `const worker&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/transport/server/tcp_windows_worker.h:105](../../../../include/transport/server/tcp_windows_worker.h#L105)

<a id="constructor-worker-3"></a>

## worker

🟠 **CONSTRUCTOR** · *transport::server::worker*

```cpp
worker(worker&&) noexcept = delete;
```

Disallows this form of transport::server::worker construction.

**Parameters:**

- **worker:** Source object to copy or move. Declaration: `worker&&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/transport/server/tcp_windows_worker.h:106](../../../../include/transport/server/tcp_windows_worker.h#L106)

<a id="destructor-destructor-worker"></a>

## ~worker

🟠 **DESTRUCTOR** · *transport::server::worker*

```cpp
~worker();
```

Destroys the worker object.

**Source:** [include/transport/server/tcp_windows_worker.h:107](../../../../include/transport/server/tcp_windows_worker.h#L107)

<a id="operator-operator"></a>

## operator=

🟠 **OPERATOR** · *transport::server::worker*

```cpp
worker& operator=(const worker&) = delete;
```

Disallows this operator= operation.

**Parameters:**

- **worker:** Source object to copy or move. Declaration: `const worker&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/transport/server/tcp_windows_worker.h:111](../../../../include/transport/server/tcp_windows_worker.h#L111)

<a id="operator-operator-2"></a>

## operator=

🟠 **OPERATOR** · *transport::server::worker*

```cpp
worker& operator=(worker&&) noexcept = delete;
```

Disallows this operator= operation.

**Parameters:**

- **worker:** Source object to copy or move. Declaration: `worker&&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/transport/server/tcp_windows_worker.h:112](../../../../include/transport/server/tcp_windows_worker.h#L112)

<a id="method-start"></a>

## start

🟠 **METHOD** · *transport::server::worker*

```cpp
void start();
```

Starts the workers and sets up the listener. If the worker is already starting, stopping, or running, it returns without doing anything. Otherwise, it parses the IP address and port, sets up the listener, workers, and accept pipeline. If anything goes wrong, it stops the worker and throws the error again.

**Returns:** No value (`void`).

**Errors:** Directly throws `std::runtime_error`. Calls made by this operation may propagate additional errors.

**Source:** [include/transport/server/tcp_windows_worker.h:122](../../../../include/transport/server/tcp_windows_worker.h#L122)

<a id="method-stop"></a>

## stop

🟠 **METHOD** · *transport::server::worker*

```cpp
void stop();
```

Stops the worker. If already stopping or with no active workers, it returns right away. Otherwise, it sets the stopping flag, closes the listener, stops all contexts, and waits for them to finish. It then cleans up and resets the state. If called from a worker thread, it throws an exception.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_windows_worker.h:156](../../../../include/transport/server/tcp_windows_worker.h#L156)

<a id="method-quiesce"></a>

## quiesce

🟠 **METHOD** · *transport::server::worker*

```cpp
void quiesce();
```

Quiesces the worker. If already quiescing or with no workers, it returns right away. Otherwise, it sets the quiescing flag, closes the listener, and calls `quiesce()` on every context. If called from a worker thread, it throws an exception. It doesn’t wait for quiescing to finish; completion occurs during `stop()`.

**Returns:** No value (`void`).

**Errors:** Directly throws `std::runtime_error`. Calls made by this operation may propagate additional errors.

**Source:** [include/transport/server/tcp_windows_worker.h:166](../../../../include/transport/server/tcp_windows_worker.h#L166)

<a id="method-set-on-connection"></a>

## set_on_connection

🟠 **METHOD** · *transport::server::worker*

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

**Source:** [include/transport/server/tcp_windows_worker.h:197](../../../../include/transport/server/tcp_windows_worker.h#L197)

<a id="method-set-on-disconnection"></a>

## set_on_disconnection

🟠 **METHOD** · *transport::server::worker*

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

**Source:** [include/transport/server/tcp_windows_worker.h:206](../../../../include/transport/server/tcp_windows_worker.h#L206)
