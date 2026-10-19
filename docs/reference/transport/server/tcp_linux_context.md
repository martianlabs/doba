<a id="tcp-linux-context"></a>
<a href="../../../../resources/docs/reference/transport/server/tcp_linux_context/index.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp_linux_context/index.svg" alt="tcp_linux_context">
</a>
<a href="../../../../resources/docs/reference/transport/server/tcp_linux_context/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp_linux_context/index-dark.svg" alt="tcp_linux_context">
</a>

**Header:** [include/transport/server/tcp_linux_context.h](../../../../include/transport/server/tcp_linux_context.h)

This struct holds an engine's TCP connection state. It manages the socket and input and output buffers, and handles sending and receiving data. It also lets you connect, stop, close, or abort the connection.

### <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transport::server

[context](#type-context)

### <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transport::server::context

[receive](#method-receive) ·
[connected](#method-connected) ·
[stop](#method-stop) ·
[close](#method-close) ·
[abort](#method-abort) ·
[can_receive](#method-can-receive) ·
[is_closed](#method-is-closed) ·
[eof](#method-eof) ·
[notify_connection](#method-notify-connection) ·
[send](#method-send) ·
[send_pending](#method-send-pending) ·
[retire_socket](#method-retire-socket) ·
[stop_engine](#method-stop-engine) ·
[mark_engine_complete](#method-mark-engine-complete) ·
[engine_complete](#method-engine-complete) ·
[notify_disconnection](#method-notify-disconnection) ·
[retirement_next](#field-retirement-next)

<a id="type-context"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> context

**TYPE** · *transport::server*

**Declaration synopsis:**

```cpp
template <protocol::contracts::engine ENty, typename CNty>
struct context : public std::enable_shared_from_this<context<ENty, CNty>> { ... };
```

This struct holds an engine's TCP connection state. It manages the socket and input and output buffers, and handles sending and receiving data. It also lets you connect, stop, close, or abort the connection.

**Template parameters:**

- **ENty:** `protocol::contracts::engine ENty`.
- **CNty:** `typename CNty`.

**Source:** [include/transport/server/tcp_linux_context.h:62](../../../../include/transport/server/tcp_linux_context.h#L62)

<a id="method-receive"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> receive

**METHOD** · *transport::server::context*

```cpp
ssize_t receive();
```

Reads data from the socket and passes it to the engine. It returns the number of bytes received, or a negative value if something goes wrong. If the connection is closing or the input buffer is full, it returns 0. It also catches errors during processing and aborts the connection if needed.

**Returns:** A value of type `ssize_t`.

**Source:** [include/transport/server/tcp_linux_context.h:109](../../../../include/transport/server/tcp_linux_context.h#L109)

<a id="method-connected"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> connected

**METHOD** · *transport::server::context*

```cpp
void connected(int epoll_fd, execution_capacity execution = {});
```

Sets the epoll file descriptor and execution capacity on the engine, with its `on_close` and `on_send` callbacks. When the engine closes, `on_close` calls this context’s `close()` method. When the engine wants to send data, `on_send` calls this context’s `send()` method with the head, body, and source it provides.

**Parameters:**

- **epoll_fd:** Linux epoll file descriptor. Declaration: `int epoll_fd`.
- **execution:** Execution state for this operation. Declaration: `execution_capacity execution = {}`.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_linux_context.h:148](../../../../include/transport/server/tcp_linux_context.h#L148)

<a id="method-stop"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> stop

**METHOD** · *transport::server::context*

```cpp
void stop(bool stopping = false);
```

Stops the connection and closes the output. If `stopping` is true, it means something outside the connection triggered the stop. The method sets the `stopping_` and `closing_` flags, then calls `close_output()` on the input. If that call fails, it aborts the connection.

**Parameters:**

- **stopping:** Whether shutdown is in progress. Declaration: `bool stopping = false`.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_linux_context.h:175](../../../../include/transport/server/tcp_linux_context.h#L175)

<a id="method-close"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> close

**METHOD** · *transport::server::context*

```cpp
void close();
```

Closes the connection and its output by setting the `closing_` flag and calling `close_output()` on the input. If that call fails, it aborts the connection.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_linux_context.h:188](../../../../include/transport/server/tcp_linux_context.h#L188)

<a id="method-abort"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> abort

**METHOD** · *transport::server::context*

```cpp
void abort();
```

Aborts the connection by calling the internal `abort_()` method.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_linux_context.h:203](../../../../include/transport/server/tcp_linux_context.h#L203)

<a id="method-can-receive"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> can_receive

**METHOD** · *transport::server::context*

```cpp
bool can_receive() const;
```

Returns true if the socket is valid and the connection isn’t closing, so it can receive data. Otherwise, it returns false.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/tcp_linux_context.h:213](../../../../include/transport/server/tcp_linux_context.h#L213)

<a id="method-is-closed"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_closed

**METHOD** · *transport::server::context*

```cpp
bool is_closed() const;
```

Returns true if the socket is invalid, which means the connection is closed. Otherwise, it returns false.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/tcp_linux_context.h:223](../../../../include/transport/server/tcp_linux_context.h#L223)

<a id="method-eof"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> eof

**METHOD** · *transport::server::context*

```cpp
bool eof() const;
```

Reports whether the eof condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/tcp_linux_context.h:230](../../../../include/transport/server/tcp_linux_context.h#L230)

<a id="method-notify-connection"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> notify_connection

**METHOD** · *transport::server::context*

```cpp
void notify_connection();
```

Lets the owner know the connection is up by calling `on_connection_`. It locks the mutex to check whether the connection has already been reported or is still being set up. If it hasn’t been reported yet, it sets the `notified_` flag and calls the callback.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_linux_context.h:239](../../../../include/transport/server/tcp_linux_context.h#L239)

<a id="method-send"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> send

**METHOD** · *transport::server::context*

```cpp
void send(std::string_view head, std::string_view body,
            std::unique_ptr<common::reader> source);
```

Sends data over the connection, using a mutex to keep things thread-safe. If the connection is closing, the socket is invalid, or there’s no data to send, it returns right away. Otherwise, it adds the data to the output buffer and updates the write interest. If anything fails, it aborts the connection.

**Parameters:**

- **head:** HTTP head data or its output buffer. Declaration: `std::string_view head`.
- **body:** HTTP body data or its output buffer. Declaration: `std::string_view body`.
- **source:** Source data or object to read from. Declaration: `std::unique_ptr<common::reader> source`.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_linux_context.h:256](../../../../include/transport/server/tcp_linux_context.h#L256)

<a id="method-send-pending"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> send_pending

**METHOD** · *transport::server::context*

```cpp
bool send_pending();
```

Sends any data waiting in the output buffer, while using a mutex to keep things thread-safe. If the socket is invalid, the connection has been aborted, or there’s nothing to send, it returns false. Otherwise, it gets the data ready and sends it over the socket. If the send succeeds, it updates the output state and checks whether there’s more data

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/tcp_linux_context.h:280](../../../../include/transport/server/tcp_linux_context.h#L280)

<a id="method-retire-socket"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> retire_socket

**METHOD** · *transport::server::context*

```cpp
int retire_socket();
```

Retires the socket by locking the mutex and checking whether it’s still valid. If it is, the method sets it to -1, marks the connection as closing, and clears the output buffer. It returns the retired socket, or -1 if the socket was already invalid.

**Returns:** A value of type `int`.

**Source:** [include/transport/server/tcp_linux_context.h:332](../../../../include/transport/server/tcp_linux_context.h#L332)

<a id="method-stop-engine"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> stop_engine

**METHOD** · *transport::server::context*

```cpp
bool stop_engine(std::function<void()> completed);
```

Stops protocol engine processing for this connection.

**Parameters:**

- **completed:** Completion state to store or report. Declaration: `std::function<void()> completed`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/tcp_linux_context.h:344](../../../../include/transport/server/tcp_linux_context.h#L344)

<a id="method-mark-engine-complete"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> mark_engine_complete

**METHOD** · *transport::server::context*

```cpp
void mark_engine_complete() noexcept;
```

Marks protocol engine processing as complete.

**Returns:** No value (`void`).

**Errors:** Declared `noexcept`.

**Source:** [include/transport/server/tcp_linux_context.h:353](../../../../include/transport/server/tcp_linux_context.h#L353)

<a id="method-engine-complete"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> engine_complete

**METHOD** · *transport::server::context*

```cpp
bool engine_complete() const noexcept;
```

Reports whether protocol engine processing is complete.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/transport/server/tcp_linux_context.h:359](../../../../include/transport/server/tcp_linux_context.h#L359)

<a id="method-notify-disconnection"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> notify_disconnection

**METHOD** · *transport::server::context*

```cpp
void notify_disconnection();
```

Tells the owner the connection has been disconnected by calling `on_disconnection_`. It locks the mutex to check whether this has already been reported. If not, it calls the callback and ignores any exceptions it throws.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tcp_linux_context.h:370](../../../../include/transport/server/tcp_linux_context.h#L370)

<a id="field-retirement-next"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> retirement_next

**FIELD** · *transport::server::context*

```cpp
context* retirement_next{nullptr};
```

Stores the retirement next value.

**Value:** `{nullptr}`

**Source:** [include/transport/server/tcp_linux_context.h:384](../../../../include/transport/server/tcp_linux_context.h#L384)
