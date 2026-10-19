<a id="byte-storage-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/common/byte_storage/index-dark.svg">
    <img src="../../../resources/docs/reference/common/byte_storage/index.svg" alt="Index for byte_storage.h">
  </picture>
</h1>

**Header:** [include/common/byte_storage.h](../../../include/common/byte_storage.h)

Provides the byte storage options type.

**common**

- [byte_storage_options](#type-byte-storage-options) · *type*

**common::byte_storage_options**

- [kDefaultSpillToDiskThreshold](#constant-kdefaultspilltodiskthreshold) · *constant*
- [spill_to_disk_threshold](#field-spill-to-disk-threshold) · *field*
- [spill_dir](#field-spill-dir) · *field*

**common**

- [byte_storage](#type-byte-storage) · *type*

**common::byte_storage**

- [byte_storage()](#constructor-byte-storage) · *constructor*
- [byte_storage(opts)](#constructor-byte-storage-2) · *constructor*
- [byte_storage(byte_storage)](#constructor-byte-storage-3) · *constructor*
- [byte_storage(in)](#constructor-byte-storage-4) · *constructor*
- [~byte_storage](#destructor-destructor-byte-storage) · *destructor*
- [operator=(byte_storage)](#operator-operator) · *operator*
- [operator=(in)](#operator-operator-2) · *operator*
- [read](#method-read) · *method*
- [write](#method-write) · *method*
- [spill](#method-spill) · *method*
- [unlink_file](#method-unlink-file) · *method*
- [finish](#method-finish) · *method*
- [fetch](#method-fetch) · *method*
- [total_size](#method-total-size) · *method*
- [exhausted](#method-exhausted) · *method*
- [ok](#method-ok) · *method*

<a id="type-byte-storage-options"></a>

## byte_storage_options

🟠 **TYPE** · *common*

**Declaration synopsis:**

```cpp
struct byte_storage_options { ... };
```

Provides the byte storage options type.

**Source:** [include/common/byte_storage.h:44](../../../include/common/byte_storage.h#L44)

<a id="constant-kdefaultspilltodiskthreshold"></a>

## kDefaultSpillToDiskThreshold

🟠 **CONSTANT** · *common::byte_storage_options*

```cpp
static constexpr std::size_t kDefaultSpillToDiskThreshold = 128 * 1024;
```

Defines the default spill to disk threshold constant.

**Value:** `128 * 1024`

**Source:** [include/common/byte_storage.h:45](../../../include/common/byte_storage.h#L45)

<a id="field-spill-to-disk-threshold"></a>

## spill_to_disk_threshold

🟠 **FIELD** · *common::byte_storage_options*

```cpp
std::size_t spill_to_disk_threshold = kDefaultSpillToDiskThreshold;
```

Stores the spill to disk threshold value.

**Value:** `kDefaultSpillToDiskThreshold`

**Source:** [include/common/byte_storage.h:46](../../../include/common/byte_storage.h#L46)

<a id="field-spill-dir"></a>

## spill_dir

🟠 **FIELD** · *common::byte_storage_options*

```cpp
std::string spill_dir;
```

Stores the spill dir value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/common/byte_storage.h:47](../../../include/common/byte_storage.h#L47)

<a id="type-byte-storage"></a>

## byte_storage

🟠 **TYPE** · *common*

**Declaration synopsis:**

```cpp
class byte_storage { ... };
```

This class provides a storage backend for bytes, with optional spilling to disk when a threshold is exceeded. It supports reading and writing bytes, and can be used in scenarios where large amounts of data need to be temporarily stored without consuming too much memory.

**Source:** [include/common/byte_storage.h:60](../../../include/common/byte_storage.h#L60)

<a id="constructor-byte-storage"></a>

## byte_storage

🟠 **CONSTRUCTOR** · *common::byte_storage*

```cpp
byte_storage() = default;
```

Constructs a byte_storage value.

**Source:** [include/common/byte_storage.h:65](../../../include/common/byte_storage.h#L65)

<a id="constructor-byte-storage-2"></a>

## byte_storage

🟠 **CONSTRUCTOR** · *common::byte_storage*

```cpp
explicit byte_storage(byte_storage_options opts);
```

Constructs a byte_storage value.

**Parameters:**

- **opts:** Storage options; defaults to their default values. Declaration: `byte_storage_options opts`.

**Source:** [include/common/byte_storage.h:66](../../../include/common/byte_storage.h#L66)

<a id="constructor-byte-storage-3"></a>

## byte_storage

🟠 **CONSTRUCTOR** · *common::byte_storage*

```cpp
byte_storage(const byte_storage&) = delete;
```

Disallows this form of common::byte_storage construction.

**Parameters:**

- **byte_storage:** Source object to copy or move. Declaration: `const byte_storage&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/common/byte_storage.h:69](../../../include/common/byte_storage.h#L69)

<a id="constructor-byte-storage-4"></a>

## byte_storage

🟠 **CONSTRUCTOR** · *common::byte_storage*

```cpp
byte_storage(byte_storage&& in) noexcept;
```

Constructs a byte_storage value.

**Parameters:**

- **in:** Input value to move or inspect. Declaration: `byte_storage&& in`.

**Errors:** Declared `noexcept`.

**Source:** [include/common/byte_storage.h:70](../../../include/common/byte_storage.h#L70)

<a id="destructor-destructor-byte-storage"></a>

## ~byte_storage

🟠 **DESTRUCTOR** · *common::byte_storage*

```cpp
~byte_storage();
```

Destroys the byte_storage object.

**Source:** [include/common/byte_storage.h:71](../../../include/common/byte_storage.h#L71)

<a id="operator-operator"></a>

## operator=

🟠 **OPERATOR** · *common::byte_storage*

```cpp
byte_storage& operator=(const byte_storage&) = delete;
```

Disallows this operator= operation.

**Parameters:**

- **byte_storage:** Source object to copy or move. Declaration: `const byte_storage&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/common/byte_storage.h:75](../../../include/common/byte_storage.h#L75)

<a id="operator-operator-2"></a>

## operator=

🟠 **OPERATOR** · *common::byte_storage*

```cpp
byte_storage& operator=(byte_storage&& in) noexcept;
```

Implements operator= for byte_storage.

**Parameters:**

- **in:** Input value to move or inspect. Declaration: `byte_storage&& in`.

**Returns:** A reference to the updated object.

**Errors:** Declared `noexcept`.

**Source:** [include/common/byte_storage.h:76](../../../include/common/byte_storage.h#L76)

<a id="method-read"></a>

## read

🟠 **METHOD** · *common::byte_storage*

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

## write

🟠 **METHOD** · *common::byte_storage*

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

## spill

🟠 **METHOD** · *common::byte_storage*

```cpp
bool spill();
```

Moves buffered data to file-backed storage.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/common/byte_storage.h:117](../../../include/common/byte_storage.h#L117)

<a id="method-unlink-file"></a>

## unlink_file

🟠 **METHOD** · *common::byte_storage*

```cpp
void unlink_file() noexcept;
```

The open handle remains readable after removing the temporary name.

**Returns:** No value (`void`).

**Errors:** Declared `noexcept`.

**Source:** [include/common/byte_storage.h:119](../../../include/common/byte_storage.h#L119)

<a id="method-finish"></a>

## finish

🟠 **METHOD** · *common::byte_storage*

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

## fetch

🟠 **METHOD** · *common::byte_storage*

```cpp
bool fetch(std::byte& b);
```

Fetches the requested value.

**Parameters:**

- **b:** Byte or text value to process. Declaration: `std::byte& b`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/common/byte_storage.h:143](../../../include/common/byte_storage.h#L143)

<a id="method-total-size"></a>

## total_size

🟠 **METHOD** · *common::byte_storage*

```cpp
[[nodiscard]] std::optional<std::size_t> total_size() const noexcept;
```

Returns the total data size.

**Returns:** A `std::optional<std::size_t>` containing the result when present; otherwise `std::nullopt`.

**Errors:** Declared `noexcept`.

**Source:** [include/common/byte_storage.h:153](../../../include/common/byte_storage.h#L153)

<a id="method-exhausted"></a>

## exhausted

🟠 **METHOD** · *common::byte_storage*

```cpp
[[nodiscard]] bool exhausted() const noexcept;
```

Reports whether the exhausted condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/byte_storage.h:159](../../../include/common/byte_storage.h#L159)

<a id="method-ok"></a>

## ok

🟠 **METHOD** · *common::byte_storage*

```cpp
[[nodiscard]] bool ok() const noexcept;
```

Reports whether the ok condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/byte_storage.h:166](../../../include/common/byte_storage.h#L166)
