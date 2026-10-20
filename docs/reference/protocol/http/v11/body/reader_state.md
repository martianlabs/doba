<a id="reader-state"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/body/reader_state/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/body/reader_state/title.svg" alt="reader_state">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/body/reader_state/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/body/reader_state/title-dark.svg" alt="reader_state">
</a>

**Header:** [include/protocol/http/v11/body/reader_state.h](../../../../../../include/protocol/http/v11/body/reader_state.h)

Result of a body reader read() call. produced  - bytes of decoded payload written into the caller-supplied output span during this call. Wire framing bytes consumed from the source reader are internal and not reported. complete  - body fully decoded; no further read() calls needed. has_error - a protocol or size-limit error was detected. error

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::body

[reader_state](#type-reader-state)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::body::reader_state

[produced](#field-produced) ·
[complete](#field-complete) ·
[has_error](#field-has-error) ·
[error](#field-error)

<a id="api"></a>
<a href="../../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-reader-state"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> reader_state

**TYPE** · *protocol::http::v11::body*

**Declaration synopsis:**

```cpp
struct reader_state { ... };
```

Result of a body reader read() call. produced  - bytes of decoded payload written into the caller-supplied output span during this call. Wire framing bytes consumed from the source reader are internal and not reported. complete  - body fully decoded; no further read() calls needed. has_error - a protocol or size-limit error was detected. error

**Source:** [include/protocol/http/v11/body/reader_state.h:46](../../../../../../include/protocol/http/v11/body/reader_state.h#L46)

<a id="field-produced"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> produced

**FIELD** · *protocol::http::v11::body::reader_state*

```cpp
std::size_t produced = 0;
```

Bytes of decoded payload written into the output span during this call.

**Value:** `0`

**Source:** [include/protocol/http/v11/body/reader_state.h:48](../../../../../../include/protocol/http/v11/body/reader_state.h#L48)

<a id="field-complete"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> complete

**FIELD** · *protocol::http::v11::body::reader_state*

```cpp
bool complete = false;
```

Body fully decoded; no further read() calls needed.

**Value:** `false`

**Source:** [include/protocol/http/v11/body/reader_state.h:50](../../../../../../include/protocol/http/v11/body/reader_state.h#L50)

<a id="field-has-error"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> has_error

**FIELD** · *protocol::http::v11::body::reader_state*

```cpp
bool has_error = false;
```

A protocol or size-limit error was detected.

**Value:** `false`

**Source:** [include/protocol/http/v11/body/reader_state.h:52](../../../../../../include/protocol/http/v11/body/reader_state.h#L52)

<a id="field-error"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> error

**FIELD** · *protocol::http::v11::body::reader_state*

```cpp
reader_error error = reader_error::none;
```

Error code, if has_error is true.

**Value:** `reader_error::none`

**Source:** [include/protocol/http/v11/body/reader_state.h:54](../../../../../../include/protocol/http/v11/body/reader_state.h#L54)
