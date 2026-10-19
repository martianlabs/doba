<a id="framer-error"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/body/framer_error/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/body/framer_error/title.svg" alt="framer_error">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/body/framer_error/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/body/framer_error/title-dark.svg" alt="framer_error">
</a>

**Header:** [include/protocol/http/v11/body/framer_error.h](../../../../../../include/protocol/http/v11/body/framer_error.h)

This header declares the types and operations listed below.

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::body

[framer_error](#enum-framer-error)

<a id="api"></a>
<a href="../../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="enum-framer-error"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> framer_error

**ENUM** · *protocol::http::v11::body*

```cpp
enum class framer_error : std::uint8_t {
  none,
  invalid_chunk_size,
  chunk_size_overflow,
  invalid_chunk_crlf,
  invalid_trailer,
  chunked_incomplete,
  raw_size_limit_exceeded,
  chunk_extension_size_limit_exceeded,
  trailer_size_limit_exceeded,
  chunked_size_limit_exceeded
};
```

Error codes shared by framer_raw and framer_chunked.

**Values:**

- **none:** None. Declaration: `none`.
- **invalid_chunk_size:** Invalid chunk size. Declaration: `invalid_chunk_size`.
- **chunk_size_overflow:** Chunk size overflow. Declaration: `chunk_size_overflow`.
- **invalid_chunk_crlf:** Invalid chunk crlf. Declaration: `invalid_chunk_crlf`.
- **invalid_trailer:** Invalid trailer. Declaration: `invalid_trailer`.
- **chunked_incomplete:** Chunked incomplete. Declaration: `chunked_incomplete`.
- **raw_size_limit_exceeded:** Raw size limit exceeded. Declaration: `raw_size_limit_exceeded`.
- **chunk_extension_size_limit_exceeded:** Chunk extension size limit exceeded. Declaration: `chunk_extension_size_limit_exceeded`.
- **trailer_size_limit_exceeded:** Trailer size limit exceeded. Declaration: `trailer_size_limit_exceeded`.
- **chunked_size_limit_exceeded:** Chunked size limit exceeded. Declaration: `chunked_size_limit_exceeded`.

**Source:** [include/protocol/http/v11/body/framer_error.h:53](../../../../../../include/protocol/http/v11/body/framer_error.h#L53)
