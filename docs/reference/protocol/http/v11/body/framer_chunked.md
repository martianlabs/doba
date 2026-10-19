<a id="framer-chunked-h"></a>
<h1>
  <a href="../../../../../../resources/docs/reference/protocol/http/v11/body/framer_chunked/index.svg#gh-light-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/v11/body/framer_chunked/index.svg" alt="Index for framer_chunked.h">
  </a>
  <a href="../../../../../../resources/docs/reference/protocol/http/v11/body/framer_chunked/index-dark.svg#gh-dark-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/v11/body/framer_chunked/index-dark.svg" alt="Index for framer_chunked.h">
  </a>
</h1>

**Header:** [include/protocol/http/v11/body/framer_chunked.h](../../../../../../include/protocol/http/v11/body/framer_chunked.h)

Validates a chunked Transfer-Encoding body.

**protocol::http::v11::body**

- [framer_chunked](#type-framer-chunked) · *type*

**protocol::http::v11::body::framer_chunked**

- [kDefaultMaxChunkExtensionSize](#constant-kdefaultmaxchunkextensionsize) · *constant*
- [kDefaultMaxTrailerSectionSize](#constant-kdefaultmaxtrailersectionsize) · *constant*
- [consume](#method-consume) · *method*

<a id="type-framer-chunked"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> framer_chunked

**TYPE** · *protocol::http::v11::body*

**Declaration synopsis:**

```cpp
class framer_chunked { ... };
```

Validates a chunked Transfer-Encoding body.

**Source:** [include/protocol/http/v11/body/framer_chunked.h:54](../../../../../../include/protocol/http/v11/body/framer_chunked.h#L54)

<a id="constant-kdefaultmaxchunkextensionsize"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kDefaultMaxChunkExtensionSize

**CONSTANT** · *protocol::http::v11::body::framer_chunked*

```cpp
static constexpr std::size_t kDefaultMaxChunkExtensionSize =
      policies::kDefaultMaxChunkExtensionSize;
```

Defines the default max chunk extension size constant.

**Value:** `policies::kDefaultMaxChunkExtensionSize`

**Source:** [include/protocol/http/v11/body/framer_chunked.h:86](../../../../../../include/protocol/http/v11/body/framer_chunked.h#L86)

<a id="constant-kdefaultmaxtrailersectionsize"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kDefaultMaxTrailerSectionSize

**CONSTANT** · *protocol::http::v11::body::framer_chunked*

```cpp
static constexpr std::size_t kDefaultMaxTrailerSectionSize =
      policies::kDefaultMaxTrailerSectionSize;
```

Defines the default max trailer section size constant.

**Value:** `policies::kDefaultMaxTrailerSectionSize`

**Source:** [include/protocol/http/v11/body/framer_chunked.h:88](../../../../../../include/protocol/http/v11/body/framer_chunked.h#L88)

<a id="method-consume"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume

**METHOD** · *protocol::http::v11::body::framer_chunked*

```cpp
framer_state consume(std::span<const std::byte> input);
```

Validates the chunked framing in input. Returns bytes consumed and whether the body is complete (last-chunk + terminating CRLF seen).

**Parameters:**

- **input:** Input data to process. Declaration: `std::span<const std::byte> input`.

**Returns:** A `framer_state` describing progress, completion, and errors.

**Source:** [include/protocol/http/v11/body/framer_chunked.h:107](../../../../../../include/protocol/http/v11/body/framer_chunked.h#L107)
