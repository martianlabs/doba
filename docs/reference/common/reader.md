<a id="reader"></a>
<a href="../../../resources/docs/reference/common/reader/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/reader/title.svg" alt="reader">
</a>
<a href="../../../resources/docs/reference/common/reader/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/reader/title-dark.svg" alt="reader">
</a>

**Header:** [include/common/reader.h](../../../include/common/reader.h)

<a id="type-reader"></a>
## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> reader

A move-only cursor over owned [byte storage](byte_storage.md), an owned open
[file](filesystem.md), borrowed bytes or shared immutable bytes. The default
reader is empty and healthy, with an unknown size because its byte storage is
unfinished. Moving preserves the unread suffix and transfers any owner.

Methods on one reader need serialized access. Separate shared readers have
independent cursors, provided their backing bytes stay immutable.

<a id="method-borrowed"></a>
## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> borrowed

```cpp
static reader borrowed(std::span<const std::byte> source) noexcept;
```

Do not copy or own the bytes. Keep them alive and unchanged until the reader
and every object it is moved into finish using them. Empty input is allowed.

<a id="method-shared"></a>
## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> shared

```cpp
static reader shared(std::shared_ptr<const void> owner,
                     std::span<const std::byte> source);
```

Retain `owner` without copying the bytes. The caller must ensure that `source`
refers to stable storage retained by that owner and is immutable through every
alias. A null owner throws `std::invalid_argument`, even for empty input.

<a id="method-rebind-borrowed"></a>
## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> rebind_borrowed

```cpp
void rebind_borrowed(std::span<const std::byte> source) noexcept;
```

Replace the span of a non-owning borrowed reader without resetting its cursor.
Use this when the same logical buffer moves: preserve its contents and size,
including already-consumed bytes. It does nothing for file, byte-storage and
shared-owner readers. It cannot rewind or recover a reader.

<a id="method-read"></a>
## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> read

```cpp
std::size_t read(std::span<std::byte> output);
```

Read up to the span size and advance the cursor. Only the returned number of
bytes is valid output. Empty output returns zero without changing state.
Borrowed/shared reads allow the destination to overlap their source.
An I/O error may return a partial count; inspect the state as well.

<a id="method-fetch"></a>
## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> fetch

```cpp
bool fetch(std::byte& byte);
```

Read one byte through the same cursor. False means no byte was produced.
The caller must check `failed()`/`ok()` to distinguish an I/O failure from end.

<a id="method-eof"></a>
<a id="method-exhausted"></a>
<a id="method-failed"></a>
<a id="method-ok"></a>
<a id="method-size"></a>
## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> State and size

| Operation | Result |
| --- | --- |
| `eof()` / `exhausted()` | The backend's read end was reached; this does not by itself report an error. |
| `failed()` | This reader has observed a backend failure while reading. |
| `ok()` | The backend has no recorded error; borrowed/shared readers return true. |
| `size()` | Total size, including bytes already read. Byte-storage size is unknown before `finish()`. |

Zero bytes is not sufficient to infer end: unfinished spilled storage also
returns zero. A reader adopting already-failed storage can have `ok() == false`
before a read updates `failed()`.

<a id="method-read-all"></a>
## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> read_all

```cpp
std::size_t read_all(std::string& out);
```

Append remaining bytes to `out` without clearing it. Return the number appended
by this call, not the final string length. Stop at end, a recorded failure or
a zero-byte read. A partial result may remain after I/O failure; check state.
String allocation failures propagate as exceptions.
