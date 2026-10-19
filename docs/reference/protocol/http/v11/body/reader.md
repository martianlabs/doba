<a id="reader-h"></a>
<h1>
  <a href="../../../../../../resources/docs/reference/protocol/http/v11/body/reader/index.svg#gh-light-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/v11/body/reader/index.svg" alt="Index for reader.h">
  </a>
  <a href="../../../../../../resources/docs/reference/protocol/http/v11/body/reader/index-dark.svg#gh-dark-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/v11/body/reader/index-dark.svg" alt="Index for reader.h">
  </a>
</h1>

**Header:** [include/protocol/http/v11/body/reader.h](../../../../../../include/protocol/http/v11/body/reader.h)

Holds a borrowed buffer or an owned file with the body decoder. (reader_chunked or reader_raw) matching the encoding actually used by the request (Transfer-Encoding: chunked vs Content-Length). This lets callers pull already-decoded payload bytes via read() without ever having to know which framing produced them.

**protocol::http::v11::body**

- [reader](#type-reader) · *type*

**protocol::http::v11::body::reader**

- [chunked(source, extension_limit, trailer_limit)](#method-chunked) · *method*
- [chunked(source, extension_limit, trailer_limit)](#method-chunked-2) · *method*
- [raw(source, content_length)](#method-raw) · *method*
- [raw(source, content_length)](#method-raw-2) · *method*
- [read](#method-read) · *method*
- [take_borrowed_raw](#method-take-borrowed-raw) · *method*
- [raw(source, length)](#method-raw-3) · *method*
- [chunked(source, extension_limit, trailer_limit)](#method-chunked-3) · *method*
- [rebind_borrowed](#method-rebind-borrowed) · *method*

<a id="type-reader"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> reader

**TYPE** · *protocol::http::v11::body*

**Declaration synopsis:**

```cpp
class reader { ... };
```

Holds a borrowed buffer or an owned file with the body decoder. (reader_chunked or reader_raw) matching the encoding actually used by the request (Transfer-Encoding: chunked vs Content-Length). This lets callers pull already-decoded payload bytes via read() without ever having to know which framing produced them.

**Source:** [include/protocol/http/v11/body/reader.h:52](../../../../../../include/protocol/http/v11/body/reader.h#L52)

<a id="method-chunked"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> chunked

**METHOD** · *protocol::http::v11::body::reader*

```cpp
static reader chunked(
      std::span<const std::byte> source,
      std::size_t extension_limit =
          reader_chunked::kDefaultMaxChunkExtensionSize,
      std::size_t trailer_limit =
          reader_chunked::kDefaultMaxTrailerSectionSize);
```

Creates a reader for a chunked Transfer-Encoding from a borrowed span source. The extension_limit and trailer_limit parameters set the maximum number of bytes allowed for chunk extensions and trailers, respectively. A limit of zero means no limit.

**Parameters:**

- **source:** Source data or object to read from. Declaration: `std::span<const std::byte> source`.
- **extension_limit:** Maximum chunk-extension size in bytes. Declaration: `std::size_t extension_limit = reader_chunked::kDefaultMaxChunkExtensionSize`.
- **trailer_limit:** Maximum trailer size in bytes. Declaration: `std::size_t trailer_limit = reader_chunked::kDefaultMaxTrailerSectionSize`.

**Returns:** A `reader` configured for the selected body format.

**Errors:** Directly throws `std::invalid_argument`. Calls made by this operation may propagate additional errors.

**Source:** [include/protocol/http/v11/body/reader.h:72](../../../../../../include/protocol/http/v11/body/reader.h#L72)

<a id="method-chunked-2"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> chunked

**METHOD** · *protocol::http::v11::body::reader*

```cpp
static reader chunked(
      common::filesystem_file&& source,
      std::size_t extension_limit =
          reader_chunked::kDefaultMaxChunkExtensionSize,
      std::size_t trailer_limit =
          reader_chunked::kDefaultMaxTrailerSectionSize);
```

Creates a reader for a chunked Transfer-Encoding from a file source. The extension_limit and trailer_limit parameters set the maximum number of bytes allowed for chunk extensions and trailers, respectively. A limit of zero means no limit.

**Parameters:**

- **source:** Source data or object to read from. Declaration: `common::filesystem_file&& source`.
- **extension_limit:** Maximum chunk-extension size in bytes. Declaration: `std::size_t extension_limit = reader_chunked::kDefaultMaxChunkExtensionSize`.
- **trailer_limit:** Maximum trailer size in bytes. Declaration: `std::size_t trailer_limit = reader_chunked::kDefaultMaxTrailerSectionSize`.

**Returns:** A `reader` configured for the selected body format.

**Errors:** Directly throws `std::invalid_argument`. Calls made by this operation may propagate additional errors.

**Source:** [include/protocol/http/v11/body/reader.h:90](../../../../../../include/protocol/http/v11/body/reader.h#L90)

<a id="method-raw"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> raw

**METHOD** · *protocol::http::v11::body::reader*

```cpp
static reader raw(std::span<const std::byte> source,
                    std::size_t content_length);
```

Creates a reader for a raw Transfer-Encoding from a borrowed span source. The content_length parameter sets the maximum number of bytes allowed for the raw content.

**Parameters:**

- **source:** Source data or object to read from. Declaration: `std::span<const std::byte> source`.
- **content_length:** Size or length limit for the named data. Declaration: `std::size_t content_length`.

**Returns:** A `reader` configured for the selected body format.

**Errors:** Directly throws `std::invalid_argument`. Calls made by this operation may propagate additional errors.

**Source:** [include/protocol/http/v11/body/reader.h:107](../../../../../../include/protocol/http/v11/body/reader.h#L107)

<a id="method-raw-2"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> raw

**METHOD** · *protocol::http::v11::body::reader*

```cpp
static reader raw(common::filesystem_file&& source,
                    std::size_t content_length);
```

Creates a reader for a raw Transfer-Encoding from a file source. The content_length parameter sets the maximum number of bytes allowed for the raw content.

**Parameters:**

- **source:** Source data or object to read from. Declaration: `common::filesystem_file&& source`.
- **content_length:** Size or length limit for the named data. Declaration: `std::size_t content_length`.

**Returns:** A `reader` configured for the selected body format.

**Errors:** Directly throws `std::invalid_argument`. Calls made by this operation may propagate additional errors.

**Source:** [include/protocol/http/v11/body/reader.h:120](../../../../../../include/protocol/http/v11/body/reader.h#L120)

<a id="method-read"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> read

**METHOD** · *protocol::http::v11::body::reader*

```cpp
reader_state read(std::span<std::byte> output);
```

Pulls wire bytes from the owned source, decodes them using the encoding selected at construction time, and writes the decoded payload into output. See reader_chunked::read/reader_raw::read for semantics.

**Parameters:**

- **output:** Destination or output callback, as specified by its type. Declaration: `std::span<std::byte> output`.

**Returns:** A `reader_state` describing progress, completion, and errors.

**Source:** [include/protocol/http/v11/body/reader.h:133](../../../../../../include/protocol/http/v11/body/reader.h#L133)

<a id="method-take-borrowed-raw"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> take_borrowed_raw

**METHOD** · *protocol::http::v11::body::reader*

```cpp
std::optional<std::span<const std::byte>> take_borrowed_raw(
      std::size_t maximum);
```

Creates a raw reader over borrowed bytes.

**Parameters:**

- **maximum:** Maximum allowed size or count. Declaration: `std::size_t maximum`.

**Returns:** A `std::optional<std::span<const std::byte>>` containing the result when present; otherwise `std::nullopt`.

**Source:** [include/protocol/http/v11/body/reader.h:141](../../../../../../include/protocol/http/v11/body/reader.h#L141)

<a id="method-raw-3"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> raw

**METHOD** · *protocol::http::v11::body::reader*

```cpp
static reader raw(common::byte_storage source, std::size_t length);
```

Takes ownership of wire bytes, including any temporary backing file.

**Parameters:**

- **source:** Source data or object to read from. Declaration: `common::byte_storage source`.
- **length:** Length in elements or bytes. Declaration: `std::size_t length`.

**Returns:** A `reader` configured for the selected body format.

**Source:** [include/protocol/http/v11/body/reader.h:149](../../../../../../include/protocol/http/v11/body/reader.h#L149)

<a id="method-chunked-3"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> chunked

**METHOD** · *protocol::http::v11::body::reader*

```cpp
static reader chunked(common::byte_storage source,
                        std::size_t extension_limit,
                        std::size_t trailer_limit);
```

Creates a reader or writer for chunked transfer encoding.

**Parameters:**

- **source:** Source data or object to read from. Declaration: `common::byte_storage source`.
- **extension_limit:** Maximum chunk-extension size in bytes. Declaration: `std::size_t extension_limit`.
- **trailer_limit:** Maximum trailer size in bytes. Declaration: `std::size_t trailer_limit`.

**Returns:** A `reader` configured for the selected body format.

**Source:** [include/protocol/http/v11/body/reader.h:152](../../../../../../include/protocol/http/v11/body/reader.h#L152)

<a id="method-rebind-borrowed"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> rebind_borrowed

**METHOD** · *protocol::http::v11::body::reader*

```cpp
void rebind_borrowed(std::span<const std::byte> source) noexcept;
```

Points borrowed byte views at the replacement buffer.

**Parameters:**

- **source:** Source data or object to read from. Declaration: `std::span<const std::byte> source`.

**Returns:** No value (`void`).

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/v11/body/reader.h:159](../../../../../../include/protocol/http/v11/body/reader.h#L159)
