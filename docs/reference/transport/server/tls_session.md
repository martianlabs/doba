<a id="tls-session"></a>
<a href="../../../../resources/docs/reference/transport/server/tls_session/index.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tls_session/index.svg" alt="tls_session">
</a>
<a href="../../../../resources/docs/reference/transport/server/tls_session/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tls_session/index-dark.svg" alt="tls_session">
</a>

**Header:** [include/transport/server/tls_session.h](../../../../include/transport/server/tls_session.h)

This class manages the TLS session for a single connection. It handles the handshake, encrypts and decrypts data, and manages the encrypted and plaintext buffers. It also lets you read and write data, check the session state, and shut it down cleanly.

### <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transport::server

[tls_context](#alias-tls-context) ·
[make_tls_context](#function-make-tls-context) ·
[tls_session](#type-tls-session)

### <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transport::server::tls_session

[result](result.md) ·
[status](#enum-status) ·
[handshake](#method-handshake) ·
[receive](#method-receive) ·
[read](#method-read) ·
[write](#method-write) ·
[shutdown](#method-shutdown) ·
[drain](#method-drain) ·
[output_bytes](#method-output-bytes) ·
[output_sent](#method-output-sent) ·
[pending](#method-pending) ·
[established](#method-established) ·
[failed](#method-failed)

<a id="alias-tls-context"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> tls_context

**ALIAS** · *transport::server*

```cpp
using tls_context = std::shared_ptr<SSL_CTX>;
```

Alias for `std::shared_ptr<SSL_CTX>`.

**Source:** [include/transport/server/tls_session.h:45](../../../../include/transport/server/tls_session.h#L45)

<a id="function-make-tls-context"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> make_tls_context

**FUNCTION** · *transport::server*

```cpp
inline tls_context make_tls_context(const tls_policies& configuration);
```

Creates tls context.

**Parameters:**

- **configuration:** Configuration for this component. Declaration: `const tls_policies& configuration`.

**Returns:** A value of type `tls_context`.

**Errors:** Directly throws `std::runtime_error`. Calls made by this operation may propagate additional errors.

**Source:** [include/transport/server/tls_session.h:52](../../../../include/transport/server/tls_session.h#L52)

<a id="type-tls-session"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> tls_session

**TYPE** · *transport::server*

**Declaration synopsis:**

```cpp
class tls_session { ... };
```

This class manages the TLS session for a single connection. It handles the handshake, encrypts and decrypts data, and manages the encrypted and plaintext buffers. It also lets you read and write data, check the session state, and shut it down cleanly.

**Source:** [include/transport/server/tls_session.h:89](../../../../include/transport/server/tls_session.h#L89)

<a id="enum-status"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> status

**ENUM** · *transport::server::tls_session*

```cpp
enum class status { ready, need_input, need_output, closed, failed };
```

Enumerates status values.

**Values:**

- **ready:** Ready. Declaration: `ready`.
- **need_input:** Need input. Declaration: `need_input`.
- **need_output:** Need output. Declaration: `need_output`.
- **closed:** Closed. Declaration: `closed`.
- **failed:** Failed. Declaration: `failed`.

**Source:** [include/transport/server/tls_session.h:94](../../../../include/transport/server/tls_session.h#L94)

<a id="method-handshake"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> handshake

**METHOD** · *transport::server::tls_session*

```cpp
status handshake();
```

Runs the TLS handshake and returns the session status. If it fails, it returns `failed`. If it needs more data to read or send, it returns `need_input` or `need_output`. If the handshake succeeds, it returns `ready`.

**Returns:** A value of type `status`.

**Source:** [include/transport/server/tls_session.h:132](../../../../include/transport/server/tls_session.h#L132)

<a id="method-receive"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> receive

**METHOD** · *transport::server::tls_session*

```cpp
std::size_t receive(std::span<const char> bytes);
```

Receives encrypted data from the network and passes it to the TLS session. Returns the number of bytes received, or 0 if the session fails.

**Parameters:**

- **bytes:** Byte sequence or byte count for this operation. Declaration: `std::span<const char> bytes`.

**Returns:** The number of bytes processed.

**Source:** [include/transport/server/tls_session.h:150](../../../../include/transport/server/tls_session.h#L150)

<a id="method-read"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> read

**METHOD** · *transport::server::tls_session*

```cpp
result read(std::span<char> bytes);
```

Reads decrypted data from the TLS session into the given buffer. It returns a `result` with the session status and number of bytes read. If the session fails, it returns `failed`. If it needs more data to read or send, it returns `need_input` or `need_output`. If all goes well, it returns `ready`.

**Parameters:**

- **bytes:** Byte sequence or byte count for this operation. Declaration: `std::span<char> bytes`.

**Returns:** A value of type `result`.

**Source:** [include/transport/server/tls_session.h:170](../../../../include/transport/server/tls_session.h#L170)

<a id="method-write"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> write

**METHOD** · *transport::server::tls_session*

```cpp
result write(std::span<const char> bytes);
```

Writes plaintext data to the TLS session to be encrypted. It returns a `result` with the session status and number of bytes written. If the session fails, it returns `failed`. If it needs more data to read or send, it returns `need_input` or `need_output`. Otherwise, it returns `ready`.

**Parameters:**

- **bytes:** Byte sequence or byte count for this operation. Declaration: `std::span<const char> bytes`.

**Returns:** A value of type `result`.

**Source:** [include/transport/server/tls_session.h:194](../../../../include/transport/server/tls_session.h#L194)

<a id="method-shutdown"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> shutdown

**METHOD** · *transport::server::tls_session*

```cpp
status shutdown();
```

Shuts down the TLS session cleanly. Returns `ready` when it’s done, `need_output` if there’s more data to send, or `failed` if shutdown doesn’t work.

**Returns:** A value of type `status`.

**Source:** [include/transport/server/tls_session.h:235](../../../../include/transport/server/tls_session.h#L235)

<a id="method-drain"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> drain

**METHOD** · *transport::server::tls_session*

```cpp
std::size_t drain(std::span<char> bytes);
```

Shuts down the TLS session cleanly. Returns `ready` when it’s done, `need_output` if there’s more data to send, or `failed` if shutdown doesn’t work.

**Parameters:**

- **bytes:** Byte sequence or byte count for this operation. Declaration: `std::span<char> bytes`.

**Returns:** The number of bytes processed.

**Source:** [include/transport/server/tls_session.h:256](../../../../include/transport/server/tls_session.h#L256)

<a id="method-output-bytes"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> output_bytes

**METHOD** · *transport::server::tls_session*

```cpp
std::span<char> output_bytes();
```

Returns a span of encrypted data that’s ready to send. It uses the network BIO to find the current output buffer and offset.

**Returns:** A `std::span<char>` view of the selected buffer.

**Source:** [include/transport/server/tls_session.h:273](../../../../include/transport/server/tls_session.h#L273)

<a id="method-output-sent"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> output_sent

**METHOD** · *transport::server::tls_session*

```cpp
void output_sent(std::size_t size);
```

Tells the session how many bytes were sent. It updates the network BIO and returns true.

**Parameters:**

- **size:** Number of elements or bytes, according to the declared type. Declaration: `std::size_t size`.

**Returns:** No value (`void`).

**Source:** [include/transport/server/tls_session.h:286](../../../../include/transport/server/tls_session.h#L286)

<a id="method-pending"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> pending

**METHOD** · *transport::server::tls_session*

```cpp
std::size_t pending() const;
```

Returns the number of bytes pending in the network BIO.

**Returns:** The pending count or size.

**Source:** [include/transport/server/tls_session.h:296](../../../../include/transport/server/tls_session.h#L296)

<a id="method-established"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> established

**METHOD** · *transport::server::tls_session*

```cpp
bool established() const;
```

Returns true if the TLS session is established and ready for data.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/tls_session.h:305](../../../../include/transport/server/tls_session.h#L305)

<a id="method-failed"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> failed

**METHOD** · *transport::server::tls_session*

```cpp
bool failed() const;
```

Returns true if the TLS session has failed.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/transport/server/tls_session.h:314](../../../../include/transport/server/tls_session.h#L314)
