<a id="byte-storage-helpers-linux"></a>
<a href="../../../resources/docs/reference/common/byte_storage_helpers_linux/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/byte_storage_helpers_linux/title.svg" alt="byte_storage_helpers_linux">
</a>
<a href="../../../resources/docs/reference/common/byte_storage_helpers_linux/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/byte_storage_helpers_linux/title-dark.svg" alt="byte_storage_helpers_linux">
</a>

**Header:** [include/common/byte_storage_helpers_linux.h](../../../include/common/byte_storage_helpers_linux.h)

Provides the byte storage file type.

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common

[byte_storage_file](#type-byte-storage-file)

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common::byte_storage_file

[open](#method-open) ·
[write](#method-write) ·
[flush](#method-flush) ·
[read](#method-read) ·
[close](#method-close)

<a id="api"></a>
<a href="../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-byte-storage-file"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> byte_storage_file

**TYPE** · *common*

**Declaration synopsis:**

```cpp
class byte_storage_file { ... };
```

Provides the byte storage file type.

**Source:** [include/common/byte_storage_helpers_linux.h:41](../../../include/common/byte_storage_helpers_linux.h#L41)

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

**Source:** [include/common/byte_storage_helpers_linux.h:64](../../../include/common/byte_storage_helpers_linux.h#L64)

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

**Source:** [include/common/byte_storage_helpers_linux.h:82](../../../include/common/byte_storage_helpers_linux.h#L82)

<a id="method-flush"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> flush

**METHOD** · *common::byte_storage_file*

```cpp
bool flush();
```

Flushes buffered output.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/common/byte_storage_helpers_linux.h:95](../../../include/common/byte_storage_helpers_linux.h#L95)

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

**Source:** [include/common/byte_storage_helpers_linux.h:99](../../../include/common/byte_storage_helpers_linux.h#L99)

<a id="method-close"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> close

**METHOD** · *common::byte_storage_file*

```cpp
void close() noexcept;
```

Closes the requested value.

**Returns:** No value (`void`).

**Errors:** Declared `noexcept`.

**Source:** [include/common/byte_storage_helpers_linux.h:115](../../../include/common/byte_storage_helpers_linux.h#L115)
