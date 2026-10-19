<a id="framer-raw"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/body/framer_raw/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/body/framer_raw/index.svg" alt="framer_raw">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/body/framer_raw/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/body/framer_raw/index-dark.svg" alt="framer_raw">
</a>

**Header:** [include/protocol/http/v11/body/framer_raw.h](../../../../../../include/protocol/http/v11/body/framer_raw.h)

Detects the Content-Length-framed body boundary within an already-wire- encoded input buffer.

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::body

[framer_raw](#type-framer-raw)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::body::framer_raw

[consume](#method-consume)

<a id="api-details"></a>
<a href="../../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-framer-raw"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> framer_raw

**TYPE** · *protocol::http::v11::body*

**Declaration synopsis:**

```cpp
class framer_raw { ... };
```

Detects the Content-Length-framed body boundary within an already-wire- encoded input buffer.

**Source:** [include/protocol/http/v11/body/framer_raw.h:48](../../../../../../include/protocol/http/v11/body/framer_raw.h#L48)

<a id="method-consume"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume

**METHOD** · *protocol::http::v11::body::framer_raw*

```cpp
framer_state consume(std::span<const std::byte> input);
```

Consumes up to (expected_ - accumulated_) bytes from input. Returns immediately with complete=true when Content-Length is reached. A zero Content-Length body completes on the first call with consumed=0.

**Parameters:**

- **input:** Input data to process. Declaration: `std::span<const std::byte> input`.

**Returns:** A `framer_state` describing progress, completion, and errors.

**Source:** [include/protocol/http/v11/body/framer_raw.h:61](../../../../../../include/protocol/http/v11/body/framer_raw.h#L61)
