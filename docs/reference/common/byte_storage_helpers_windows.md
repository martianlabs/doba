<a id="byte-storage-helpers-windows-h"></a>
<h1>
  <a href="../../../resources/docs/reference/common/byte_storage_helpers_windows/index.svg#gh-light-mode-only">
    <img src="../../../resources/docs/reference/common/byte_storage_helpers_windows/index.svg" alt="Index for byte_storage_helpers_windows.h">
  </a>
  <a href="../../../resources/docs/reference/common/byte_storage_helpers_windows/index-dark.svg#gh-dark-mode-only">
    <img src="../../../resources/docs/reference/common/byte_storage_helpers_windows/index-dark.svg" alt="Index for byte_storage_helpers_windows.h">
  </a>
</h1>

**Header:** [include/common/byte_storage_helpers_windows.h](../../../include/common/byte_storage_helpers_windows.h)

Provides the byte storage file type.

**common**

- [byte_storage_file](#type-byte-storage-file) · *type*

**common::byte_storage_file**

- [open](#method-open) · *method*
- [write](#method-write) · *method*
- [flush](#method-flush) · *method*
- [read](#method-read) · *method*
- [close](#method-close) · *method*

<a id="type-byte-storage-file"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> byte_storage_file

**TYPE** · *common*

**Declaration synopsis:**

```cpp
class byte_storage_file { ... };
```

Provides the byte storage file type.

**Source:** [include/common/byte_storage_helpers_windows.h:43](../../../include/common/byte_storage_helpers_windows.h#L43)

<a id="method-open"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> open

**METHOD** · *common::byte_storage_file*

```cpp
bool open(const std::filesystem::path& directory,
            std::filesystem::path& path);
```

Opens the named resource.

**Parameters:**

- **directory:** Directory path used by this operation. Declaration: `const std::filesystem::path& directory`.
- **path:** Filesystem path or route path, according to the call. Declaration: `std::filesystem::path& path`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/common/byte_storage_helpers_windows.h:66](../../../include/common/byte_storage_helpers_windows.h#L66)

<a id="method-write"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> write

**METHOD** · *common::byte_storage_file*

```cpp
bool write(const char* data, std::size_t size);
```

Writes data.

**Parameters:**

- **data:** Data passed to the operation. Declaration: `const char* data`.
- **size:** Number of elements or bytes, according to the declared type. Declaration: `std::size_t size`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/common/byte_storage_helpers_windows.h:87](../../../include/common/byte_storage_helpers_windows.h#L87)

<a id="method-flush"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> flush

**METHOD** · *common::byte_storage_file*

```cpp
bool flush();
```

Flushes buffered output.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/common/byte_storage_helpers_windows.h:104](../../../include/common/byte_storage_helpers_windows.h#L104)

<a id="method-read"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> read

**METHOD** · *common::byte_storage_file*

```cpp
bool read(std::size_t position, char* data, std::size_t size,
            std::size_t& read);
```

Reads data.

**Parameters:**

- **position:** Position in the input or output sequence. Declaration: `std::size_t position`.
- **data:** Data passed to the operation. Declaration: `char* data`.
- **size:** Number of elements or bytes, according to the declared type. Declaration: `std::size_t size`.
- **read:** Number of bytes read. Declaration: `std::size_t& read`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/common/byte_storage_helpers_windows.h:108](../../../include/common/byte_storage_helpers_windows.h#L108)

<a id="method-close"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> close

**METHOD** · *common::byte_storage_file*

```cpp
void close() noexcept;
```

Closes the requested value.

**Returns:** No value (`void`).

**Errors:** Declared `noexcept`.

**Source:** [include/common/byte_storage_helpers_windows.h:128](../../../include/common/byte_storage_helpers_windows.h#L128)
