<a id="byte-storage-helpers-linux-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/common/byte_storage_helpers_linux/index-dark.svg">
    <img src="../../../resources/docs/reference/common/byte_storage_helpers_linux/index.svg" alt="Index for byte_storage_helpers_linux.h">
  </picture>
</h1>

**Header:** [include/common/byte_storage_helpers_linux.h](../../../include/common/byte_storage_helpers_linux.h)

Provides the byte storage file type.

**common**

- [byte_storage_file](#type-byte-storage-file) · *type*

**common::byte_storage_file**

- [byte_storage_file()](#constructor-byte-storage-file) · *constructor*
- [byte_storage_file(byte_storage_file)](#constructor-byte-storage-file-2) · *constructor*
- [byte_storage_file(in)](#constructor-byte-storage-file-3) · *constructor*
- [~byte_storage_file](#destructor-destructor-byte-storage-file) · *destructor*
- [operator=(byte_storage_file)](#operator-operator) · *operator*
- [operator=(in)](#operator-operator-2) · *operator*
- [open](#method-open) · *method*
- [write](#method-write) · *method*
- [flush](#method-flush) · *method*
- [read](#method-read) · *method*
- [close](#method-close) · *method*

<a id="type-byte-storage-file"></a>

## byte_storage_file

🟠 **TYPE** · *common*

**Declaration synopsis:**

```cpp
class byte_storage_file { ... };
```

Provides the byte storage file type.

**Source:** [include/common/byte_storage_helpers_linux.h:41](../../../include/common/byte_storage_helpers_linux.h#L41)

<a id="constructor-byte-storage-file"></a>

## byte_storage_file

🟠 **CONSTRUCTOR** · *common::byte_storage_file*

```cpp
byte_storage_file() = default;
```

Constructs a byte_storage_file value.

**Source:** [include/common/byte_storage_helpers_linux.h:46](../../../include/common/byte_storage_helpers_linux.h#L46)

<a id="constructor-byte-storage-file-2"></a>

## byte_storage_file

🟠 **CONSTRUCTOR** · *common::byte_storage_file*

```cpp
byte_storage_file(const byte_storage_file&) = delete;
```

Disallows this form of common::byte_storage_file construction.

**Parameters:**

- **byte_storage_file:** Source object to copy or move. Declaration: `const byte_storage_file&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/common/byte_storage_helpers_linux.h:47](../../../include/common/byte_storage_helpers_linux.h#L47)

<a id="constructor-byte-storage-file-3"></a>

## byte_storage_file

🟠 **CONSTRUCTOR** · *common::byte_storage_file*

```cpp
byte_storage_file(byte_storage_file&& in) noexcept;
```

Constructs a byte_storage_file value.

**Parameters:**

- **in:** Input value to move or inspect. Declaration: `byte_storage_file&& in`.

**Errors:** Declared `noexcept`.

**Source:** [include/common/byte_storage_helpers_linux.h:48](../../../include/common/byte_storage_helpers_linux.h#L48)

<a id="destructor-destructor-byte-storage-file"></a>

## ~byte_storage_file

🟠 **DESTRUCTOR** · *common::byte_storage_file*

```cpp
~byte_storage_file();
```

Destroys the byte_storage_file object.

**Source:** [include/common/byte_storage_helpers_linux.h:50](../../../include/common/byte_storage_helpers_linux.h#L50)

<a id="operator-operator"></a>

## operator=

🟠 **OPERATOR** · *common::byte_storage_file*

```cpp
byte_storage_file& operator=(const byte_storage_file&) = delete;
```

Disallows this operator= operation.

**Parameters:**

- **byte_storage_file:** Source object to copy or move. Declaration: `const byte_storage_file&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/common/byte_storage_helpers_linux.h:54](../../../include/common/byte_storage_helpers_linux.h#L54)

<a id="operator-operator-2"></a>

## operator=

🟠 **OPERATOR** · *common::byte_storage_file*

```cpp
byte_storage_file& operator=(byte_storage_file&& in) noexcept;
```

Implements operator= for byte_storage_file.

**Parameters:**

- **in:** Input value to move or inspect. Declaration: `byte_storage_file&& in`.

**Returns:** A reference to the updated object.

**Errors:** Declared `noexcept`.

**Source:** [include/common/byte_storage_helpers_linux.h:55](../../../include/common/byte_storage_helpers_linux.h#L55)

<a id="method-open"></a>

## open

🟠 **METHOD** · *common::byte_storage_file*

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

## write

🟠 **METHOD** · *common::byte_storage_file*

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

## flush

🟠 **METHOD** · *common::byte_storage_file*

```cpp
bool flush();
```

Flushes buffered output.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/common/byte_storage_helpers_linux.h:95](../../../include/common/byte_storage_helpers_linux.h#L95)

<a id="method-read"></a>

## read

🟠 **METHOD** · *common::byte_storage_file*

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

## close

🟠 **METHOD** · *common::byte_storage_file*

```cpp
void close() noexcept;
```

Closes the requested value.

**Returns:** No value (`void`).

**Errors:** Declared `noexcept`.

**Source:** [include/common/byte_storage_helpers_linux.h:115](../../../include/common/byte_storage_helpers_linux.h#L115)
