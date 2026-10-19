<a id="reader-error"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/body/reader_error/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/body/reader_error/index.svg" alt="reader_error">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/body/reader_error/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/body/reader_error/index-dark.svg" alt="reader_error">
</a>

**Header:** [include/protocol/http/v11/body/reader_error.h](../../../../../../include/protocol/http/v11/body/reader_error.h)

This header declares the types and operations listed below.

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::body

[reader_error](#enum-reader-error)

<a id="enum-reader-error"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> reader_error

**ENUM** · *protocol::http::v11::body*

```cpp
enum class reader_error : std::uint8_t {
  none,
  io_error,
  invalid_chunk_size,
  chunk_size_overflow,
  invalid_chunk_crlf,
  invalid_trailer,
  chunked_incomplete,
  raw_incomplete,
  raw_size_limit_exceeded,
  chunk_extension_size_limit_exceeded,
  trailer_size_limit_exceeded
};
```

Error codes shared by reader_raw and reader_chunked.

**Values:**

- **none:** None. Declaration: `none`.
- **io_error:** Io error. Declaration: `io_error`.
- **invalid_chunk_size:** Invalid chunk size. Declaration: `invalid_chunk_size`.
- **chunk_size_overflow:** Chunk size overflow. Declaration: `chunk_size_overflow`.
- **invalid_chunk_crlf:** Invalid chunk crlf. Declaration: `invalid_chunk_crlf`.
- **invalid_trailer:** Invalid trailer. Declaration: `invalid_trailer`.
- **chunked_incomplete:** Chunked incomplete. Declaration: `chunked_incomplete`.
- **raw_incomplete:** Raw incomplete. Declaration: `raw_incomplete`.
- **raw_size_limit_exceeded:** Raw size limit exceeded. Declaration: `raw_size_limit_exceeded`.
- **chunk_extension_size_limit_exceeded:** Chunk extension size limit exceeded. Declaration: `chunk_extension_size_limit_exceeded`.
- **trailer_size_limit_exceeded:** Trailer size limit exceeded. Declaration: `trailer_size_limit_exceeded`.

**Source:** [include/protocol/http/v11/body/reader_error.h:50](../../../../../../include/protocol/http/v11/body/reader_error.h#L50)
