<a id="writer-raw-h"></a>
<h1>
  <a href="../../../../../../resources/docs/reference/protocol/http/v11/body/writer_raw/index.svg#gh-light-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/v11/body/writer_raw/index.svg" alt="Index for writer_raw.h">
  </a>
  <a href="../../../../../../resources/docs/reference/protocol/http/v11/body/writer_raw/index-dark.svg#gh-dark-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/v11/body/writer_raw/index-dark.svg" alt="Index for writer_raw.h">
  </a>
</h1>

**Header:** [include/protocol/http/v11/body/writer_raw.h](../../../../../../include/protocol/http/v11/body/writer_raw.h)

Encodes an outgoing Content-Length-framed body: the raw framing is a pure passthrough, so this simply forwards the caller-supplied payload bytes verbatim into the destination common::writer.

**protocol::http::v11::body**

- [writer_raw](#type-writer-raw) · *type*

**protocol::http::v11::body::writer_raw**

- [write(payload, dst)](#method-write) · *method*
- [write(payload, dst)](#method-write-2) · *method*
- [end](#method-end) · *method*

<a id="type-writer-raw"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> writer_raw

**TYPE** · *protocol::http::v11::body*

**Declaration synopsis:**

```cpp
class writer_raw { ... };
```

Encodes an outgoing Content-Length-framed body: the raw framing is a pure passthrough, so this simply forwards the caller-supplied payload bytes verbatim into the destination common::writer.

**Source:** [include/protocol/http/v11/body/writer_raw.h:43](../../../../../../include/protocol/http/v11/body/writer_raw.h#L43)

<a id="method-write"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> write

**METHOD** · *protocol::http::v11::body::writer_raw*

```cpp
static bool write(std::span<const std::byte> payload, common::writer& dst);
```

Writes the given raw payload bytes into dst, unchanged.

**Parameters:**

- **payload:** Payload bytes to process or send. Declaration: `std::span<const std::byte> payload`.
- **dst:** Destination for the result. Declaration: `common::writer& dst`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/body/writer_raw.h:50](../../../../../../include/protocol/http/v11/body/writer_raw.h#L50)

<a id="method-write-2"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> write

**METHOD** · *protocol::http::v11::body::writer_raw*

```cpp
static bool write(std::string_view payload, common::writer& dst);
```

Writes data.

**Parameters:**

- **payload:** Payload bytes to process or send. Declaration: `std::string_view payload`.
- **dst:** Destination for the result. Declaration: `common::writer& dst`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/body/writer_raw.h:56](../../../../../../include/protocol/http/v11/body/writer_raw.h#L56)

<a id="method-end"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> end

**METHOD** · *protocol::http::v11::body::writer_raw*

```cpp
static bool end(common::writer&);
```

Raw (Content-Length) framing has no terminating sequence; provided for API symmetry with writer_chunked so callers can treat both uniformly.

**Parameters:**

- **writer:** Writer to use or move into this object. Declaration: `common::writer&`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/body/writer_raw.h:65](../../../../../../include/protocol/http/v11/body/writer_raw.h#L65)
