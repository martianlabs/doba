<a id="contracts"></a>
<a href="../../../resources/docs/reference/protocol/contracts/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/protocol/contracts/title.svg" alt="contracts">
</a>
<a href="../../../resources/docs/reference/protocol/contracts/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/protocol/contracts/title-dark.svg" alt="contracts">
</a>

**Header:** [include/protocol/contracts.h](../../../include/protocol/contracts.h)

This header declares the types and operations listed below.

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol

[send_delegate](#alias-send-delegate)

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::contracts

[engine](#concept-engine) ·
[completion_engine](#concept-completion-engine) ·
[engine_factory](#concept-engine-factory)

<a id="api"></a>
<a href="../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="alias-send-delegate"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> send_delegate

**ALIAS** · *protocol*

```cpp
using send_delegate = std::function<void(std::string_view, std::string_view,
                                         std::unique_ptr<common::reader>)>;
```

Sends head, body and an optional source, in that order. With bundled TCP,
head/body bytes are copied into transport storage before the callback returns;
reader ownership transfers to the transport. Borrowed backing storage must
remain valid until the reader is released. A reader is terminal: subsequent
nonempty output or another reader is rejected until that output drains.

Invoke send/close only inside the current engine input call, on the thread
executing that call. Registration stores the callbacks in the engine for its
context lifetime; they must not escape for delayed or cross-thread invocation.
See the [TCP runtime contract](../transport/server/tcp.md#runtime-contract)
for closure, empty submissions, limits and platform differences.

**Source:** [include/protocol/contracts.h:48](../../../include/protocol/contracts.h#L48)

<a id="concept-engine"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> engine

**CONCEPT** · *protocol::contracts*

```cpp
template <typename ENty>
concept engine = requires(ENty& engine, std::string_view input,
                          protocol::send_delegate output,
                          std::function<void()> close) {
  typename ENty::policies_type;
  { engine.set_on_send(std::move(output)) } -> std::same_as<void>;
  { engine.set_on_close(std::move(close)) } -> std::same_as<void>;
  { engine.on_bytes_received(input) } -> std::same_as<void>;
};
```

This concept checks the structural engine interface. Bundled TCP serializes
input per connection and stops delivering new input once closing begins. Input
views borrow immutable bytes only for the duration of the call; the engine
retains its own incomplete protocol messages.

Send/close execute synchronously inside that input call. Windows may use a
different IOCP worker for a later call; serialization does not imply fixed thread
affinity. Other connections may execute concurrently. Protocol work and any
independent execution service belong to the engine. TCP handles socket I/O,
accepted output and its own operation lifetime.

The concept cannot enforce these runtime rules. A custom implementation must
honor the [runtime contract](../transport/server/tcp.md#runtime-contract);
satisfying the C++ requires-expression alone does not establish conformance.

**Template parameters:**

- **ENty:** `typename ENty`.

**Source:** [include/protocol/contracts.h:67](../../../include/protocol/contracts.h#L67)

<a id="concept-completion-engine"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> completion_engine

**CONCEPT** · *protocol::contracts*

```cpp
template <typename ENty>
concept completion_engine =
    engine<ENty> && requires(ENty& engine, std::function<void()> completed) {
      { engine.stop(std::move(completed)) } -> std::same_as<bool>;
    };
```

A stopped engine reports when all work accepted by it has completed. A false
result means the callback will be called exactly once later. This optional
interface remains available to independent execution services. Bundled TCP
does not call engine.stop, schedule coroutines or wait for application tasks.

**Template parameters:**

- **ENty:** `typename ENty`.

**Source:** [include/protocol/contracts.h:87](../../../include/protocol/contracts.h#L87)

<a id="concept-engine-factory"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> engine_factory

**CONCEPT** · *protocol::contracts*

```cpp
template <typename FNty, typename ENty>
concept engine_factory = engine<ENty> && std::move_constructible<FNty> &&
                         requires(const FNty& create_engine) {
                           { create_engine() } -> std::same_as<ENty>;
                         };
```

Creates an independent engine for each connection. Factory calls for different
connections may overlap across workers. Synchronization of shared factory or
application state belongs to its owner.

**Template parameters:**

- **FNty:** `typename FNty`.
- **ENty:** `typename ENty`.

**Source:** [include/protocol/contracts.h:101](../../../include/protocol/contracts.h#L101)
