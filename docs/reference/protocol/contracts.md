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

A function that takes the head and body data, plus an optional source.

**Source:** [include/protocol/contracts.h:45](../../../include/protocol/contracts.h#L45)

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

Protocol engine contract. Input callbacks are serialized per connection and stop when closing begins. Input views borrow bytes that remain immutable and valid throughout the callback. The output delegate accepts concurrent submissions from any thread. The transport owns and drains submitted output independently of the engine.

**Template parameters:**

- **ENty:** `typename ENty`.

**Source:** [include/protocol/contracts.h:64](../../../include/protocol/contracts.h#L64)

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

A stopped engine reports when all work accepted by it has completed. A false result means the callback will be called exactly once later.

**Template parameters:**

- **ENty:** `typename ENty`.

**Source:** [include/protocol/contracts.h:82](../../../include/protocol/contracts.h#L82)

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

Protocol engine factory contract.

**Template parameters:**

- **FNty:** `typename FNty`.
- **ENty:** `typename ENty`.

**Source:** [include/protocol/contracts.h:95](../../../include/protocol/contracts.h#L95)
