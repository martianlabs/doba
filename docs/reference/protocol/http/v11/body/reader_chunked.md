<a id="reader-chunked"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/body/reader_chunked/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/body/reader_chunked/index.svg" alt="reader_chunked">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/body/reader_chunked/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/body/reader_chunked/index-dark.svg" alt="reader_chunked">
</a>

**Header:** [include/protocol/http/v11/body/reader_chunked.h](../../../../../../include/protocol/http/v11/body/reader_chunked.h)

Decodes a chunked Transfer-Encoding body (as accumulated verbatim by framer_chunked) by pulling wire bytes on demand from a common::reader source and filling the caller-supplied output span.

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::body

[reader_chunked](#type-reader-chunked)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::body::reader_chunked

[kDefaultMaxChunkExtensionSize](#constant-kdefaultmaxchunkextensionsize) ·
[kDefaultMaxTrailerSectionSize](#constant-kdefaultmaxtrailersectionsize) ·
[read](#method-read)

<a id="type-reader-chunked"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> reader_chunked

**TYPE** · *protocol::http::v11::body*

**Declaration synopsis:**

```cpp
class reader_chunked { ... };
```

Decodes a chunked Transfer-Encoding body (as accumulated verbatim by framer_chunked) by pulling wire bytes on demand from a common::reader source and filling the caller-supplied output span.

**Source:** [include/protocol/http/v11/body/reader_chunked.h:62](../../../../../../include/protocol/http/v11/body/reader_chunked.h#L62)

<a id="constant-kdefaultmaxchunkextensionsize"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kDefaultMaxChunkExtensionSize

**CONSTANT** · *protocol::http::v11::body::reader_chunked*

```cpp
static constexpr std::size_t kDefaultMaxChunkExtensionSize =
      policies::kDefaultMaxChunkExtensionSize;
```

Defines the default max chunk extension size constant.

**Value:** `policies::kDefaultMaxChunkExtensionSize`

**Source:** [include/protocol/http/v11/body/reader_chunked.h:94](../../../../../../include/protocol/http/v11/body/reader_chunked.h#L94)

<a id="constant-kdefaultmaxtrailersectionsize"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kDefaultMaxTrailerSectionSize

**CONSTANT** · *protocol::http::v11::body::reader_chunked*

```cpp
static constexpr std::size_t kDefaultMaxTrailerSectionSize =
      policies::kDefaultMaxTrailerSectionSize;
```

Defines the default max trailer section size constant.

**Value:** `policies::kDefaultMaxTrailerSectionSize`

**Source:** [include/protocol/http/v11/body/reader_chunked.h:96](../../../../../../include/protocol/http/v11/body/reader_chunked.h#L96)

<a id="method-read"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> read

**METHOD** · *protocol::http::v11::body::reader_chunked*

```cpp
reader_state read(common::reader& src, std::span<std::byte> output);
```

Pulls wire bytes from src, walking the chunked framing, and writes only chunk-data bytes into output. Returns the number of decoded payload bytes produced and whether the body is complete (last-chunk + terminating CRLF seen).

**Parameters:**

- **src:** Source reader or byte range. Declaration: `common::reader& src`.
- **output:** Destination or output callback, as specified by its type. Declaration: `std::span<std::byte> output`.

**Returns:** A `reader_state` describing progress, completion, and errors.

**Source:** [include/protocol/http/v11/body/reader_chunked.h:112](../../../../../../include/protocol/http/v11/body/reader_chunked.h#L112)
