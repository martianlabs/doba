<a id="send-state-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../resources/docs/reference/transport/server/send_state/index-dark.svg">
    <img src="../../../../resources/docs/reference/transport/server/send_state/index.svg" alt="Index for send_state.h">
  </picture>
</h1>

**Header:** [include/transport/server/send_state.h](../../../../include/transport/server/send_state.h)

This struct manages the state of outgoing data for a TCP connection. It maintains a buffer for inline data and a queue of sources for additional data to be sent. It provides methods to push new data, fill the buffer, consume sent data, and check if there is pending data to be sent.

**transport::server**

- [send_state](#type-send-state) · *type*

**transport::server::send_state**

- [source_entry](#type-source-entry) · *type*

**transport::server::send_state::source_entry**

- [prefix](#field-prefix) · *field*
- [source](#field-source) · *field*

**transport::server::send_state**

- [send_state](#constructor-send-state) · *constructor*
- [push](#method-push) · *method*
- [fill](#method-fill) · *method*
- [consume](#method-consume) · *method*
- [queued](#method-queued) · *method*
- [clear](#method-clear) · *method*
- [buffer](#field-buffer) · *field*
- [offset](#field-offset) · *field*

<a id="type-send-state"></a>

## send_state

🟠 **TYPE** · *transport::server*

**Declaration synopsis:**

```cpp
struct send_state { ... };
```

This struct manages the state of outgoing data for a TCP connection. It maintains a buffer for inline data and a queue of sources for additional data to be sent. It provides methods to push new data, fill the buffer, consume sent data, and check if there is pending data to be sent.

**Source:** [include/transport/server/send_state.h:53](../../../../include/transport/server/send_state.h#L53)

<a id="type-source-entry"></a>

## source_entry

🟠 **TYPE** · *transport::server::send_state*

**Declaration synopsis:**

```cpp
struct source_entry { ... };
```

Provides the source entry transport component.

**Source:** [include/transport/server/send_state.h:57](../../../../include/transport/server/send_state.h#L57)

<a id="field-prefix"></a>

## prefix

🟠 **FIELD** · *transport::server::send_state::source_entry*

```cpp
std::string prefix;
```

Prefix data to be sent before reading from the source (memory based)

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/transport/server/send_state.h:59](../../../../include/transport/server/send_state.h#L59)

<a id="field-source"></a>

## source

🟠 **FIELD** · *transport::server::send_state::source_entry*

```cpp
std::unique_ptr<common::reader> source;
```

Source reader for additional data (stream based)

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/transport/server/send_state.h:61](../../../../include/transport/server/send_state.h#L61)

<a id="constructor-send-state"></a>

## send_state

🟠 **CONSTRUCTOR** · *transport::server::send_state*

```cpp
explicit send_state(std::size_t capacity);
```

Constructs a send_state value.

**Parameters:**

- **capacity:** Available capacity in the destination. Declaration: `std::size_t capacity`.

**Source:** [include/transport/server/send_state.h:66](../../../../include/transport/server/send_state.h#L66)

<a id="method-push"></a>

## push

🟠 **METHOD** · *transport::server::send_state*

```cpp
bool push(std::string_view head, std::string_view body,
            std::unique_ptr<common::reader> source);
```

Tries to add a new data entry to the send state. It checks whether there is enough room for the head, body, and any space reserved for sources. If everything fits, it adds the head and body to the pending buffer and puts the source in the sources queue, if one was provided. Returns true if the push succeeds, or false if it doesn’t.

**Parameters:**

- **head:** HTTP head data or its output buffer. Declaration: `std::string_view head`.
- **body:** HTTP body data or its output buffer. Declaration: `std::string_view body`.
- **source:** Source data or object to read from. Declaration: `std::unique_ptr<common::reader> source`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/send_state.h:77](../../../../include/transport/server/send_state.h#L77)

<a id="method-fill"></a>

## fill

🟠 **METHOD** · *transport::server::send_state*

```cpp
bool fill();
```

Fills the buffer with data from the pending buffer or the sources queue. First, it clears the current buffer and resets the offset. If there are no sources, it swaps the pending buffer into the main buffer and returns true. Otherwise, it checks the first source for a prefix to send. If it finds one, it swaps it into the main buffer and returns

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/send_state.h:112](../../../../include/transport/server/send_state.h#L112)

<a id="method-consume"></a>

## consume

🟠 **METHOD** · *transport::server::send_state*

```cpp
bool consume(std::size_t sent);
```

Removes the requested number of bytes from the buffer. If that’s more than the number of bytes left, it returns false. Otherwise, it moves the offset forward. If the data came from the pending buffer rather than an external source, it also reduces the inline byte count. Returns true when the bytes have been consumed successfully.

**Parameters:**

- **sent:** Number of bytes sent. Declaration: `std::size_t sent`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/send_state.h:155](../../../../include/transport/server/send_state.h#L155)

<a id="method-queued"></a>

## queued

🟠 **METHOD** · *transport::server::send_state*

```cpp
bool queued() const;
```

Checks whether there’s any data waiting to be sent. Returns true if the pending buffer or the sources queue contains data.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/send_state.h:167](../../../../include/transport/server/send_state.h#L167)

<a id="method-clear"></a>

## clear

🟠 **METHOD** · *transport::server::send_state*

```cpp
void clear();
```

Clears the send state by removing all pending data and resetting the buffer, offset, inline byte count, source reservation, and active source flag.

**Returns:** No value (`void`).

**Source:** [include/transport/server/send_state.h:175](../../../../include/transport/server/send_state.h#L175)

<a id="field-buffer"></a>

## buffer

🟠 **FIELD** · *transport::server::send_state*

```cpp
std::string buffer;
```

Stores the buffer value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/transport/server/send_state.h:187](../../../../include/transport/server/send_state.h#L187)

<a id="field-offset"></a>

## offset

🟠 **FIELD** · *transport::server::send_state*

```cpp
std::size_t offset{0};
```

Stores the offset value.

**Value:** `{0}`

**Source:** [include/transport/server/send_state.h:188](../../../../include/transport/server/send_state.h#L188)
