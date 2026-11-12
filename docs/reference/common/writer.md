<a id="writer"></a>
<a href="../../../resources/docs/reference/common/writer/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/writer/title.svg" alt="writer">
</a>
<a href="../../../resources/docs/reference/common/writer/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/writer/title-dark.svg" alt="writer">
</a>

**Header:** [include/common/writer.h](../../../include/common/writer.h)

<a id="type-writer"></a>
## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> writer

Move-only output backed by [byte_storage](byte_storage.md), using its default
or supplied [storage options](byte_storage_options.md). Successful writes count
the appended bytes; ownership and storage state move with the writer. Serialize
access to each writer.

<a id="method-write"></a>
<a id="method-write-2"></a>
<a id="method-write-3"></a>
## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> write

```cpp
bool write(const char* ptr, std::size_t len);
bool write(std::string_view bytes);
bool write(std::span<const std::byte> bytes);
```

Append the supplied bytes, including embedded zero bytes. The pointer overload
requires `len` readable bytes for a nonempty write. Return false after sealing
or a storage I/O failure; allocation failures may throw. Empty writes follow
the same sealed/error checks. Only successful calls increase the byte count.

<a id="method-finish"></a>
## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> finish

```cpp
void finish(std::size_t bytes);
```

Seal the underlying storage and record the supplied size without validation.
Pass the actual total bytes written. Repeated calls do nothing. This is storage
finalization, not a disk-durability operation. Check `ok()` separately for
recorded storage errors.

<a id="method-ok"></a>
## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> ok

```cpp
bool ok() const noexcept;
```

Report storage I/O health. A sealed writer can still be healthy even though
further writes are rejected.

<a id="method-release"></a>
## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> release

```cpp
byte_storage release();
```

If needed, finish using the count of successful writes, then move out the
storage with its read cursor and error state. This does not clear or fix an
earlier error. Treat the writer as consumed; assign a new writer before reuse.
