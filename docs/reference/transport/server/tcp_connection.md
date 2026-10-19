<a id="tcp-connection"></a>
<a href="../../../../resources/docs/reference/transport/server/tcp_connection/title.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp_connection/title.svg" alt="tcp_connection">
</a>
<a href="../../../../resources/docs/reference/transport/server/tcp_connection/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp_connection/title-dark.svg" alt="tcp_connection">
</a>

**Header:** [include/transport/server/tcp_connection.h](../../../../include/transport/server/tcp_connection.h)

This struct keeps track of a TCP connection used by an engine. It stores the engine and an input buffer, and handles incoming data, and outgoing data. It also lets you check the connection status and manage the output buffers.

### <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transport::server

[tcp_connection](#type-tcp-connection)

### <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transport::server::tcp_connection

[shared_state](#alias-shared-state) ·
[receive_buffer](#method-receive-buffer) ·
[process](#method-process) ·
[prepare_output](#method-prepare-output) ·
[output_bytes](#method-output-bytes) ·
[output_buffers](#method-output-buffers) ·
[output_sent](#method-output-sent) ·
[output_pending](#method-output-pending) ·
[established](#method-established) ·
[peer_closed](#method-peer-closed) ·
[eof](#method-eof) ·
[close_output](#method-close-output) ·
[engine](#field-engine) ·
[buffer](#field-buffer) ·
[capacity](#field-capacity)

<a id="api"></a>
<a href="../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-tcp-connection"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> tcp_connection

**TYPE** · *transport::server*

**Declaration synopsis:**

```cpp
template <protocol::contracts::engine ENty>
struct tcp_connection { ... };
```

This struct keeps track of a TCP connection used by an engine. It stores the engine and an input buffer, and handles incoming data, and outgoing data. It also lets you check the connection status and manage the output buffers.

**Template parameters:**

- **ENty:** `protocol::contracts::engine ENty`.

**Source:** [include/transport/server/tcp_connection.h:57](../../../../include/transport/server/tcp_connection.h#L57)

<a id="alias-shared-state"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> shared_state

**ALIAS** · *transport::server::tcp_connection*

```cpp
using shared_state = std::nullptr_t;
```

Alias for `std::nullptr_t`.

**Source:** [include/transport/server/tcp_connection.h:61](../../../../include/transport/server/tcp_connection.h#L61)

<a id="method-receive-buffer"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> receive_buffer

**METHOD** · *transport::server::tcp_connection*

```cpp
std::span<char> receive_buffer() noexcept;
```

Processes the received bytes in the input buffer using the engine.

**Returns:** A `std::span<char>` view of the selected buffer.

**Errors:** Declared `noexcept`.

**Source:** [include/transport/server/tcp_connection.h:79](../../../../include/transport/server/tcp_connection.h#L79)

<a id="method-process"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> process

**METHOD** · *transport::server::tcp_connection*

```cpp
void process(std::size_t received);
```

Processes pending connection input and output.

**Parameters:**

- **received:** Number of bytes received. Declaration: `std::size_t received`.

**Returns:** No value (`void`).

**Errors:** Directly throws `std::out_of_range`. Calls made by this operation may propagate additional errors.

**Source:** [include/transport/server/tcp_connection.h:82](../../../../include/transport/server/tcp_connection.h#L82)

<a id="method-prepare-output"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> prepare_output

**METHOD** · *transport::server::tcp_connection*

```cpp
bool prepare_output(send_state& output);
```

Gets the output buffer ready to send data. If it already contains unsent bytes, or the engine can add more data to it, the method returns true. Otherwise, it returns false.

**Parameters:**

- **output:** Destination or output callback, as specified by its type. Declaration: `send_state& output`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/tcp_connection.h:95](../../../../include/transport/server/tcp_connection.h#L95)

<a id="method-output-bytes"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> output_bytes

**METHOD** · *transport::server::tcp_connection*

```cpp
std::span<char> output_bytes(send_state& output);
```

Returns a span of the unsent bytes in the output buffer. If there are no unsent bytes, it returns an empty span.

**Parameters:**

- **output:** Destination or output callback, as specified by its type. Declaration: `send_state& output`.

**Returns:** A `std::span<char>` view of the selected buffer.

**Source:** [include/transport/server/tcp_connection.h:104](../../../../include/transport/server/tcp_connection.h#L104)

<a id="method-output-buffers"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> output_buffers

**METHOD** · *transport::server::tcp_connection*

```cpp
std::size_t output_buffers(send_state& output,
                             std::span<std::span<char>> buffers);
```

Fills the provided array of buffers with the unsent bytes in the output buffer.

**Parameters:**

- **output:** Destination or output callback, as specified by its type. Declaration: `send_state& output`.
- **buffers:** Buffers used for input or output. Declaration: `std::span<std::span<char>> buffers`.

**Returns:** The number of output buffers prepared.

**Source:** [include/transport/server/tcp_connection.h:113](../../../../include/transport/server/tcp_connection.h#L113)

<a id="method-output-sent"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> output_sent

**METHOD** · *transport::server::tcp_connection*

```cpp
bool output_sent(send_state& output, std::size_t sent);
```

Updates the output buffer after sending the specified number of bytes.

**Parameters:**

- **output:** Destination or output callback, as specified by its type. Declaration: `send_state& output`.
- **sent:** Number of bytes sent. Declaration: `std::size_t sent`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/tcp_connection.h:124](../../../../include/transport/server/tcp_connection.h#L124)

<a id="method-output-pending"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> output_pending

**METHOD** · *transport::server::tcp_connection*

```cpp
bool output_pending(const send_state& output) const;
```

Checks if there are any unsent bytes in the output buffer.

**Parameters:**

- **output:** Destination or output callback, as specified by its type. Declaration: `const send_state& output`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/tcp_connection.h:132](../../../../include/transport/server/tcp_connection.h#L132)

<a id="method-established"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> established

**METHOD** · *transport::server::tcp_connection*

```cpp
bool established() const;
```

Reports whether the connection is established.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/tcp_connection.h:138](../../../../include/transport/server/tcp_connection.h#L138)

<a id="method-peer-closed"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> peer_closed

**METHOD** · *transport::server::tcp_connection*

```cpp
bool peer_closed() const;
```

Reports whether the remote peer closed the connection.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/tcp_connection.h:142](../../../../include/transport/server/tcp_connection.h#L142)

<a id="method-eof"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> eof

**METHOD** · *transport::server::tcp_connection*

```cpp
bool eof() const;
```

Reports whether the eof condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/tcp_connection.h:146](../../../../include/transport/server/tcp_connection.h#L146)

<a id="method-close-output"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> close_output

**METHOD** · *transport::server::tcp_connection*

```cpp
bool close_output();
```

Closes output.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/tcp_connection.h:150](../../../../include/transport/server/tcp_connection.h#L150)

<a id="field-engine"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> engine

**FIELD** · *transport::server::tcp_connection*

```cpp
ENty engine;
```

Stores the engine value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/transport/server/tcp_connection.h:154](../../../../include/transport/server/tcp_connection.h#L154)

<a id="field-buffer"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> buffer

**FIELD** · *transport::server::tcp_connection*

```cpp
std::unique_ptr<char[]> buffer;
```

Stores the buffer value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/transport/server/tcp_connection.h:155](../../../../include/transport/server/tcp_connection.h#L155)

<a id="field-capacity"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> capacity

**FIELD** · *transport::server::tcp_connection*

```cpp
const std::size_t capacity;
```

Stores the capacity value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/transport/server/tcp_connection.h:156](../../../../include/transport/server/tcp_connection.h#L156)
