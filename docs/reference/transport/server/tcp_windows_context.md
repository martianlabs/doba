<a id="tcp-windows-context-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../resources/docs/reference/transport/server/tcp_windows_context/index-dark.svg">
    <img src="../../../../resources/docs/reference/transport/server/tcp_windows_context/index.svg" alt="Index for tcp_windows_context.h">
  </picture>
</h1>

**Header:** [include/transport/server/tcp_windows_context.h](../../../../include/transport/server/tcp_windows_context.h)

This struct keeps track of an engine's TCP connection. It manages the socket and input and output buffers, and handles sending and receiving data. It also lets you connect, stop, close, and abort the connection.

**transport::server**

- [context](#type-context) · *type*

**transport::server::context**

- [context&lt;FAty&gt;(in_socket, receive_buffer_capacity, send_buffer_size, create_engine, …)](#constructor-context) · *constructor*
- [context(context)](#constructor-context-2) · *constructor*
- [context(context)](#constructor-context-3) · *constructor*
- [~context](#destructor-destructor-context) · *destructor*
- [operator=(context)](#operator-operator) · *operator*
- [operator=(context)](#operator-operator-2) · *operator*
- [send_completed](#method-send-completed) · *method*
- [output_ready](#method-output-ready) · *method*
- [send](#method-send) · *method*
- [arm_next_receive_operation](#method-arm-next-receive-operation) · *method*
- [receive_completed](#method-receive-completed) · *method*
- [receive_failed](#method-receive-failed) · *method*
- [send_failed](#method-send-failed) · *method*
- [abort](#method-abort) · *method*
- [connected](#method-connected) · *method*
- [close](#method-close) · *method*
- [stop](#method-stop) · *method*
- [quiesce](#method-quiesce) · *method*
- [notify_disconnection](#method-notify-disconnection) · *method*
- [engine_completed](#method-engine-completed) · *method*
- [notify_connection](#method-notify-connection) · *method*
- [eof](#method-eof) · *method*

<a id="type-context"></a>

## context

🟠 **TYPE** · *transport::server*

**Declaration synopsis:**

```cpp
template <protocol::contracts::engine ENty, typename CNty>
struct context : public std::enable_shared_from_this<context<ENty, CNty>> { ... };
```

This struct keeps track of an engine's TCP connection. It manages the socket and input and output buffers, and handles sending and receiving data. It also lets you connect, stop, close, and abort the connection.

**Template parameters:**

- **ENty:** `protocol::contracts::engine ENty`.
- **CNty:** `typename CNty`.

**Source:** [include/transport/server/tcp_windows_context.h:65](../../../../include/transport/server/tcp_windows_context.h#L65)

<a id="constructor-context"></a>

## context

🟠 **CONSTRUCTOR** · *transport::server::context*

```cpp
template <protocol::contracts::engine_factory<ENty> FAty>
  context(SOCKET in_socket, std::size_t receive_buffer_capacity,
          std::size_t send_buffer_size, const FAty& create_engine,
          const std::atomic<bool>& stopping,
          types::on_client_connected_delegate on_connection,
          types::on_client_disconnected_delegate on_disconnection,
          std::function<void(context*)> on_retirement,
          typename CNty::shared_state shared_state,
          std::size_t encrypted_receive_buffer_capacity = 0,
          std::size_t network_bio_buffer_capacity = 0);
```

Constructs a context value.

**Template parameters:**

- **FAty:** `protocol::contracts::engine_factory<ENty> FAty`.

**Parameters:**

- **in_socket:** Initial value supplied to the constructor. Declaration: `SOCKET in_socket`.
- **receive_buffer_capacity:** Initial value supplied to the constructor. Declaration: `std::size_t receive_buffer_capacity`.
- **send_buffer_size:** Size or length limit for the named data. Declaration: `std::size_t send_buffer_size`.
- **create_engine:** Initial value supplied to the constructor. Declaration: `const FAty& create_engine`.
- **stopping:** Whether shutdown is in progress. Declaration: `const std::atomic<bool>& stopping`.
- **on_connection:** Callback for the named event. Declaration: `types::on_client_connected_delegate on_connection`.
- **on_disconnection:** Callback for the named event. Declaration: `types::on_client_disconnected_delegate on_disconnection`.
- **on_retirement:** Callback for the named event. Declaration: `std::function<void(context*)> on_retirement`.
- **shared_state:** Initial value supplied to the constructor. Declaration: `typename CNty::shared_state shared_state`.
- **encrypted_receive_buffer_capacity:** Initial value supplied to the constructor. Declaration: `std::size_t encrypted_receive_buffer_capacity = 0`.
- **network_bio_buffer_capacity:** Initial value supplied to the constructor. Declaration: `std::size_t network_bio_buffer_capacity = 0`.

**Source:** [include/transport/server/tcp_windows_context.h:70](../../../../include/transport/server/tcp_windows_context.h#L70)

<a id="constructor-context-2"></a>

## context

🟠 **CONSTRUCTOR** · *transport::server::context*

```cpp
context(const context&) = delete;
```

Disallows this form of transport::server::context construction.

**Parameters:**

- **context:** Context for this operation. Declaration: `const context&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/transport/server/tcp_windows_context.h:99](../../../../include/transport/server/tcp_windows_context.h#L99)

<a id="constructor-context-3"></a>

## context

🟠 **CONSTRUCTOR** · *transport::server::context*

```cpp
context(context&&) noexcept = delete;
```

Disallows this form of transport::server::context construction.

**Parameters:**

- **context:** Context for this operation. Declaration: `context&&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/transport/server/tcp_windows_context.h:100](../../../../include/transport/server/tcp_windows_context.h#L100)

<a id="destructor-destructor-context"></a>

## ~context

🟠 **DESTRUCTOR** · *transport::server::context*

```cpp
~context() = default;
```

Destroys the context object.

**Source:** [include/transport/server/tcp_windows_context.h:101](../../../../include/transport/server/tcp_windows_context.h#L101)

<a id="operator-operator"></a>

## operator=

🟠 **OPERATOR** · *transport::server::context*

```cpp
context& operator=(const context&) = delete;
```

Disallows this operator= operation.

**Parameters:**

- **context:** Context for this operation. Declaration: `const context&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/transport/server/tcp_windows_context.h:105](../../../../include/transport/server/tcp_windows_context.h#L105)

<a id="operator-operator-2"></a>

## operator=

🟠 **OPERATOR** · *transport::server::context*

```cpp
context& operator=(context&&) noexcept = delete;
```

Disallows this operator= operation.

**Parameters:**

- **context:** Context for this operation. Declaration: `context&&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/transport/server/tcp_windows_context.h:106](../../../../include/transport/server/tcp_windows_context.h#L106)

<a id="method-send-completed"></a>

## send_completed

🟠 **METHOD** · *transport::server::context*

```cpp
void send_completed(std::size_t size, std::size_t submitted_size);
```

Called when a send operation finishes. It takes the number of bytes sent and the total number submitted, then updates the send state and checks the output buffer. If the socket is invalid or something goes wrong, it aborts the connection. Otherwise, it keeps sending any data that’s still waiting.

**Parameters:**

- **size:** Number of elements or bytes, according to the declared type. Declaration: `std::size_t size`.
- **submitted_size:** Size or length limit for the named data. Declaration: `std::size_t submitted_size`.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_windows_context.h:116](../../../../include/transport/server/tcp_windows_context.h#L116)

<a id="method-output-ready"></a>

## output_ready

🟠 **METHOD** · *transport::server::context*

```cpp
void output_ready();
```

Called when the output buffer is ready to send more data. It resets the send state to idle and checks whether anything else is waiting. If the socket is invalid or something goes wrong, it aborts the connection. Otherwise, it keeps sending the remaining data.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_windows_context.h:137](../../../../include/transport/server/tcp_windows_context.h#L137)

<a id="method-send"></a>

## send

🟠 **METHOD** · *transport::server::context*

```cpp
void send(std::string_view head, std::string_view body,
            std::unique_ptr<common::reader> source);
```

Sends data over the connection using a head, body, and optional source reader. It locks the send mutex to keep things thread-safe, then checks that the connection isn’t closing and the socket is valid. If there’s room, it adds the data to the output buffer and asks for it to be sent. If the data won’t fit, it marks the connection as aborted and

**Parameters:**

- **head:** HTTP head data or its output buffer. Declaration: `std::string_view head`.
- **body:** HTTP body data or its output buffer. Declaration: `std::string_view body`.
- **source:** Source data or object to read from. Declaration: `std::unique_ptr<common::reader> source`.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_windows_context.h:152](../../../../include/transport/server/tcp_windows_context.h#L152)

<a id="method-arm-next-receive-operation"></a>

## arm_next_receive_operation

🟠 **METHOD** · *transport::server::context*

```cpp
bool arm_next_receive_operation();
```

Gets the next receive operation ready. It locks the send mutex to keep things thread-safe, then checks whether the connection is quiescing or stopping. If so, it aborts the connection or closes the input. It also aborts if the socket is invalid or a receive cannot be armed. Otherwise, it starts another receive operation and returns true.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/tcp_windows_context.h:175](../../../../include/transport/server/tcp_windows_context.h#L175)

<a id="method-receive-completed"></a>

## receive_completed

🟠 **METHOD** · *transport::server::context*

```cpp
bool receive_completed(std::size_t size);
```

Called when a receive operation finishes. It takes the number of bytes received and updates the input buffer. It locks the receive mutex to keep things thread-safe, then checks whether the connection is quiescing or stopping. If so, it aborts the connection or closes the input. It also aborts if the socket is invalid or processing fails. full.

**Parameters:**

- **size:** Number of elements or bytes, according to the declared type. Declaration: `std::size_t size`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/tcp_windows_context.h:209](../../../../include/transport/server/tcp_windows_context.h#L209)

<a id="method-receive-failed"></a>

## receive_failed

🟠 **METHOD** · *transport::server::context*

```cpp
void receive_failed();
```

Called when a receive operation fails. It locks the send mutex, marks receiving as closed, and aborts the connection.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_windows_context.h:259](../../../../include/transport/server/tcp_windows_context.h#L259)

<a id="method-send-failed"></a>

## send_failed

🟠 **METHOD** · *transport::server::context*

```cpp
void send_failed();
```

Called when a send operation fails. It locks the send mutex, resets the send state to idle, and aborts the connection.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_windows_context.h:270](../../../../include/transport/server/tcp_windows_context.h#L270)

<a id="method-abort"></a>

## abort

🟠 **METHOD** · *transport::server::context*

```cpp
void abort();
```

Aborts the pending operation.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_windows_context.h:278](../../../../include/transport/server/tcp_windows_context.h#L278)

<a id="method-connected"></a>

## connected

🟠 **METHOD** · *transport::server::context*

```cpp
void connected(HANDLE completion_port, execution_capacity execution = {});
```

Called when the connection is established. It takes a completion port handle and an optional execution capacity. It locks the send mutex, the completion port handle, and configures the engine with the capacity and callbacks for closing and sending.

**Parameters:**

- **completion_port:** Windows I/O completion port handle. Declaration: `HANDLE completion_port`.
- **execution:** Execution state for this operation. Declaration: `execution_capacity execution = {}`.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_windows_context.h:290](../../../../include/transport/server/tcp_windows_context.h#L290)

<a id="method-close"></a>

## close

🟠 **METHOD** · *transport::server::context*

```cpp
void close();
```

Closes the connection and its output by setting `closing_` and calling `close_output()` on the input. If that fails, it aborts the connection.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_windows_context.h:315](../../../../include/transport/server/tcp_windows_context.h#L315)

<a id="method-stop"></a>

## stop

🟠 **METHOD** · *transport::server::context*

```cpp
void stop();
```

Stops the connection and closes the output. It locks the send mutex, sets `stopping_` and `closing_`, then calls `close_output()` on the input.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_windows_context.h:332](../../../../include/transport/server/tcp_windows_context.h#L332)

<a id="method-quiesce"></a>

## quiesce

🟠 **METHOD** · *transport::server::context*

```cpp
void quiesce();
```

Quiesces the connection by locking both the receive and send mutexes and setting `quiescing_` to `true`. This stops any new send or receive operations from starting.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_windows_context.h:352](../../../../include/transport/server/tcp_windows_context.h#L352)

<a id="method-notify-disconnection"></a>

## notify_disconnection

🟠 **METHOD** · *transport::server::context*

```cpp
void notify_disconnection();
```

Lets the owner know the connection has been disconnected by calling `on_disconnection_`. It locks the send mutex to check whether this has already been reported. If not, it sets `disconnected_` to `true` and calls the callback, ignoring any exceptions it throws.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_windows_context.h:365](../../../../include/transport/server/tcp_windows_context.h#L365)

<a id="method-engine-completed"></a>

## engine_completed

🟠 **METHOD** · *transport::server::context*

```cpp
void engine_completed();
```

Reports whether protocol engine processing completed.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_windows_context.h:383](../../../../include/transport/server/tcp_windows_context.h#L383)

<a id="method-notify-connection"></a>

## notify_connection

🟠 **METHOD** · *transport::server::context*

```cpp
void notify_connection();
```

Lets the owner know the connection is up by calling `on_connection_`. It locks the send mutex to check whether the connection has already been reported or is still being set up. If it hasn’t been reported yet, it sets `notified_` to `true` and calls the callback.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_windows_context.h:396](../../../../include/transport/server/tcp_windows_context.h#L396)

<a id="method-eof"></a>

## eof

🟠 **METHOD** · *transport::server::context*

```cpp
bool eof() const;
```

Reports whether the eof condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/tcp_windows_context.h:407](../../../../include/transport/server/tcp_windows_context.h#L407)
