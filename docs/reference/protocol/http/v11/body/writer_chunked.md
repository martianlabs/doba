<a id="writer-chunked"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/body/writer_chunked/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/body/writer_chunked/title.svg" alt="writer_chunked">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/body/writer_chunked/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/body/writer_chunked/title-dark.svg" alt="writer_chunked">
</a>

**Header:** [include/protocol/http/v11/body/writer_chunked.h](../../../../../../include/protocol/http/v11/body/writer_chunked.h)

Encodes an outgoing chunked Transfer-Encoding body. Each call to write() wraps the caller-supplied raw payload into a single chunk: <hex-size>\r\n<payload>\r\n A zero-length payload is skipped (it would otherwise be mistaken for the terminating chunk). Call end() once, after the last write(), to emit the terminating "0\r\n\r\n" sequence (no

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::body

[writer_chunked](#type-writer-chunked)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::body::writer_chunked

[write(payload, dst)](#method-write) ·
[write(payload, dst)](#method-write-2) ·
[end](#method-end)

<a id="api"></a>
<a href="../../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-writer-chunked"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> writer_chunked

**TYPE** · *protocol::http::v11::body*

**Declaration synopsis:**

```cpp
class writer_chunked { ... };
```

Encodes an outgoing chunked Transfer-Encoding body. Each call to write() wraps the caller-supplied raw payload into a single chunk: <hex-size>\r\n<payload>\r\n A zero-length payload is skipped (it would otherwise be mistaken for the terminating chunk). Call end() once, after the last write(), to emit the terminating "0\r\n\r\n" sequence (no

**Source:** [include/protocol/http/v11/body/writer_chunked.h:49](../../../../../../include/protocol/http/v11/body/writer_chunked.h#L49)

<a id="method-write"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> write

**METHOD** · *protocol::http::v11::body::writer_chunked*

```cpp
bool write(std::span<const std::byte> payload, common::writer& dst);
```

Encodes payload as a single chunk and writes it into dst.

**Parameters:**

- **payload:** Payload bytes to process or send. Declaration: `std::span<const std::byte> payload`.
- **dst:** Destination for the result. Declaration: `common::writer& dst`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/body/writer_chunked.h:56](../../../../../../include/protocol/http/v11/body/writer_chunked.h#L56)

<a id="method-write-2"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> write

**METHOD** · *protocol::http::v11::body::writer_chunked*

```cpp
bool write(std::string_view payload, common::writer& dst);
```

Writes data.

**Parameters:**

- **payload:** Payload bytes to process or send. Declaration: `std::string_view payload`.
- **dst:** Destination for the result. Declaration: `common::writer& dst`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/body/writer_chunked.h:66](../../../../../../include/protocol/http/v11/body/writer_chunked.h#L66)

<a id="method-end"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> end

**METHOD** · *protocol::http::v11::body::writer_chunked*

```cpp
bool end(common::writer& dst);
```

Emits the terminating chunk (no trailers). Idempotent: subsequent calls are no-ops.

**Parameters:**

- **dst:** Destination for the result. Declaration: `common::writer& dst`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/body/writer_chunked.h:78](../../../../../../include/protocol/http/v11/body/writer_chunked.h#L78)
