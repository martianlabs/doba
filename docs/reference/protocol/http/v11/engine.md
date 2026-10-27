<a id="engine"></a>
<a href="../../../../../resources/docs/reference/protocol/http/v11/engine/title.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/v11/engine/title.svg" alt="engine">
</a>
<a href="../../../../../resources/docs/reference/protocol/http/v11/engine/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/v11/engine/title-dark.svg" alt="engine">
</a>

**Header:** [include/protocol/http/v11/engine.h](../../../../../include/protocol/http/v11/engine.h)

HTTP/1.1 protocol engine.

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11

[engine](#type-engine)

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::engine

[policies_type](#alias-policies-type) ·
[set_on_send](#method-set-on-send) ·
[set_on_close](#method-set-on-close) ·
[set_execution_capacity](#method-set-execution-capacity) ·
[stop](#method-stop) ·
[on_bytes_received](#method-on-bytes-received)

[set_input_flow_control](#method-set-input-flow-control)

[input_paused](#method-input-paused)

[resume_input](#method-resume-input)

<a id="api"></a>
<a href="../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-engine"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> engine

**TYPE** · *protocol::http::v11*

**Declaration synopsis:**

```cpp
template <contracts::request RQty, contracts::response RSty,
          typename ROty = router<RQty, RSty>>
  requires contracts::router<ROty, RQty, RSty>
class engine { ... };
```

HTTP/1.1 protocol engine.

**Template parameters:**

- **RQty:** `contracts::request RQty`.
- **RSty:** `contracts::response RSty`.
- **ROty:** `typename ROty = router<RQty, RSty>`.

**Source:** [include/protocol/http/v11/engine.h:64](../../../../../include/protocol/http/v11/engine.h#L64)

<a id="alias-policies-type"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> policies_type

**ALIAS** · *protocol::http::v11::engine*

```cpp
using policies_type = v11::policies;
```

Alias for `v11::policies`.

**Source:** [include/protocol/http/v11/engine.h:69](../../../../../include/protocol/http/v11/engine.h#L69)

<a id="method-set-on-send"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> set_on_send

**METHOD** · *protocol::http::v11::engine*

```cpp
void set_on_send(protocol::send_delegate output);
```

Let's set the 'on_send' callback to send data to the transport.

**Parameters:**

- **output:** Destination or output callback, as specified by its type. Declaration: `protocol::send_delegate output`.

**Returns:** No value (`void`).

**Source:** [include/protocol/http/v11/engine.h:94](../../../../../include/protocol/http/v11/engine.h#L94)

<a id="method-set-on-close"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> set_on_close

**METHOD** · *protocol::http::v11::engine*

```cpp
void set_on_close(std::function<void()> close);
```

Let's set the 'on_close' callback to close the transport.

**Parameters:**

- **close:** Callback that closes the connection. Declaration: `std::function<void()> close`.

**Returns:** No value (`void`).

**Source:** [include/protocol/http/v11/engine.h:103](../../../../../include/protocol/http/v11/engine.h#L103)

<a id="method-set-execution-capacity"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> set_execution_capacity

**METHOD** · *protocol::http::v11::engine*

```cpp
void set_execution_capacity(
      transport::server::execution_capacity execution) noexcept;
```

Sets the execution capacity for the engine.

**Parameters:**

- **execution:** Execution state for this operation. Declaration: `transport::server::execution_capacity execution`.

**Returns:** No value (`void`).

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/v11/engine.h:112](../../../../../include/protocol/http/v11/engine.h#L112)

<a id="method-stop"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> stop

**METHOD** · *protocol::http::v11::engine*

```cpp
bool stop(std::function<void()> completed);
```

Stops the engine lifecycle.

**Parameters:**

- **completed:** Completion state to store or report. Declaration: `std::function<void()> completed`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/engine.h:119](../../../../../include/protocol/http/v11/engine.h#L119)

<a id="method-on-bytes-received"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> on_bytes_received

**METHOD** · *protocol::http::v11::engine*

```cpp
void on_bytes_received(std::string_view input);
```

Handles bytes received from the transport.

**Parameters:**

- **input:** A borrowed view of the newly received bytes. Declaration: `std::string_view input`. The transport keeps these bytes immutable and valid throughout the call.

**Returns:** No value (`void`).

**Source:** [include/protocol/http/v11/engine.h:141](../../../../../include/protocol/http/v11/engine.h#L141)

<a id="method-set-input-flow-control"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> set_input_flow_control

**METHOD** · *protocol::http::v11::engine*

```cpp
void set_input_flow_control(std::function<bool()> pressure, std::function<void()> ready);
```

Optional generic transport integration, configured before receiving input. The pressure callback reports transport output pressure. The ready callback schedules an input-service opportunity when ordered-response credit changes; it must not reenter the engine. Bundled transports use weak context ownership. Engines without this optional integration retain hard-limit closure behavior.

**Source:** [include/protocol/http/v11/engine.h:130](../../../../../include/protocol/http/v11/engine.h#L130)

<a id="method-input-paused"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> input_paused

**METHOD** · *protocol::http::v11::engine*

```cpp
bool input_paused() const noexcept;
```

Reports whether input processing stopped at a message boundary for output or pending-response credit. The transport must suspend further input delivery, the engine owns a copy of the deferred remainder of one bounded receive block. An in-progress message can finish before pausing so that an interim response does not prevent its body from arriving.

**Source:** [include/protocol/http/v11/engine.h:135](../../../../../include/protocol/http/v11/engine.h#L135)

<a id="method-resume-input"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> resume_input

**METHOD** · *protocol::http::v11::engine*

```cpp
void resume_input();
```

Services the deferred remainder of one receive block and reevaluates pressure. Serialize with all receive callbacks and stop; invoke outside transport output locks. The bundled TCP and TLS transports resume input on their workers. The ready callback is a wakeup, not permission to invoke this method concurrently.

**Source:** [include/protocol/http/v11/engine.h:136](../../../../../include/protocol/http/v11/engine.h#L136)
