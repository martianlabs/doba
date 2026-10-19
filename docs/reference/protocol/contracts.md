<a id="contracts-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/protocol/contracts/index-dark.svg">
    <img src="../../../resources/docs/reference/protocol/contracts/index.svg" alt="Index for contracts.h">
  </picture>
</h1>

**Header:** [include/protocol/contracts.h](../../../include/protocol/contracts.h)

This header declares the types and operations listed below.

**protocol**

- [send_delegate](#alias-send-delegate) · *alias*

**protocol::contracts**

- [engine](#concept-engine) · *concept*
- [completion_engine](#concept-completion-engine) · *concept*
- [engine_factory](#concept-engine-factory) · *concept*

<a id="alias-send-delegate"></a>

## send_delegate

🟠 **ALIAS** · *protocol*

```cpp
using send_delegate = std::function<void(std::string_view, std::string_view,
                                         std::unique_ptr<common::reader>)>;
```

A function that takes the head and body data, plus an optional source.

**Source:** [include/protocol/contracts.h:45](../../../include/protocol/contracts.h#L45)

<a id="concept-engine"></a>

## engine

🟠 **CONCEPT** · *protocol::contracts*

```cpp
template <typename ENty>
concept engine = requires(ENty& engine, const char* buf, std::size_t sze,
                          protocol::send_delegate output,
                          std::function<void()> close) {
  typename ENty::policies_type;
  { engine.set_on_send(std::move(output)) } -> std::same_as<void>;
  { engine.set_on_close(std::move(close)) } -> std::same_as<void>;
  { engine.on_bytes_received(buf, sze) } -> std::same_as<void>;
};
```

Protocol engine contract. Input callbacks are serialized per connection and stop when closing begins. The output delegate accepts concurrent submissions from any thread. The transport owns and drains submitted output independently of the engine.

**Template parameters:**

- **ENty:** `typename ENty`.

**Source:** [include/protocol/contracts.h:63](../../../include/protocol/contracts.h#L63)

<a id="concept-completion-engine"></a>

## completion_engine

🟠 **CONCEPT** · *protocol::contracts*

```cpp
template <typename ENty>
concept completion_engine =
    engine<ENty> && requires(ENty& engine, std::function<void()> completed) {
      { engine.stop(std::move(completed)) } -> std::same_as<bool>;
    };
```

A stopped engine reports when all work accepted by it has completed. A false result means the callback will be called exactly once later.

**Template parameters:**

- **ENty:** `typename ENty`.

**Source:** [include/protocol/contracts.h:81](../../../include/protocol/contracts.h#L81)

<a id="concept-engine-factory"></a>

## engine_factory

🟠 **CONCEPT** · *protocol::contracts*

```cpp
template <typename FNty, typename ENty>
concept engine_factory = engine<ENty> && std::move_constructible<FNty> &&
                         requires(const FNty& create_engine) {
                           { create_engine() } -> std::same_as<ENty>;
                         };
```

Protocol engine factory contract.

**Template parameters:**

- **FNty:** `typename FNty`.
- **ENty:** `typename ENty`.

**Source:** [include/protocol/contracts.h:94](../../../include/protocol/contracts.h#L94)
