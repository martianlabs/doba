<a id="writer-state-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/v11/body/writer_state/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/v11/body/writer_state/index.svg" alt="Index for writer_state.h">
  </picture>
</h1>

**Header:** [include/protocol/http/v11/body/writer_state.h](../../../../../../include/protocol/http/v11/body/writer_state.h)

Result of a body writer write() call. consumed  - bytes taken from the input span (wire bytes, including any chunked framing). Caller advances its buffer by this amount; the remainder belongs to the next request. complete  - body fully accumulated; no further write() calls needed. has_error - a protocol or size-limit error was detected. error

**protocol::http::v11::body**

- [writer_state](#type-writer-state) · *type*

**protocol::http::v11::body::writer_state**

- [consumed](#field-consumed) · *field*
- [complete](#field-complete) · *field*
- [has_error](#field-has-error) · *field*
- [error](#field-error) · *field*

<a id="type-writer-state"></a>

## writer_state

🟠 **TYPE** · *protocol::http::v11::body*

**Declaration synopsis:**

```cpp
struct writer_state { ... };
```

Result of a body writer write() call. consumed  - bytes taken from the input span (wire bytes, including any chunked framing). Caller advances its buffer by this amount; the remainder belongs to the next request. complete  - body fully accumulated; no further write() calls needed. has_error - a protocol or size-limit error was detected. error

**Source:** [include/protocol/http/v11/body/writer_state.h:46](../../../../../../include/protocol/http/v11/body/writer_state.h#L46)

<a id="field-consumed"></a>

## consumed

🟠 **FIELD** · *protocol::http::v11::body::writer_state*

```cpp
std::size_t consumed = 0;
```

Consumed bytes from the input span (wire bytes, including framing).

**Value:** `0`

**Source:** [include/protocol/http/v11/body/writer_state.h:48](../../../../../../include/protocol/http/v11/body/writer_state.h#L48)

<a id="field-complete"></a>

## complete

🟠 **FIELD** · *protocol::http::v11::body::writer_state*

```cpp
bool complete = false;
```

Body fully accumulated; no further write() calls needed.

**Value:** `false`

**Source:** [include/protocol/http/v11/body/writer_state.h:50](../../../../../../include/protocol/http/v11/body/writer_state.h#L50)

<a id="field-has-error"></a>

## has_error

🟠 **FIELD** · *protocol::http::v11::body::writer_state*

```cpp
bool has_error = false;
```

A protocol or size-limit error was detected.

**Value:** `false`

**Source:** [include/protocol/http/v11/body/writer_state.h:52](../../../../../../include/protocol/http/v11/body/writer_state.h#L52)

<a id="field-error"></a>

## error

🟠 **FIELD** · *protocol::http::v11::body::writer_state*

```cpp
writer_error error = writer_error::none;
```

Error code, if has_error is true.

**Value:** `writer_error::none`

**Source:** [include/protocol/http/v11/body/writer_state.h:54](../../../../../../include/protocol/http/v11/body/writer_state.h#L54)
