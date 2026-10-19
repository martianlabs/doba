<a id="engine-h"></a>
<h1>
  <a href="../../../../../resources/docs/reference/protocol/http/v11/engine/index.svg#gh-light-mode-only">
    <img src="../../../../../resources/docs/reference/protocol/http/v11/engine/index.svg" alt="Index for engine.h">
  </a>
  <a href="../../../../../resources/docs/reference/protocol/http/v11/engine/index-dark.svg#gh-dark-mode-only">
    <img src="../../../../../resources/docs/reference/protocol/http/v11/engine/index-dark.svg" alt="Index for engine.h">
  </a>
</h1>

**Header:** [include/protocol/http/v11/engine.h](../../../../../include/protocol/http/v11/engine.h)

HTTP/1.1 protocol engine.

**protocol::http::v11**

- [engine](#type-engine) · *type*

**protocol::http::v11::engine**

- [policies_type](#alias-policies-type) · *alias*
- [set_on_send](#method-set-on-send) · *method*
- [set_on_close](#method-set-on-close) · *method*
- [set_execution_capacity](#method-set-execution-capacity) · *method*
- [stop](#method-stop) · *method*
- [on_bytes_received](#method-on-bytes-received) · *method*

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
void on_bytes_received(const char* buffer, std::size_t size);
```

Handles bytes received from the transport.

**Parameters:**

- **buffer:** Buffer used for the operation. Declaration: `const char* buffer`.
- **size:** Number of elements or bytes, according to the declared type. Declaration: `std::size_t size`.

**Returns:** No value (`void`).

**Source:** [include/protocol/http/v11/engine.h:128](../../../../../include/protocol/http/v11/engine.h#L128)
