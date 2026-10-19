<a id="writer-error"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/body/writer_error/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/body/writer_error/index.svg" alt="writer_error">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/body/writer_error/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/body/writer_error/index-dark.svg" alt="writer_error">
</a>

**Header:** [include/protocol/http/v11/body/writer_error.h](../../../../../../include/protocol/http/v11/body/writer_error.h)

This header declares the types and operations listed below.

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::body

[writer_error](#enum-writer-error)

<a id="api-details"></a>
<a href="../../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="enum-writer-error"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> writer_error

**ENUM** · *protocol::http::v11::body*

```cpp
enum class writer_error : std::uint8_t {
  none,
  io_error,
  invalid_chunk_size,
  chunk_size_overflow,
  invalid_chunk_crlf,
  invalid_trailer,
  chunked_incomplete,
  raw_size_limit_exceeded,
  chunk_extension_size_limit_exceeded,
  trailer_size_limit_exceeded
};
```

Error codes shared by writer_raw and writer_chunked.

**Values:**

- **none:** None. Declaration: `none`.
- **io_error:** Io error. Declaration: `io_error`.
- **invalid_chunk_size:** Invalid chunk size. Declaration: `invalid_chunk_size`.
- **chunk_size_overflow:** Chunk size overflow. Declaration: `chunk_size_overflow`.
- **invalid_chunk_crlf:** Invalid chunk crlf. Declaration: `invalid_chunk_crlf`.
- **invalid_trailer:** Invalid trailer. Declaration: `invalid_trailer`.
- **chunked_incomplete:** Chunked incomplete. Declaration: `chunked_incomplete`.
- **raw_size_limit_exceeded:** Raw size limit exceeded. Declaration: `raw_size_limit_exceeded`.
- **chunk_extension_size_limit_exceeded:** Chunk extension size limit exceeded. Declaration: `chunk_extension_size_limit_exceeded`.
- **trailer_size_limit_exceeded:** Trailer size limit exceeded. Declaration: `trailer_size_limit_exceeded`.

**Source:** [include/protocol/http/v11/body/writer_error.h:55](../../../../../../include/protocol/http/v11/body/writer_error.h#L55)
