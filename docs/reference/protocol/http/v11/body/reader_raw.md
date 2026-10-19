<a id="reader-raw"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/body/reader_raw/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/body/reader_raw/index.svg" alt="reader_raw">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/body/reader_raw/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/body/reader_raw/index-dark.svg" alt="reader_raw">
</a>

**Header:** [include/protocol/http/v11/body/reader_raw.h](../../../../../../include/protocol/http/v11/body/reader_raw.h)

Decodes a Content-Length-framed body by pulling wire bytes from a common::reader source and filling the caller-supplied output span.

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::body

[reader_raw](#type-reader-raw)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::body::reader_raw

[read](#method-read) ·
[take_view](#method-take-view)

<a id="type-reader-raw"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> reader_raw

**TYPE** · *protocol::http::v11::body*

**Declaration synopsis:**

```cpp
class reader_raw { ... };
```

Decodes a Content-Length-framed body by pulling wire bytes from a common::reader source and filling the caller-supplied output span.

**Source:** [include/protocol/http/v11/body/reader_raw.h:55](../../../../../../include/protocol/http/v11/body/reader_raw.h#L55)

<a id="method-read"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> read

**METHOD** · *protocol::http::v11::body::reader_raw*

```cpp
reader_state read(common::reader& src, std::span<std::byte> output);
```

Pulls up to (expected_ - accumulated_) bytes from src into output. Returns immediately with complete=true when Content-Length is reached. A zero Content-Length body completes on the first call with produced=0.

**Parameters:**

- **src:** Source reader or byte range. Declaration: `common::reader& src`.
- **output:** Destination or output callback, as specified by its type. Declaration: `std::span<std::byte> output`.

**Returns:** A `reader_state` describing progress, completion, and errors.

**Source:** [include/protocol/http/v11/body/reader_raw.h:68](../../../../../../include/protocol/http/v11/body/reader_raw.h#L68)

<a id="method-take-view"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> take_view

**METHOD** · *protocol::http::v11::body::reader_raw*

```cpp
std::optional<std::span<const std::byte>> take_view(
      std::span<const std::byte> source, std::size_t maximum);
```

Takes the current non-owning byte view.

**Parameters:**

- **source:** Source data or object to read from. Declaration: `std::span<const std::byte> source`.
- **maximum:** Maximum allowed size or count. Declaration: `std::size_t maximum`.

**Returns:** A `std::optional<std::span<const std::byte>>` containing the result when present; otherwise `std::nullopt`.

**Source:** [include/protocol/http/v11/body/reader_raw.h:96](../../../../../../include/protocol/http/v11/body/reader_raw.h#L96)
