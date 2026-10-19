<a id="tls-connection-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../resources/docs/reference/transport/server/tls_connection/index-dark.svg">
    <img src="../../../../resources/docs/reference/transport/server/tls_connection/index.svg" alt="Index for tls_connection.h">
  </picture>
</h1>

**Header:** [include/transport/server/tls_connection.h](../../../../include/transport/server/tls_connection.h)

This struct wraps an engine and a TLS session to handle an encrypted connection. It manages the encrypted and plaintext buffers, handles the TLS handshake, and processes incoming and outgoing data. It also lets you process received data, prepare output, and check the connection state.

**transport::server**

- [tls_connection](#type-tls-connection) · *type*

**transport::server::tls_connection**

- [shared_state](#alias-shared-state) · *alias*
- [tls_connection&lt;FAty&gt;(receive_buffer_capacity, send_buffer_size, create_engine, context)](#constructor-tls-connection) · *constructor*
- [tls_connection&lt;FAty&gt;(receive_buffer_capacity, send_buffer_size, create_engine, context, …)](#constructor-tls-connection-2) · *constructor*
- [receive_buffer](#method-receive-buffer) · *method*
- [process](#method-process) · *method*
- [prepare_output](#method-prepare-output) · *method*
- [output_bytes](#method-output-bytes) · *method*
- [output_buffers](#method-output-buffers) · *method*
- [output_sent](#method-output-sent) · *method*
- [output_pending](#method-output-pending) · *method*
- [established](#method-established) · *method*
- [peer_closed](#method-peer-closed) · *method*
- [eof](#method-eof) · *method*
- [close_output](#method-close-output) · *method*
- [engine](#field-engine) · *field*
- [buffer](#field-buffer) · *field*
- [capacity](#field-capacity) · *field*
- [size](#field-size) · *field*

<a id="type-tls-connection"></a>

## tls_connection

🟠 **TYPE** · *transport::server*

**Declaration synopsis:**

```cpp
template <protocol::contracts::engine ENty>
struct tls_connection { ... };
```

This struct wraps an engine and a TLS session to handle an encrypted connection. It manages the encrypted and plaintext buffers, handles the TLS handshake, and processes incoming and outgoing data. It also lets you process received data, prepare output, and check the connection state.

**Template parameters:**

- **ENty:** `protocol::contracts::engine ENty`.

**Source:** [include/transport/server/tls_connection.h:57](../../../../include/transport/server/tls_connection.h#L57)

<a id="alias-shared-state"></a>

## shared_state

🟠 **ALIAS** · *transport::server::tls_connection*

```cpp
using shared_state = tls_context;
```

Alias for `tls_context`.

**Source:** [include/transport/server/tls_connection.h:61](../../../../include/transport/server/tls_connection.h#L61)

<a id="constructor-tls-connection"></a>

## tls_connection

🟠 **CONSTRUCTOR** · *transport::server::tls_connection*

```cpp
template <protocol::contracts::engine_factory<ENty> FAty>
  tls_connection(std::size_t receive_buffer_capacity,
                 std::size_t send_buffer_size,
                 const FAty& create_engine, shared_state context);
```

Constructs a tls_connection value.

**Template parameters:**

- **FAty:** `protocol::contracts::engine_factory<ENty> FAty`.

**Parameters:**

- **receive_buffer_capacity:** Initial value supplied to the constructor. Declaration: `std::size_t receive_buffer_capacity`.
- **send_buffer_size:** Size or length limit for the named data. Declaration: `std::size_t send_buffer_size`.
- **create_engine:** Initial value supplied to the constructor. Declaration: `const FAty& create_engine`.
- **context:** Context for this operation. Declaration: `shared_state context`.

**Source:** [include/transport/server/tls_connection.h:66](../../../../include/transport/server/tls_connection.h#L66)

<a id="constructor-tls-connection-2"></a>

## tls_connection

🟠 **CONSTRUCTOR** · *transport::server::tls_connection*

```cpp
template <protocol::contracts::engine_factory<ENty> FAty>
  tls_connection(std::size_t receive_buffer_capacity,
                 std::size_t send_buffer_size,
                 const FAty& create_engine, shared_state context,
                 std::size_t encrypted_receive_buffer_capacity,
                 std::size_t network_bio_buffer_capacity);
```

Constructs a tls_connection value.

**Template parameters:**

- **FAty:** `protocol::contracts::engine_factory<ENty> FAty`.

**Parameters:**

- **receive_buffer_capacity:** Initial value supplied to the constructor. Declaration: `std::size_t receive_buffer_capacity`.
- **send_buffer_size:** Size or length limit for the named data. Declaration: `std::size_t send_buffer_size`.
- **create_engine:** Initial value supplied to the constructor. Declaration: `const FAty& create_engine`.
- **context:** Context for this operation. Declaration: `shared_state context`.
- **encrypted_receive_buffer_capacity:** Initial value supplied to the constructor. Declaration: `std::size_t encrypted_receive_buffer_capacity`.
- **network_bio_buffer_capacity:** Initial value supplied to the constructor. Declaration: `std::size_t network_bio_buffer_capacity`.

**Source:** [include/transport/server/tls_connection.h:74](../../../../include/transport/server/tls_connection.h#L74)

<a id="method-receive-buffer"></a>

## receive_buffer

🟠 **METHOD** · *transport::server::tls_connection*

```cpp
std::span<char> receive_buffer() noexcept;
```

Reads data from the encrypted buffer and passes it to the engine. Only TLS retains encrypted leftovers. If the TLS handshake fails or the connection closes, it throws an exception.

**Returns:** A `std::span<char>` view of the selected buffer.

**Errors:** Declared `noexcept`.

**Source:** [include/transport/server/tls_connection.h:93](../../../../include/transport/server/tls_connection.h#L93)

<a id="method-process"></a>

## process

🟠 **METHOD** · *transport::server::tls_connection*

```cpp
void process(std::size_t count);
```

Processes pending connection input and output.

**Parameters:**

- **count:** Number of elements or bytes to process. Declaration: `std::size_t count`.

**Returns:** No value (`void`).

**Errors:** Directly throws `std::out_of_range`, `std::runtime_error`. Calls made by this operation may propagate additional errors.

**Source:** [include/transport/server/tls_connection.h:96](../../../../include/transport/server/tls_connection.h#L96)

<a id="method-prepare-output"></a>

## prepare_output

🟠 **METHOD** · *transport::server::tls_connection*

```cpp
bool prepare_output(send_state& output);
```

Gets the output buffer ready to send. It processes any pending TLS data, handles a requested TLS shutdown, and fills the buffer with encrypted data. It returns false if anything goes wrong; otherwise, it returns true.

**Parameters:**

- **output:** Destination or output callback, as specified by its type. Declaration: `send_state& output`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/tls_connection.h:125](../../../../include/transport/server/tls_connection.h#L125)

<a id="method-output-bytes"></a>

## output_bytes

🟠 **METHOD** · *transport::server::tls_connection*

```cpp
std::span<char> output_bytes(send_state&);
```

Returns a span of encrypted data that’s ready to send. It uses `send_state` to find the current output buffer and offset.

**Parameters:**

- **send_state:** State for the current send operation. Declaration: `send_state&`.

**Returns:** A `std::span<char>` view of the selected buffer.

**Source:** [include/transport/server/tls_connection.h:161](../../../../include/transport/server/tls_connection.h#L161)

<a id="method-output-buffers"></a>

## output_buffers

🟠 **METHOD** · *transport::server::tls_connection*

```cpp
std::size_t output_buffers(send_state& output,
                             std::span<std::span<char>> buffers);
```

Fills the given buffers with encrypted data that’s ready to send. It returns the number of buffers filled, or 0 if there’s nothing to send. It uses `send_state` to find the current output buffer and offset.

**Parameters:**

- **output:** Destination or output callback, as specified by its type. Declaration: `send_state& output`.
- **buffers:** Buffers used for input or output. Declaration: `std::span<std::span<char>> buffers`.

**Returns:** The number of output buffers prepared.

**Source:** [include/transport/server/tls_connection.h:170](../../../../include/transport/server/tls_connection.h#L170)

<a id="method-output-sent"></a>

## output_sent

🟠 **METHOD** · *transport::server::tls_connection*

```cpp
bool output_sent(send_state&, std::size_t sent);
```

Tells the connection how many bytes were sent. It updates the TLS session and returns true.

**Parameters:**

- **send_state:** State for the current send operation. Declaration: `send_state&`.
- **sent:** Number of bytes sent. Declaration: `std::size_t sent`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/tls_connection.h:182](../../../../include/transport/server/tls_connection.h#L182)

<a id="method-output-pending"></a>

## output_pending

🟠 **METHOD** · *transport::server::tls_connection*

```cpp
bool output_pending(const send_state& output) const;
```

Returns true if there’s still data to send in either the TLS session or the output buffer. It uses `send_state` to find the current output buffer and offset.

**Parameters:**

- **output:** Destination or output callback, as specified by its type. Declaration: `const send_state& output`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/tls_connection.h:193](../../../../include/transport/server/tls_connection.h#L193)

<a id="method-established"></a>

## established

🟠 **METHOD** · *transport::server::tls_connection*

```cpp
bool established() const;
```

Reports whether the connection is established.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/tls_connection.h:201](../../../../include/transport/server/tls_connection.h#L201)

<a id="method-peer-closed"></a>

## peer_closed

🟠 **METHOD** · *transport::server::tls_connection*

```cpp
bool peer_closed() const;
```

Reports whether the remote peer closed the connection.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/tls_connection.h:205](../../../../include/transport/server/tls_connection.h#L205)

<a id="method-eof"></a>

## eof

🟠 **METHOD** · *transport::server::tls_connection*

```cpp
bool eof() const;
```

Reports whether the eof condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/tls_connection.h:209](../../../../include/transport/server/tls_connection.h#L209)

<a id="method-close-output"></a>

## close_output

🟠 **METHOD** · *transport::server::tls_connection*

```cpp
bool close_output();
```

Closes output.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/tls_connection.h:213](../../../../include/transport/server/tls_connection.h#L213)

<a id="field-engine"></a>

## engine

🟠 **FIELD** · *transport::server::tls_connection*

```cpp
ENty engine;
```

Stores the engine value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/transport/server/tls_connection.h:221](../../../../include/transport/server/tls_connection.h#L221)

<a id="field-buffer"></a>

## buffer

🟠 **FIELD** · *transport::server::tls_connection*

```cpp
std::unique_ptr<char[]> buffer;
```

Stores the buffer value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/transport/server/tls_connection.h:222](../../../../include/transport/server/tls_connection.h#L222)

<a id="field-capacity"></a>

## capacity

🟠 **FIELD** · *transport::server::tls_connection*

```cpp
const std::size_t capacity;
```

Stores the capacity value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/transport/server/tls_connection.h:223](../../../../include/transport/server/tls_connection.h#L223)

<a id="field-size"></a>

## size

🟠 **FIELD** · *transport::server::tls_connection*

```cpp
std::size_t size{0};
```

Stores the size value.

**Value:** `{0}`

**Source:** [include/transport/server/tls_connection.h:224](../../../../include/transport/server/tls_connection.h#L224)
