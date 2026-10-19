<a id="framer-chunked-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/v11/body/framer_chunked/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/v11/body/framer_chunked/index.svg" alt="Index for framer_chunked.h">
  </picture>
</h1>

**Header:** [include/protocol/http/v11/body/framer_chunked.h](../../../../../../include/protocol/http/v11/body/framer_chunked.h)

Validates a chunked Transfer-Encoding body.

**protocol::http::v11::body**

- [framer_chunked](#type-framer-chunked) · *type*

**protocol::http::v11::body::framer_chunked**

- [kDefaultMaxChunkExtensionSize](#constant-kdefaultmaxchunkextensionsize) · *constant*
- [kDefaultMaxTrailerSectionSize](#constant-kdefaultmaxtrailersectionsize) · *constant*
- [framer_chunked()](#constructor-framer-chunked) · *constructor*
- [framer_chunked(payload_limit, extension_limit, trailer_limit)](#constructor-framer-chunked-2) · *constructor*
- [consume](#method-consume) · *method*

<a id="type-framer-chunked"></a>

## framer_chunked

🟠 **TYPE** · *protocol::http::v11::body*

**Declaration synopsis:**

```cpp
class framer_chunked { ... };
```

Validates a chunked Transfer-Encoding body.

**Source:** [include/protocol/http/v11/body/framer_chunked.h:54](../../../../../../include/protocol/http/v11/body/framer_chunked.h#L54)

<a id="constant-kdefaultmaxchunkextensionsize"></a>

## kDefaultMaxChunkExtensionSize

🟠 **CONSTANT** · *protocol::http::v11::body::framer_chunked*

```cpp
static constexpr std::size_t kDefaultMaxChunkExtensionSize =
      policies::kDefaultMaxChunkExtensionSize;
```

Defines the default max chunk extension size constant.

**Value:** `policies::kDefaultMaxChunkExtensionSize`

**Source:** [include/protocol/http/v11/body/framer_chunked.h:86](../../../../../../include/protocol/http/v11/body/framer_chunked.h#L86)

<a id="constant-kdefaultmaxtrailersectionsize"></a>

## kDefaultMaxTrailerSectionSize

🟠 **CONSTANT** · *protocol::http::v11::body::framer_chunked*

```cpp
static constexpr std::size_t kDefaultMaxTrailerSectionSize =
      policies::kDefaultMaxTrailerSectionSize;
```

Defines the default max trailer section size constant.

**Value:** `policies::kDefaultMaxTrailerSectionSize`

**Source:** [include/protocol/http/v11/body/framer_chunked.h:88](../../../../../../include/protocol/http/v11/body/framer_chunked.h#L88)

<a id="constructor-framer-chunked"></a>

## framer_chunked

🟠 **CONSTRUCTOR** · *protocol::http::v11::body::framer_chunked*

```cpp
framer_chunked() = default;
```

Constructs a framer_chunked value.

**Source:** [include/protocol/http/v11/body/framer_chunked.h:93](../../../../../../include/protocol/http/v11/body/framer_chunked.h#L93)

<a id="constructor-framer-chunked-2"></a>

## framer_chunked

🟠 **CONSTRUCTOR** · *protocol::http::v11::body::framer_chunked*

```cpp
explicit framer_chunked(
      std::size_t payload_limit,
      std::size_t extension_limit = kDefaultMaxChunkExtensionSize,
      std::size_t trailer_limit = kDefaultMaxTrailerSectionSize);
```

Constructs a framer_chunked value.

**Parameters:**

- **payload_limit:** Initial value supplied to the constructor. Declaration: `std::size_t payload_limit`.
- **extension_limit:** Maximum chunk-extension size in bytes. Declaration: `std::size_t extension_limit = kDefaultMaxChunkExtensionSize`.
- **trailer_limit:** Maximum trailer size in bytes. Declaration: `std::size_t trailer_limit = kDefaultMaxTrailerSectionSize`.

**Source:** [include/protocol/http/v11/body/framer_chunked.h:94](../../../../../../include/protocol/http/v11/body/framer_chunked.h#L94)

<a id="method-consume"></a>

## consume

🟠 **METHOD** · *protocol::http::v11::body::framer_chunked*

```cpp
framer_state consume(std::span<const std::byte> input);
```

Validates the chunked framing in input. Returns bytes consumed and whether the body is complete (last-chunk + terminating CRLF seen).

**Parameters:**

- **input:** Input data to process. Declaration: `std::span<const std::byte> input`.

**Returns:** A `framer_state` describing progress, completion, and errors.

**Source:** [include/protocol/http/v11/body/framer_chunked.h:107](../../../../../../include/protocol/http/v11/body/framer_chunked.h#L107)
