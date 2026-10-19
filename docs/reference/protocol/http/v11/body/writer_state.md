<a id="writer-state"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/body/writer_state/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/body/writer_state/index.svg" alt="writer_state">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/body/writer_state/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/body/writer_state/index-dark.svg" alt="writer_state">
</a>

**Header:** [include/protocol/http/v11/body/writer_state.h](../../../../../../include/protocol/http/v11/body/writer_state.h)

Result of a body writer write() call. consumed  - bytes taken from the input span (wire bytes, including any chunked framing). Caller advances its buffer by this amount; the remainder belongs to the next request. complete  - body fully accumulated; no further write() calls needed. has_error - a protocol or size-limit error was detected. error

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::body

[writer_state](#type-writer-state)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::body::writer_state

[consumed](#field-consumed) ·
[complete](#field-complete) ·
[has_error](#field-has-error) ·
[error](#field-error)

<a id="api-details"></a>
<a href="../../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-writer-state"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> writer_state

**TYPE** · *protocol::http::v11::body*

**Declaration synopsis:**

```cpp
struct writer_state { ... };
```

Result of a body writer write() call. consumed  - bytes taken from the input span (wire bytes, including any chunked framing). Caller advances its buffer by this amount; the remainder belongs to the next request. complete  - body fully accumulated; no further write() calls needed. has_error - a protocol or size-limit error was detected. error

**Source:** [include/protocol/http/v11/body/writer_state.h:46](../../../../../../include/protocol/http/v11/body/writer_state.h#L46)

<a id="field-consumed"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consumed

**FIELD** · *protocol::http::v11::body::writer_state*

```cpp
std::size_t consumed = 0;
```

Consumed bytes from the input span (wire bytes, including framing).

**Value:** `0`

**Source:** [include/protocol/http/v11/body/writer_state.h:48](../../../../../../include/protocol/http/v11/body/writer_state.h#L48)

<a id="field-complete"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> complete

**FIELD** · *protocol::http::v11::body::writer_state*

```cpp
bool complete = false;
```

Body fully accumulated; no further write() calls needed.

**Value:** `false`

**Source:** [include/protocol/http/v11/body/writer_state.h:50](../../../../../../include/protocol/http/v11/body/writer_state.h#L50)

<a id="field-has-error"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> has_error

**FIELD** · *protocol::http::v11::body::writer_state*

```cpp
bool has_error = false;
```

A protocol or size-limit error was detected.

**Value:** `false`

**Source:** [include/protocol/http/v11/body/writer_state.h:52](../../../../../../include/protocol/http/v11/body/writer_state.h#L52)

<a id="field-error"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> error

**FIELD** · *protocol::http::v11::body::writer_state*

```cpp
writer_error error = writer_error::none;
```

Error code, if has_error is true.

**Value:** `writer_error::none`

**Source:** [include/protocol/http/v11/body/writer_state.h:54](../../../../../../include/protocol/http/v11/body/writer_state.h#L54)
