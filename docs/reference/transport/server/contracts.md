<a id="contracts"></a>
<a href="../../../../resources/docs/reference/transport/server/contracts/title.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/contracts/title.svg" alt="contracts">
</a>
<a href="../../../../resources/docs/reference/transport/server/contracts/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/contracts/title-dark.svg" alt="contracts">
</a>

**Header:** [include/transport/server/contracts.h](../../../../include/transport/server/contracts.h)

Declares the structural transport interface. Protocols describe future
processing through [processing_result](../../protocol/processing_result.md),
returned directly by both engine entry points. No application work/executor
capability is supplied or injected by this contract.

<a id="concept-transport"></a>

## transport

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

The concept checks construction, notifications, start and stop. It cannot prove
runtime ownership or synchronization. The bundled backends additionally require:

- A sequential external owner configures, starts and stops the transport.
  Administrative calls cannot originate inside worker callbacks.
- RX and processing turns are serialized per connection. Send/close callbacks
  run only inside those entries; inline bytes are copied before return and
  terminal reader ownership transfers to the transport.
- Native operations retain their buffers and exact connection until completion.
- Close withdraws processing interests while accepted output drains. Linux
  administrative stop cancels pending output; Windows drains it and real IOCP
  operations before workers join.
- The engine owns coroutine frames, cooperative scheduling and response order.
  TCP delivers generic immediate/deadline/output-drain turns. TLS adapts those
  same entries without changing native TCP ownership.

See the [TCP runtime contract](tcp.md#runtime-contract),
[Linux turns](tcp_linux_processing.md) and
[Windows turns](tcp_windows_processing.md). These are behavioral requirements,
not additional executor, pressure or quiescence interfaces.
