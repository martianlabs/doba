<a id="contracts"></a>
<a href="../../../../resources/docs/reference/transport/server/contracts/title.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/contracts/title.svg" alt="contracts">
</a>
<a href="../../../../resources/docs/reference/transport/server/contracts/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/contracts/title-dark.svg" alt="contracts">
</a>

**Header:** [include/transport/server/contracts.h](../../../../include/transport/server/contracts.h)

Declares the structural transport interface and execution capability types.
Execution capabilities are supplied independently to engines; bundled TCP
provides synchronous socket I/O and never injects an application executor.
See the [TCP runtime contract](tcp.md#runtime-contract).

### <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transport::server

[execution_work](#type-execution-work) ·
[execution_capacity](#type-execution-capacity)

### <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transport::server::execution_work

[context](#field-context) ·
[run](#field-run)

### <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transport::server::execution_capacity

[owner](#field-owner) ·
[submit](#field-submit)

### <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transport::server::contracts

[execution_receiver](#concept-execution-receiver) ·
[transport](#concept-transport)

<a id="api"></a>
<a href="../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-execution-work"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> execution_work

**TYPE** · *transport::server*

**Declaration synopsis:**

```cpp
struct execution_work { ... };
```

An opaque work item for an independently supplied execution service. The
engine/service owns its state and execution lifetime. Bundled TCP does not
queue or execute these items.

**Source:** [include/transport/server/contracts.h:41](../../../../include/transport/server/contracts.h#L41)

<a id="field-context"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> context

**FIELD** · *transport::server::execution_work*

```cpp
void* context{nullptr};
```

Stores the context value.

**Value:** `{nullptr}`

**Source:** [include/transport/server/contracts.h:42](../../../../include/transport/server/contracts.h#L42)

<a id="field-run"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> run

**FIELD** · *transport::server::execution_work*

```cpp
void (*run)(void*) noexcept{nullptr};
```

Stores the run value.

**Value:** `{nullptr}`

**Source:** [include/transport/server/contracts.h:43](../../../../include/transport/server/contracts.h#L43)

<a id="type-execution-capacity"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> execution_capacity

**TYPE** · *transport::server*

**Declaration synopsis:**

```cpp
struct execution_capacity { ... };
```

While active, accepted work runs once, no earlier than its threshold.

**Source:** [include/transport/server/contracts.h:55](../../../../include/transport/server/contracts.h#L55)

<a id="field-owner"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> owner

**FIELD** · *transport::server::execution_capacity*

```cpp
void* owner{nullptr};
```

Stores the owner value.

**Value:** `{nullptr}`

**Source:** [include/transport/server/contracts.h:56](../../../../include/transport/server/contracts.h#L56)

<a id="field-submit"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> submit

**FIELD** · *transport::server::execution_capacity*

```cpp
void (*submit)(void*, std::chrono::steady_clock::time_point,
                 execution_work){nullptr};
```

Stores the submit value.

**Value:** `{nullptr}`

**Source:** [include/transport/server/contracts.h:57](../../../../include/transport/server/contracts.h#L57)

<a id="concept-execution-receiver"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> execution_receiver

**CONCEPT** · *transport::server::contracts*

```cpp
template <typename ENty>
concept execution_receiver =
    requires(ENty& engine, execution_capacity execution) {
      { engine.set_execution_capacity(execution) } -> std::same_as<void>;
    };
```

Checks the compile-time requirements for execution receiver.

**Template parameters:**

- **ENty:** `typename ENty`.

**Source:** [include/transport/server/contracts.h:69](../../../../include/transport/server/contracts.h#L69)

<a id="concept-transport"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transport

**CONCEPT** · *transport::server::contracts*

```cpp
template <typename TRty, typename ENty, typename FNty>
concept transport =
    protocol::contracts::engine_factory<FNty, ENty> &&
    std::constructible_from<TRty, typename TRty::policies_type, FNty> &&
    requires(TRty& tr, std::function<void()> callback) {
      typename TRty::policies_type;
      // Set the callback for when a connection is established
      { tr.set_on_connection(std::move(callback)) } -> std::same_as<void>;
      // Set the callback for when a connection is disconnected
      { tr.set_on_disconnection(std::move(callback)) } -> std::same_as<void>;
      // Start the transport
      { tr.start() } -> std::same_as<void>;
      // Stop the transport
      { tr.stop() } -> std::same_as<void>;
    };
```

Checks construction from policies and an engine factory, notification
registration, start and stop. Runtime requirements of bundled TCP also apply:

- A sequential external owner configures, starts and stops the transport;
  administrative calls must not originate inside worker callbacks.
- Input is serialized per connection. Send/close run synchronously inside input;
  accepted inline output is copied before returning and readers transfer ownership.
- Socket buffers and operation state remain valid until the kernel releases them.
- Engine close drains accepted output. Linux administrative stop cancels pending
  output; Windows administrative stop drains it and real IOCP completions.
- TLS, coroutine scheduling and application-task lifetime are outside TCP.

The concept checks signatures, not these behavioral guarantees. The
[TCP runtime contract](tcp.md#runtime-contract) defines ownership, terminal
readers, limits, errors and the deliberate differences between platforms.

**Template parameters:**

- **TRty:** `typename TRty`.
- **ENty:** `typename ENty`.
- **FNty:** `typename FNty`.

**Source:** [include/transport/server/contracts.h:90](../../../../include/transport/server/contracts.h#L90)

The bundled TCP transports do not supply execution_capacity. These shared
types remain available to protocol engines and their independently supplied
execution services.
