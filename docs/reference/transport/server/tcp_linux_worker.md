<a id="tcp-linux-worker-h"></a>
<h1>
  <a href="../../../../resources/docs/reference/transport/server/tcp_linux_worker/index.svg#gh-light-mode-only">
    <img src="../../../../resources/docs/reference/transport/server/tcp_linux_worker/index.svg" alt="Index for tcp_linux_worker.h">
  </a>
  <a href="../../../../resources/docs/reference/transport/server/tcp_linux_worker/index-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/docs/reference/transport/server/tcp_linux_worker/index-dark.svg" alt="Index for tcp_linux_worker.h">
  </a>
</h1>

**Header:** [include/transport/server/tcp_linux_worker.h](../../../../include/transport/server/tcp_linux_worker.h)

This struct represents a worker thread that handles TCP connections for an engine. It manages the `epoll` instance, listener socket, and event file descriptors. It also sets up, starts, and stops the worker, runs its thread, checks whether the current thread is that worker, and handles connection events.

**transport::server**

- [kStopEventId](#constant-kstopeventid) · *constant*
- [kWorkEventId](#constant-kworkeventid) · *constant*
- [kLstnrEventId](#constant-klstnreventid) · *constant*
- [worker](#type-worker) · *type*

**transport::server::worker**

- [setup](#method-setup) · *method*
- [start](#method-start) · *method*
- [is_current_thread()](#method-is-current-thread) · *method*
- [is_current_thread(cr_engine)](#method-is-current-thread-2) · *method*
- [stop](#method-stop) · *method*
- [request_stop](#method-request-stop) · *method*
- [request_quiesce](#method-request-quiesce) · *method*
- [wait_quiesced](#method-wait-quiesced) · *method*

<a id="constant-kstopeventid"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kStopEventId

**CONSTANT** · *transport::server*

```cpp
static constexpr uint64_t kStopEventId = 0;
```

Defines the stop event id constant.

**Value:** `0`

**Source:** [include/transport/server/tcp_linux_worker.h:56](../../../../include/transport/server/tcp_linux_worker.h#L56)

<a id="constant-kworkeventid"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kWorkEventId

**CONSTANT** · *transport::server*

```cpp
static constexpr uint64_t kWorkEventId = 1;
```

Defines the work event id constant.

**Value:** `1`

**Source:** [include/transport/server/tcp_linux_worker.h:57](../../../../include/transport/server/tcp_linux_worker.h#L57)

<a id="constant-klstnreventid"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kLstnrEventId

**CONSTANT** · *transport::server*

```cpp
static constexpr uint64_t kLstnrEventId = std::numeric_limits<uint64_t>::max();
```

Defines the lstnr event id constant.

**Value:** `std::numeric_limits<uint64_t>::max()`

**Source:** [include/transport/server/tcp_linux_worker.h:58](../../../../include/transport/server/tcp_linux_worker.h#L58)

<a id="type-worker"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> worker

**TYPE** · *transport::server*

**Declaration synopsis:**

```cpp
template <protocol::contracts::engine ENty,
          protocol::contracts::engine_factory<ENty> FAty, typename CNty,
          typename POty>
struct worker { ... };
```

This struct represents a worker thread that handles TCP connections for an engine. It manages the `epoll` instance, listener socket, and event file descriptors. It also sets up, starts, and stops the worker, runs its thread, checks whether the current thread is that worker, and handles connection events.

**Template parameters:**

- **ENty:** `protocol::contracts::engine ENty`.
- **FAty:** `protocol::contracts::engine_factory<ENty> FAty`.
- **CNty:** `typename CNty`.
- **POty:** `typename POty`.

**Source:** [include/transport/server/tcp_linux_worker.h:80](../../../../include/transport/server/tcp_linux_worker.h#L80)

<a id="method-setup"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> setup

**METHOD** · *transport::server::worker*

```cpp
void setup(in_addr ip, uint16_t port);
```

Sets up the worker by creating the `epoll` instance, event file descriptors, and listener socket. It configures the listener, binds it to the given IP address and port, and registers its events with `epoll`. If anything goes wrong, it cleans up the resources and throws a runtime error.

**Parameters:**

- **ip:** IPv4 address to bind or inspect. Declaration: `in_addr ip`.
- **port:** Network port value. Declaration: `uint16_t port`.

**Returns:** No value (`void`).

**Errors:** Directly throws `std::runtime_error`. Calls made by this operation may propagate additional errors.

**Source:** [include/transport/server/tcp_linux_worker.h:111](../../../../include/transport/server/tcp_linux_worker.h#L111)

<a id="method-start"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> start

**METHOD** · *transport::server::worker*

```cpp
void start();
```

Starts the worker lifecycle.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_linux_worker.h:171](../../../../include/transport/server/tcp_linux_worker.h#L171)

<a id="method-is-current-thread"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_current_thread

**METHOD** · *transport::server::worker*

```cpp
bool is_current_thread() const;
```

Reports whether current thread applies.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/tcp_linux_worker.h:181](../../../../include/transport/server/tcp_linux_worker.h#L181)

<a id="method-is-current-thread-2"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_current_thread

**METHOD** · *transport::server::worker*

```cpp
static bool is_current_thread(const FAty& cr_engine);
```

Reports whether current thread applies.

**Parameters:**

- **cr_engine:** Protocol engine factory or engine reference. Declaration: `const FAty& cr_engine`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/tcp_linux_worker.h:185](../../../../include/transport/server/tcp_linux_worker.h#L185)

<a id="method-stop"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> stop

**METHOD** · *transport::server::worker*

```cpp
void stop();
```

Stops the worker lifecycle.

**Returns:** No value (`void`).

**Errors:** Directly throws `std::runtime_error`. Calls made by this operation may propagate additional errors.

**Source:** [include/transport/server/tcp_linux_worker.h:191](../../../../include/transport/server/tcp_linux_worker.h#L191)

<a id="method-request-stop"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> request_stop

**METHOD** · *transport::server::worker*

```cpp
void request_stop();
```

Asks the worker to stop by setting the `stopping_` flag and writing to the stop event file descriptor. If it’s already stopping, it returns right away. The method is thread-safe and can be called from any thread.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_linux_worker.h:209](../../../../include/transport/server/tcp_linux_worker.h#L209)

<a id="method-request-quiesce"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> request_quiesce

**METHOD** · *transport::server::worker*

```cpp
void request_quiesce();
```

Requests quiescence and signals the worker's stop event descriptor. Call while the worker is active and coordinate with `stop()`. Use `wait_quiesced()` to wait for the request to be handled.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_linux_worker.h:224](../../../../include/transport/server/tcp_linux_worker.h#L224)

<a id="method-wait-quiesced"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> wait_quiesced

**METHOD** · *transport::server::worker*

```cpp
void wait_quiesced() const;
```

Waits on `quiesced_` until the worker handles the request. Call `request_quiesce()` first.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_linux_worker.h:236](../../../../include/transport/server/tcp_linux_worker.h#L236)
