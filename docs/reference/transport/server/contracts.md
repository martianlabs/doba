<a id="contracts"></a>
<a href="../../../../resources/docs/reference/transport/server/contracts/title.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/contracts/title.svg" alt="contracts">
</a>
<a href="../../../../resources/docs/reference/transport/server/contracts/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/contracts/title-dark.svg" alt="contracts">
</a>

**Header:** [include/transport/server/contracts.h](../../../../include/transport/server/contracts.h)

Provides the execution work transport component.

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

Provides the execution work transport component.

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

**Source:** [include/transport/server/contracts.h:53](../../../../include/transport/server/contracts.h#L53)

<a id="field-owner"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> owner

**FIELD** · *transport::server::execution_capacity*

```cpp
void* owner{nullptr};
```

Stores the owner value.

**Value:** `{nullptr}`

**Source:** [include/transport/server/contracts.h:54](../../../../include/transport/server/contracts.h#L54)

<a id="field-submit"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> submit

**FIELD** · *transport::server::execution_capacity*

```cpp
void (*submit)(void*, std::chrono::steady_clock::time_point,
                 execution_work){nullptr};
```

Stores the submit value.

**Value:** `{nullptr}`

**Source:** [include/transport/server/contracts.h:55](../../../../include/transport/server/contracts.h#L55)

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

**Source:** [include/transport/server/contracts.h:67](../../../../include/transport/server/contracts.h#L67)

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

This concept defines the requirements for a transport type. It requires that the transport type is constructible from its policies and an engine factory, and that it provides methods for setting callbacks for connection and disconnection events, as well as starting and stopping the transport.

**Template parameters:**

- **TRty:** `typename TRty`.
- **ENty:** `typename ENty`.
- **FNty:** `typename FNty`.

**Source:** [include/transport/server/contracts.h:84](../../../../include/transport/server/contracts.h#L84)
