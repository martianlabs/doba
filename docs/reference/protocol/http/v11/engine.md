<a id="engine-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../resources/docs/reference/protocol/http/v11/engine/index-dark.svg">
    <img src="../../../../../resources/docs/reference/protocol/http/v11/engine/index.svg" alt="Index for engine.h">
  </picture>
</h1>

**Header:** [include/protocol/http/v11/engine.h](../../../../../include/protocol/http/v11/engine.h)

HTTP/1.1 protocol engine.

**protocol::http::v11**

- [engine](#type-engine) · *type*

**protocol::http::v11::engine**

- [policies_type](#alias-policies-type) · *alias*
- [engine(configuration, router, on_async_started, on_async_finished)](#constructor-engine) · *constructor*
- [engine(engine)](#constructor-engine-2) · *constructor*
- [engine(engine)](#constructor-engine-3) · *constructor*
- [~engine](#destructor-destructor-engine) · *destructor*
- [operator=(engine)](#operator-operator) · *operator*
- [operator=(engine)](#operator-operator-2) · *operator*
- [set_on_send](#method-set-on-send) · *method*
- [set_on_close](#method-set-on-close) · *method*
- [set_execution_capacity](#method-set-execution-capacity) · *method*
- [stop](#method-stop) · *method*
- [on_bytes_received](#method-on-bytes-received) · *method*

<a id="type-engine"></a>

## engine

🟠 **TYPE** · *protocol::http::v11*

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

## policies_type

🟠 **ALIAS** · *protocol::http::v11::engine*

```cpp
using policies_type = v11::policies;
```

Alias for `v11::policies`.

**Source:** [include/protocol/http/v11/engine.h:69](../../../../../include/protocol/http/v11/engine.h#L69)

<a id="constructor-engine"></a>

## engine

🟠 **CONSTRUCTOR** · *protocol::http::v11::engine*

```cpp
engine(policies_type configuration, const ROty& router,
         std::function<void()> on_async_started = {},
         std::function<void()> on_async_finished = {});
```

Constructs a engine value.

**Parameters:**

- **configuration:** Configuration for this component. Declaration: `policies_type configuration`.
- **router:** Router used for request dispatch. Declaration: `const ROty& router`.
- **on_async_started:** Callback for the named event. Declaration: `std::function<void()> on_async_started = {}`.
- **on_async_finished:** Callback for the named event. Declaration: `std::function<void()> on_async_finished = {}`.

**Source:** [include/protocol/http/v11/engine.h:73](../../../../../include/protocol/http/v11/engine.h#L73)

<a id="constructor-engine-2"></a>

## engine

🟠 **CONSTRUCTOR** · *protocol::http::v11::engine*

```cpp
engine(const engine&) = delete;
```

Disallows this form of protocol::http::v11::engine construction.

**Parameters:**

- **engine:** Source object to copy or move. Declaration: `const engine&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/protocol/http/v11/engine.h:81](../../../../../include/protocol/http/v11/engine.h#L81)

<a id="constructor-engine-3"></a>

## engine

🟠 **CONSTRUCTOR** · *protocol::http::v11::engine*

```cpp
engine(engine&&) noexcept = delete;
```

Disallows this form of protocol::http::v11::engine construction.

**Parameters:**

- **engine:** Source object to copy or move. Declaration: `engine&&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/protocol/http/v11/engine.h:82](../../../../../include/protocol/http/v11/engine.h#L82)

<a id="destructor-destructor-engine"></a>

## ~engine

🟠 **DESTRUCTOR** · *protocol::http::v11::engine*

```cpp
~engine() = default;
```

Destroys the engine object.

**Source:** [include/protocol/http/v11/engine.h:83](../../../../../include/protocol/http/v11/engine.h#L83)

<a id="operator-operator"></a>

## operator=

🟠 **OPERATOR** · *protocol::http::v11::engine*

```cpp
engine& operator=(const engine&) = delete;
```

Disallows this operator= operation.

**Parameters:**

- **engine:** Source object to copy or move. Declaration: `const engine&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/protocol/http/v11/engine.h:87](../../../../../include/protocol/http/v11/engine.h#L87)

<a id="operator-operator-2"></a>

## operator=

🟠 **OPERATOR** · *protocol::http::v11::engine*

```cpp
engine& operator=(engine&&) noexcept = delete;
```

Disallows this operator= operation.

**Parameters:**

- **engine:** Source object to copy or move. Declaration: `engine&&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/protocol/http/v11/engine.h:88](../../../../../include/protocol/http/v11/engine.h#L88)

<a id="method-set-on-send"></a>

## set_on_send

🟠 **METHOD** · *protocol::http::v11::engine*

```cpp
void set_on_send(protocol::send_delegate output);
```

Let's set the 'on_send' callback to send data to the transport.

**Parameters:**

- **output:** Destination or output callback, as specified by its type. Declaration: `protocol::send_delegate output`.

**Returns:** No value (`void`).

**Source:** [include/protocol/http/v11/engine.h:94](../../../../../include/protocol/http/v11/engine.h#L94)

<a id="method-set-on-close"></a>

## set_on_close

🟠 **METHOD** · *protocol::http::v11::engine*

```cpp
void set_on_close(std::function<void()> close);
```

Let's set the 'on_close' callback to close the transport.

**Parameters:**

- **close:** Callback that closes the connection. Declaration: `std::function<void()> close`.

**Returns:** No value (`void`).

**Source:** [include/protocol/http/v11/engine.h:103](../../../../../include/protocol/http/v11/engine.h#L103)

<a id="method-set-execution-capacity"></a>

## set_execution_capacity

🟠 **METHOD** · *protocol::http::v11::engine*

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

## stop

🟠 **METHOD** · *protocol::http::v11::engine*

```cpp
bool stop(std::function<void()> completed);
```

Stops the engine lifecycle.

**Parameters:**

- **completed:** Completion state to store or report. Declaration: `std::function<void()> completed`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/engine.h:119](../../../../../include/protocol/http/v11/engine.h#L119)

<a id="method-on-bytes-received"></a>

## on_bytes_received

🟠 **METHOD** · *protocol::http::v11::engine*

```cpp
void on_bytes_received(const char* buffer, std::size_t size);
```

Handles bytes received from the transport.

**Parameters:**

- **buffer:** Buffer used for the operation. Declaration: `const char* buffer`.
- **size:** Number of elements or bytes, according to the declared type. Declaration: `std::size_t size`.

**Returns:** No value (`void`).

**Source:** [include/protocol/http/v11/engine.h:128](../../../../../include/protocol/http/v11/engine.h#L128)
