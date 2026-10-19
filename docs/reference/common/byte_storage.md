<a id="byte-storage"></a>
<a href="../../../resources/docs/reference/common/byte_storage/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/byte_storage/title.svg" alt="byte_storage">
</a>
<a href="../../../resources/docs/reference/common/byte_storage/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/byte_storage/title-dark.svg" alt="byte_storage">
</a>

**Header:** [include/common/byte_storage.h](../../../include/common/byte_storage.h)

Provides the byte storage options type.

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common

[byte_storage_options](#type-byte-storage-options) ·
[byte_storage](#type-byte-storage)

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common::byte_storage_options

[kDefaultSpillToDiskThreshold](#constant-kdefaultspilltodiskthreshold) ·
[spill_to_disk_threshold](#field-spill-to-disk-threshold) ·
[spill_dir](#field-spill-dir)

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common::byte_storage

[read](#method-read) ·
[write](#method-write) ·
[spill](#method-spill) ·
[unlink_file](#method-unlink-file) ·
[finish](#method-finish) ·
[fetch](#method-fetch) ·
[total_size](#method-total-size) ·
[exhausted](#method-exhausted) ·
[ok](#method-ok)

<a id="api"></a>
<a href="../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-byte-storage-options"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> byte_storage_options

**TYPE** · *common*

**Declaration synopsis:**

```cpp
struct byte_storage_options { ... };
```

Provides the byte storage options type.

**Source:** [include/common/byte_storage.h:44](../../../include/common/byte_storage.h#L44)

<a id="constant-kdefaultspilltodiskthreshold"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kDefaultSpillToDiskThreshold

**CONSTANT** · *common::byte_storage_options*

```cpp
static constexpr std::size_t kDefaultSpillToDiskThreshold = 128 * 1024;
```

Defines the default spill to disk threshold constant.

**Value:** `128 * 1024`

**Source:** [include/common/byte_storage.h:45](../../../include/common/byte_storage.h#L45)

<a id="field-spill-to-disk-threshold"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> spill_to_disk_threshold

**FIELD** · *common::byte_storage_options*

```cpp
std::size_t spill_to_disk_threshold = kDefaultSpillToDiskThreshold;
```

Stores the spill to disk threshold value.

**Value:** `kDefaultSpillToDiskThreshold`

**Source:** [include/common/byte_storage.h:46](../../../include/common/byte_storage.h#L46)

<a id="field-spill-dir"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> spill_dir

**FIELD** · *common::byte_storage_options*

```cpp
std::string spill_dir;
```

Stores the spill dir value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/common/byte_storage.h:47](../../../include/common/byte_storage.h#L47)

<a id="type-byte-storage"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> byte_storage

**TYPE** · *common*

**Declaration synopsis:**

```cpp
class byte_storage { ... };
```

This class provides a storage backend for bytes, with optional spilling to disk when a threshold is exceeded. It supports reading and writing bytes, and can be used in scenarios where large amounts of data need to be temporarily stored without consuming too much memory.

**Source:** [include/common/byte_storage.h:60](../../../include/common/byte_storage.h#L60)

<a id="method-read"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> read

**METHOD** · *common::byte_storage*

```cpp
std::size_t read(char* out, std::size_t want);
```

Reads data.

**Parameters:**

- **out:** Output parameter that receives the result. Declaration: `char* out`.
- **want:** Requested byte count. Declaration: `std::size_t want`.

**Returns:** The number of bytes processed.

**Source:** [include/common/byte_storage.h:85](../../../include/common/byte_storage.h#L85)

<a id="method-write"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> write

**METHOD** · *common::byte_storage*

```cpp
bool write(const char* ptr, std::size_t len);
```

Writes data.

**Parameters:**

- **ptr:** Pointer to the input or output memory. Declaration: `const char* ptr`.
- **len:** Length in elements or bytes, according to the declared type. Declaration: `std::size_t len`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/common/byte_storage.h:96](../../../include/common/byte_storage.h#L96)

<a id="method-spill"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> spill

**METHOD** · *common::byte_storage*

```cpp
bool spill();
```

Moves buffered data to file-backed storage.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/common/byte_storage.h:117](../../../include/common/byte_storage.h#L117)

<a id="method-unlink-file"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> unlink_file

**METHOD** · *common::byte_storage*

```cpp
void unlink_file() noexcept;
```

The open handle remains readable after removing the temporary name.

**Returns:** No value (`void`).

**Errors:** Declared `noexcept`.

**Source:** [include/common/byte_storage.h:119](../../../include/common/byte_storage.h#L119)

<a id="method-finish"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> finish

**METHOD** · *common::byte_storage*

```cpp
void finish(std::size_t stored) noexcept;
```

Finalizes the buffered data or output stream.

**Parameters:**

- **stored:** Number of bytes already stored. Declaration: `std::size_t stored`.

**Returns:** No value (`void`).

**Errors:** Declared `noexcept`.

**Source:** [include/common/byte_storage.h:128](../../../include/common/byte_storage.h#L128)

<a id="method-fetch"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> fetch

**METHOD** · *common::byte_storage*

```cpp
bool fetch(std::byte& b);
```

Fetches the requested value.

**Parameters:**

- **b:** Byte or text value to process. Declaration: `std::byte& b`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/common/byte_storage.h:143](../../../include/common/byte_storage.h#L143)

<a id="method-total-size"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> total_size

**METHOD** · *common::byte_storage*

```cpp
[[nodiscard]] std::optional<std::size_t> total_size() const noexcept;
```

Returns the total data size.

**Returns:** A `std::optional<std::size_t>` containing the result when present; otherwise `std::nullopt`.

**Errors:** Declared `noexcept`.

**Source:** [include/common/byte_storage.h:153](../../../include/common/byte_storage.h#L153)

<a id="method-exhausted"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> exhausted

**METHOD** · *common::byte_storage*

```cpp
[[nodiscard]] bool exhausted() const noexcept;
```

Reports whether the exhausted condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/byte_storage.h:159](../../../include/common/byte_storage.h#L159)

<a id="method-ok"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> ok

**METHOD** · *common::byte_storage*

```cpp
[[nodiscard]] bool ok() const noexcept;
```

Reports whether the ok condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/byte_storage.h:166](../../../include/common/byte_storage.h#L166)
