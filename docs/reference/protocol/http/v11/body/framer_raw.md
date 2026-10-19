<a id="framer-raw-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/v11/body/framer_raw/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/v11/body/framer_raw/index.svg" alt="Index for framer_raw.h">
  </picture>
</h1>

**Header:** [include/protocol/http/v11/body/framer_raw.h](../../../../../../include/protocol/http/v11/body/framer_raw.h)

Detects the Content-Length-framed body boundary within an already-wire- encoded input buffer.

**protocol::http::v11::body**

- [framer_raw](#type-framer-raw) · *type*

**protocol::http::v11::body::framer_raw**

- [framer_raw](#constructor-framer-raw) · *constructor*
- [consume](#method-consume) · *method*

<a id="type-framer-raw"></a>

## framer_raw

🟠 **TYPE** · *protocol::http::v11::body*

**Declaration synopsis:**

```cpp
class framer_raw { ... };
```

Detects the Content-Length-framed body boundary within an already-wire- encoded input buffer.

**Source:** [include/protocol/http/v11/body/framer_raw.h:48](../../../../../../include/protocol/http/v11/body/framer_raw.h#L48)

<a id="constructor-framer-raw"></a>

## framer_raw

🟠 **CONSTRUCTOR** · *protocol::http::v11::body::framer_raw*

```cpp
explicit framer_raw(std::size_t content_length);
```

Constructs a framer_raw value.

**Parameters:**

- **content_length:** Size or length limit for the named data. Declaration: `std::size_t content_length`.

**Source:** [include/protocol/http/v11/body/framer_raw.h:53](../../../../../../include/protocol/http/v11/body/framer_raw.h#L53)

<a id="method-consume"></a>

## consume

🟠 **METHOD** · *protocol::http::v11::body::framer_raw*

```cpp
framer_state consume(std::span<const std::byte> input);
```

Consumes up to (expected_ - accumulated_) bytes from input. Returns immediately with complete=true when Content-Length is reached. A zero Content-Length body completes on the first call with consumed=0.

**Parameters:**

- **input:** Input data to process. Declaration: `std::span<const std::byte> input`.

**Returns:** A `framer_state` describing progress, completion, and errors.

**Source:** [include/protocol/http/v11/body/framer_raw.h:61](../../../../../../include/protocol/http/v11/body/framer_raw.h#L61)
