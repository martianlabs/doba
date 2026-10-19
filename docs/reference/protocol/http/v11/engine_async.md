<a id="engine-async-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../resources/docs/reference/protocol/http/v11/engine_async/index-dark.svg">
    <img src="../../../../../resources/docs/reference/protocol/http/v11/engine_async/index.svg" alt="Index for engine_async.h">
  </picture>
</h1>

**Header:** [include/protocol/http/v11/engine_async.h](../../../../../include/protocol/http/v11/engine_async.h)

The asynchronous engine manages the state of multiple concurrent requests and responses. It ensures that responses are sent in the order of their corresponding requests, even if they complete out of order. It also handles connection closing and resource management for pending requests.

**protocol::http::v11**

- [engine_async](#type-engine-async) · *type*

**protocol::http::v11::engine_async**

- [engine_async(max_pending_requests, on_async_started, on_async_finished)](#constructor-engine-async) · *constructor*
- [engine_async(engine_async)](#constructor-engine-async-2) · *constructor*
- [engine_async(engine_async)](#constructor-engine-async-3) · *constructor*
- [~engine_async](#destructor-destructor-engine-async) · *destructor*
- [operator=(engine_async)](#operator-operator) · *operator*
- [operator=(engine_async)](#operator-operator-2) · *operator*
- [handles](#method-handles) · *method*
- [handle_request](#method-handle-request) · *method*
- [handle_error](#method-handle-error) · *method*
- [handle_interim](#method-handle-interim) · *method*
- [set_on_send](#method-set-on-send) · *method*
- [set_on_close](#method-set-on-close) · *method*
- [reserve](#method-reserve) · *method*
- [begin](#method-begin) · *method*
- [end](#method-end) · *method*
- [stop](#method-stop) · *method*
- [closing](#method-closing) · *method*
- [accepting](#method-accepting) · *method*
- [idle](#method-idle) · *method*
- [submit](#method-submit) · *method*
- [close_now](#method-close-now) · *method*

<a id="type-engine-async"></a>

## engine_async

🟠 **TYPE** · *protocol::http::v11*

**Declaration synopsis:**

```cpp
class engine_async { ... };
```

The asynchronous engine manages the state of multiple concurrent requests and responses. It ensures that responses are sent in the order of their corresponding requests, even if they complete out of order. It also handles connection closing and resource management for pending requests.

**Source:** [include/protocol/http/v11/engine_async.h:62](../../../../../include/protocol/http/v11/engine_async.h#L62)

<a id="constructor-engine-async"></a>

## engine_async

🟠 **CONSTRUCTOR** · *protocol::http::v11::engine_async*

```cpp
explicit engine_async(std::size_t max_pending_requests,
                        std::function<void()> on_async_started,
                        std::function<void()> on_async_finished);
```

Constructs a engine_async value.

**Parameters:**

- **max_pending_requests:** Initial value supplied to the constructor. Declaration: `std::size_t max_pending_requests`.
- **on_async_started:** Callback for the named event. Declaration: `std::function<void()> on_async_started`.
- **on_async_finished:** Callback for the named event. Declaration: `std::function<void()> on_async_finished`.

**Source:** [include/protocol/http/v11/engine_async.h:67](../../../../../include/protocol/http/v11/engine_async.h#L67)

<a id="constructor-engine-async-2"></a>

## engine_async

🟠 **CONSTRUCTOR** · *protocol::http::v11::engine_async*

```cpp
engine_async(const engine_async&) = delete;
```

Disallows this form of protocol::http::v11::engine_async construction.

**Parameters:**

- **engine_async:** Source object to copy or move. Declaration: `const engine_async&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/protocol/http/v11/engine_async.h:73](../../../../../include/protocol/http/v11/engine_async.h#L73)

<a id="constructor-engine-async-3"></a>

## engine_async

🟠 **CONSTRUCTOR** · *protocol::http::v11::engine_async*

```cpp
engine_async(engine_async&&) noexcept = delete;
```

Disallows this form of protocol::http::v11::engine_async construction.

**Parameters:**

- **engine_async:** Source object to copy or move. Declaration: `engine_async&&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/protocol/http/v11/engine_async.h:74](../../../../../include/protocol/http/v11/engine_async.h#L74)

<a id="destructor-destructor-engine-async"></a>

## ~engine_async

🟠 **DESTRUCTOR** · *protocol::http::v11::engine_async*

```cpp
~engine_async() = default;
```

Destroys the engine_async object.

**Source:** [include/protocol/http/v11/engine_async.h:75](../../../../../include/protocol/http/v11/engine_async.h#L75)

<a id="operator-operator"></a>

## operator=

🟠 **OPERATOR** · *protocol::http::v11::engine_async*

```cpp
engine_async& operator=(const engine_async&) = delete;
```

Disallows this operator= operation.

**Parameters:**

- **engine_async:** Source object to copy or move. Declaration: `const engine_async&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/protocol/http/v11/engine_async.h:79](../../../../../include/protocol/http/v11/engine_async.h#L79)

<a id="operator-operator-2"></a>

## operator=

🟠 **OPERATOR** · *protocol::http::v11::engine_async*

```cpp
engine_async& operator=(engine_async&&) noexcept = delete;
```

Disallows this operator= operation.

**Parameters:**

- **engine_async:** Source object to copy or move. Declaration: `engine_async&&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/protocol/http/v11/engine_async.h:80](../../../../../include/protocol/http/v11/engine_async.h#L80)

<a id="method-handles"></a>

## handles

🟠 **METHOD** · *protocol::http::v11::engine_async*

```cpp
template <typename Match>
  static bool handles(const Match& match);
```

Determines if the given router match result indicates that the request should be handled asynchronously.

**Template parameters:**

- **Match:** `typename Match`.

**Parameters:**

- **match:** Match result to inspect or update. Declaration: `const Match& match`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/engine_async.h:88](../../../../../include/protocol/http/v11/engine_async.h#L88)

<a id="method-handle-request"></a>

## handle_request

🟠 **METHOD** · *protocol::http::v11::engine_async*

```cpp
template <typename RQty, typename RSty, typename ROty>
  bool handle_request(decoder<RQty, RSty>& owner, const ROty& router,
                      RQty& request,
                      const router_match_result<RQty, RSty>& match,
                      std::string_view path,
                      transport::server::execution_capacity execution,
                      const std::shared_ptr<engine_async>& state,
                      bool& closed, std::string& head, std::string& body);
```

Handles an incoming request. If the request is to be handled asynchronously, it dispatches it to the appropriate handler and manages the response.

**Template parameters:**

- **RQty:** `typename RQty`.
- **RSty:** `typename RSty`.
- **ROty:** `typename ROty`.

**Parameters:**

- **owner:** Owning decoder or context. Declaration: `decoder<RQty, RSty>& owner`.
- **router:** Router used for request dispatch. Declaration: `const ROty& router`.
- **request:** Request being processed. Declaration: `RQty& request`.
- **match:** Match result to inspect or update. Declaration: `const router_match_result<RQty, RSty>& match`.
- **path:** Filesystem path or route path, according to the call. Declaration: `std::string_view path`.
- **execution:** Execution state for this operation. Declaration: `transport::server::execution_capacity execution`.
- **state:** Shared operation state. Declaration: `const std::shared_ptr<engine_async>& state`.
- **closed:** Connection-closed state. Declaration: `bool& closed`.
- **head:** HTTP head data or its output buffer. Declaration: `std::string& head`.
- **body:** HTTP body data or its output buffer. Declaration: `std::string& body`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/engine_async.h:99](../../../../../include/protocol/http/v11/engine_async.h#L99)

<a id="method-handle-error"></a>

## handle_error

🟠 **METHOD** · *protocol::http::v11::engine_async*

```cpp
template <typename RQty, typename RSty, typename ROty>
  bool handle_error(RSty& response, bool& closed);
```

Handles an error response. It serializes the response and submits it to the transport.

**Template parameters:**

- **RQty:** `typename RQty`.
- **RSty:** `typename RSty`.
- **ROty:** `typename ROty`.

**Parameters:**

- **response:** Response being produced. Declaration: `RSty& response`.
- **closed:** Connection-closed state. Declaration: `bool& closed`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/engine_async.h:131](../../../../../include/protocol/http/v11/engine_async.h#L131)

<a id="method-handle-interim"></a>

## handle_interim

🟠 **METHOD** · *protocol::http::v11::engine_async*

```cpp
template <typename RQty, typename RSty, typename ROty>
  bool handle_interim(RSty& response);
```

Handles an interim response. It serializes the response and submits it to the transport.

**Template parameters:**

- **RQty:** `typename RQty`.
- **RSty:** `typename RSty`.
- **ROty:** `typename ROty`.

**Parameters:**

- **response:** Response being produced. Declaration: `RSty& response`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/engine_async.h:146](../../../../../include/protocol/http/v11/engine_async.h#L146)

<a id="method-set-on-send"></a>

## set_on_send

🟠 **METHOD** · *protocol::http::v11::engine_async*

```cpp
void set_on_send(protocol::send_delegate output);
```

Sets on send.

**Parameters:**

- **output:** Destination or output callback, as specified by its type. Declaration: `protocol::send_delegate output`.

**Returns:** No value (`void`).

**Source:** [include/protocol/http/v11/engine_async.h:158](../../../../../include/protocol/http/v11/engine_async.h#L158)

<a id="method-set-on-close"></a>

## set_on_close

🟠 **METHOD** · *protocol::http::v11::engine_async*

```cpp
void set_on_close(std::function<void()> close);
```

Sets on close.

**Parameters:**

- **close:** Callback that closes the connection. Declaration: `std::function<void()> close`.

**Returns:** No value (`void`).

**Source:** [include/protocol/http/v11/engine_async.h:164](../../../../../include/protocol/http/v11/engine_async.h#L164)

<a id="method-reserve"></a>

## reserve

🟠 **METHOD** · *protocol::http::v11::engine_async*

```cpp
std::optional<std::size_t> reserve();
```

Reserves a position for a new request. If the maximum number of pending requests has been reached, it closes the connection and returns nullopt.

**Returns:** A `std::optional<std::size_t>` containing the result when present; otherwise `std::nullopt`.

**Source:** [include/protocol/http/v11/engine_async.h:173](../../../../../include/protocol/http/v11/engine_async.h#L173)

<a id="method-begin"></a>

## begin

🟠 **METHOD** · *protocol::http::v11::engine_async*

```cpp
void begin() noexcept;
```

Increments the count of active asynchronous operations. This should be called when an asynchronous operation starts.

**Returns:** No value (`void`).

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/v11/engine_async.h:189](../../../../../include/protocol/http/v11/engine_async.h#L189)

<a id="method-end"></a>

## end

🟠 **METHOD** · *protocol::http::v11::engine_async*

```cpp
void end();
```

Decrements the count of active asynchronous operations. If this was the last active operation and the engine is stopping, it calls the 'on_completed_' callback. This should be called when an asynchronous operation finishes.

**Returns:** No value (`void`).

**Source:** [include/protocol/http/v11/engine_async.h:198](../../../../../include/protocol/http/v11/engine_async.h#L198)

<a id="method-stop"></a>

## stop

🟠 **METHOD** · *protocol::http::v11::engine_async*

```cpp
bool stop(std::function<void()> completed);
```

Initiates the stopping process for the engine. It sets the stopping flag and, if there are no active asynchronous operations, it calls the 'on_completed_' callback.

**Parameters:**

- **completed:** Completion state to store or report. Declaration: `std::function<void()> completed`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/engine_async.h:217](../../../../../include/protocol/http/v11/engine_async.h#L217)

<a id="method-closing"></a>

## closing

🟠 **METHOD** · *protocol::http::v11::engine_async*

```cpp
bool closing() const noexcept;
```

Returns whether the engine is in the process of closing the connection.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/v11/engine_async.h:230](../../../../../include/protocol/http/v11/engine_async.h#L230)

<a id="method-accepting"></a>

## accepting

🟠 **METHOD** · *protocol::http::v11::engine_async*

```cpp
bool accepting() const noexcept;
```

Returns whether the engine is in the process of accepting new connections.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/v11/engine_async.h:239](../../../../../include/protocol/http/v11/engine_async.h#L239)

<a id="method-idle"></a>

## idle

🟠 **METHOD** · *protocol::http::v11::engine_async*

```cpp
bool idle() const noexcept;
```

Returns whether the engine is idle, meaning it is accepting new connections, has no active asynchronous operations, and all pending requests have been sent.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/v11/engine_async.h:249](../../../../../include/protocol/http/v11/engine_async.h#L249)

<a id="method-submit"></a>

## submit

🟠 **METHOD** · *protocol::http::v11::engine_async*

```cpp
void submit(std::size_t position, serialization_result result, bool final,
              bool close);
```

Submits a serialized response to the transport. It ensures that responses are sent in the order of their corresponding requests.

**Parameters:**

- **position:** Position in the input or output sequence. Declaration: `std::size_t position`.
- **result:** Result value to store or return. Declaration: `serialization_result result`.
- **final:** Whether this is the final part. Declaration: `bool final`.
- **close:** Callback that closes the connection. Declaration: `bool close`.

**Returns:** No value (`void`).

**Source:** [include/protocol/http/v11/engine_async.h:259](../../../../../include/protocol/http/v11/engine_async.h#L259)

<a id="method-close-now"></a>

## close_now

🟠 **METHOD** · *protocol::http::v11::engine_async*

```cpp
void close_now();
```

Immediately closes the connection. It sets the accepting flag to false, sets the closing flag to true, and calls the 'on_close_' callback.

**Returns:** No value (`void`).

**Source:** [include/protocol/http/v11/engine_async.h:320](../../../../../include/protocol/http/v11/engine_async.h#L320)
