<a id="writer"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/body/writer/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/body/writer/title.svg" alt="writer">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/body/writer/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/body/writer/title-dark.svg" alt="writer">
</a>

**Header:** [include/protocol/http/v11/body/writer.h](../../../../../../include/protocol/http/v11/body/writer.h)

Owns the wire-level common::writer sink together with the body encoder (writer_chunked or writer_raw) matching the framing chosen for the outgoing response (Transfer-Encoding: chunked vs Content-Length). This lets callers push raw payload bytes via write() without ever having to know which framing will encode them, mirroring body::reader on the

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::body

[body_writer](#type-body-writer)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::body::body_writer

[chunked](#method-chunked) ·
[raw](#method-raw) ·
[write(payload)](#method-write) ·
[write(payload)](#method-write-2) ·
[end](#method-end) ·
[release](#method-release) ·
[is_chunked](#method-is-chunked) ·
[bytes_written](#method-bytes-written)

<a id="api"></a>
<a href="../../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-body-writer"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> body_writer

**TYPE** · *protocol::http::v11::body*

**Declaration synopsis:**

```cpp
class body_writer { ... };
```

Owns the wire-level common::writer sink together with the body encoder (writer_chunked or writer_raw) matching the framing chosen for the outgoing response (Transfer-Encoding: chunked vs Content-Length). This lets callers push raw payload bytes via write() without ever having to know which framing will encode them, mirroring body::reader on the

**Source:** [include/protocol/http/v11/body/writer.h:52](../../../../../../include/protocol/http/v11/body/writer.h#L52)

<a id="method-chunked"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> chunked

**METHOD** · *protocol::http::v11::body::body_writer*

```cpp
static body_writer chunked(common::byte_storage_options opts = {});
```

Creates a reader or writer for chunked transfer encoding.

**Parameters:**

- **opts:** Storage options; defaults to their default values. Declaration: `common::byte_storage_options opts = {}`.

**Returns:** A `body_writer` configured for the selected body format.

**Source:** [include/protocol/http/v11/body/writer.h:57](../../../../../../include/protocol/http/v11/body/writer.h#L57)

<a id="method-raw"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> raw

**METHOD** · *protocol::http::v11::body::body_writer*

```cpp
static body_writer raw(common::byte_storage_options opts = {});
```

Creates a reader or writer for an unchunked body.

**Parameters:**

- **opts:** Storage options; defaults to their default values. Declaration: `common::byte_storage_options opts = {}`.

**Returns:** A `body_writer` configured for the selected body format.

**Source:** [include/protocol/http/v11/body/writer.h:60](../../../../../../include/protocol/http/v11/body/writer.h#L60)

<a id="method-write"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> write

**METHOD** · *protocol::http::v11::body::body_writer*

```cpp
bool write(std::span<const std::byte> payload);
```

Encodes the caller-supplied raw payload using the framing selected at construction time and appends it into the owned sink.

**Parameters:**

- **payload:** Payload bytes to process or send. Declaration: `std::span<const std::byte> payload`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/body/writer.h:76](../../../../../../include/protocol/http/v11/body/writer.h#L76)

<a id="method-write-2"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> write

**METHOD** · *protocol::http::v11::body::body_writer*

```cpp
bool write(std::string_view payload);
```

Writes data.

**Parameters:**

- **payload:** Payload bytes to process or send. Declaration: `std::string_view payload`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/body/writer.h:90](../../../../../../include/protocol/http/v11/body/writer.h#L90)

<a id="method-end"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> end

**METHOD** · *protocol::http::v11::body::body_writer*

```cpp
bool end();
```

Finalizes the body framing (emits the terminating chunk for chunked encoding; a no-op for raw). Idempotent.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/body/writer.h:107](../../../../../../include/protocol/http/v11/body/writer.h#L107)

<a id="method-release"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> release

**METHOD** · *protocol::http::v11::body::body_writer*

```cpp
[[nodiscard]] common::byte_storage release();
```

Finalizes the framing and the underlying storage, then hands over the accumulated (possibly spilled to disk) body bytes to the caller.

**Returns:** A value of type `common::byte_storage`.

**Source:** [include/protocol/http/v11/body/writer.h:117](../../../../../../include/protocol/http/v11/body/writer.h#L117)

<a id="method-is-chunked"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_chunked

**METHOD** · *protocol::http::v11::body::body_writer*

```cpp
[[nodiscard]] bool is_chunked() const noexcept;
```

Reports whether this instance was constructed via chunked() (as opposed to raw()). Lets callers that receive an already-built/used body_writer (e.g. response::set_body) pick the matching wire framing headers.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/v11/body/writer.h:128](../../../../../../include/protocol/http/v11/body/writer.h#L128)

<a id="method-bytes-written"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> bytes_written

**METHOD** · *protocol::http::v11::body::body_writer*

```cpp
[[nodiscard]] std::size_t bytes_written() const noexcept;
```

Returns the total number of raw payload bytes passed to write() so far (i.e. before framing overhead). Lets callers that receive an already- used raw body_writer (e.g. response::set_body) derive Content-Length without tracking it themselves.

**Returns:** The bytes written count or size.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/v11/body/writer.h:139](../../../../../../include/protocol/http/v11/body/writer.h#L139)
