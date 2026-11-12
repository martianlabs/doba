<a id="byte-storage"></a>
<a href="../../../resources/docs/reference/common/byte_storage/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/byte_storage/title.svg" alt="byte_storage">
</a>
<a href="../../../resources/docs/reference/common/byte_storage/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/byte_storage/title-dark.svg" alt="byte_storage">
</a>

**Header:** [include/common/byte_storage.h](../../../include/common/byte_storage.h)

<a id="type-byte-storage"></a>
## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> byte_storage

Move-only byte storage, initially in memory. A write above the configured
[threshold](byte_storage_options.md) spills all buffered bytes to a private
temporary file. Exactly reaching the threshold does not spill; zero disables
automatic spilling. Construction does not reserve the threshold in memory.

Destruction closes the file and attempts to remove its temporary name. Moving
transfers the bytes, file, cursor and state; move assignment first cleans up the
destination. Serialize access to each storage object.

<a id="method-write"></a>
## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> write

```cpp
bool write(const char* ptr, std::size_t len);
```

Append `len` bytes. The source must contain that many readable bytes for the
call. A zero-length write succeeds before finalization, including a null
pointer. Writes after `finish()` or an I/O failure return false, including
empty writes. File creation/write failures return false and make `ok()` false;
allocation failures may throw. The caller must check the result.

<a id="method-read"></a>
## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> read

```cpp
std::size_t read(char* out, std::size_t want);
```

Copy up to `want` unread bytes and advance the cursor. The destination must
hold the requested bytes. A zero request does not access the destination.
Memory bytes can be read before finalization; spilled storage returns zero
until `finish()` records its size. An I/O failure can return a partial count
and makes `ok()` false; later reads return zero.

<a id="method-fetch"></a>
## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> fetch

```cpp
bool fetch(std::byte& byte);
```

Read one byte using the same cursor as `read()`. Return false without changing
the output byte at end, before spilled storage is finished, or after an I/O
failure. Check `ok()` to distinguish an I/O error.

<a id="method-spill"></a>
## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> spill

```cpp
bool spill();
```

Explicitly move memory bytes to a temporary file, even with a zero automatic
threshold. Preserve the read cursor. An already-spilled store returns true,
even after an earlier I/O error; check `ok()` for health.
File creation/copy failure returns false and records an I/O error; this is not
a way to recover a failed store. Allocation failures may throw.

<a id="method-unlink-file"></a>
## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> unlink_file

```cpp
void unlink_file() noexcept;
```

Attempt to remove the temporary name while keeping the open handle readable.
Removal errors are ignored and the name remains available for another cleanup
attempt. This does nothing when there is no temporary name.

<a id="method-finish"></a>
## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> finish

```cpp
void finish(std::size_t stored) noexcept;
```

Seal writes and record `stored` as the total byte count. The caller must supply
the actual number of stored bytes; the value is not validated. The first call
wins and does not rewind the read cursor. It calls the backend flush hook,
which is currently a no-op on both platforms. It does not call `fsync` or
`FlushFileBuffers`, so finalization makes no disk-durability guarantee.

<a id="method-total-size"></a>
<a id="method-exhausted"></a>
<a id="method-ok"></a>
## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> State

| Operation | Result |
| --- | --- |
| `total_size()` | The count passed to the first `finish()`, or `nullopt` before it. |
| `exhausted()` | Memory cursor has reached the current memory size; for spilled storage, finalization is also required. |
| `ok()` | No recorded file creation, read, write or flush failure. |

An unfinished memory store can become exhausted and later receive more writes.
Finalization and read exhaustion are separate states.
