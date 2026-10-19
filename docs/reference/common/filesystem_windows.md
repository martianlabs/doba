<a id="filesystem-windows-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/common/filesystem_windows/index-dark.svg">
    <img src="../../../resources/docs/reference/common/filesystem_windows/index.svg" alt="Index for filesystem_windows.h">
  </picture>
</h1>

**Header:** [include/common/filesystem_windows.h](../../../include/common/filesystem_windows.h)

Windows filesystem file implementation.

**common**

- [filesystem_file](#type-filesystem-file) · *type*

**common::filesystem_file**

- [filesystem_file()](#constructor-filesystem-file) · *constructor*
- [filesystem_file(filesystem_file)](#constructor-filesystem-file-2) · *constructor*
- [filesystem_file(in)](#constructor-filesystem-file-3) · *constructor*
- [~filesystem_file](#destructor-destructor-filesystem-file) · *destructor*
- [operator=(filesystem_file)](#operator-operator) · *operator*
- [operator=(in)](#operator-operator-2) · *operator*
- [open](#method-open) · *method*
- [read](#method-read) · *method*
- [size](#method-size) · *method*
- [is_open](#method-is-open) · *method*
- [eof](#method-eof) · *method*
- [failed](#method-failed) · *method*
- [close](#method-close) · *method*

<a id="type-filesystem-file"></a>

## filesystem_file

🟠 **TYPE** · *common*

**Declaration synopsis:**

```cpp
class filesystem_file { ... };
```

Windows filesystem file implementation.

**Source:** [include/common/filesystem_windows.h:47](../../../include/common/filesystem_windows.h#L47)

<a id="constructor-filesystem-file"></a>

## filesystem_file

🟠 **CONSTRUCTOR** · *common::filesystem_file*

```cpp
filesystem_file() = default;
```

Constructs a filesystem_file value.

**Source:** [include/common/filesystem_windows.h:52](../../../include/common/filesystem_windows.h#L52)

<a id="constructor-filesystem-file-2"></a>

## filesystem_file

🟠 **CONSTRUCTOR** · *common::filesystem_file*

```cpp
filesystem_file(const filesystem_file&) = delete;
```

Disallows this form of common::filesystem_file construction.

**Parameters:**

- **filesystem_file:** Source object to copy or move. Declaration: `const filesystem_file&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/common/filesystem_windows.h:53](../../../include/common/filesystem_windows.h#L53)

<a id="constructor-filesystem-file-3"></a>

## filesystem_file

🟠 **CONSTRUCTOR** · *common::filesystem_file*

```cpp
filesystem_file(filesystem_file&& in) noexcept;
```

Constructs a filesystem_file value.

**Parameters:**

- **in:** Input value to move or inspect. Declaration: `filesystem_file&& in`.

**Errors:** Declared `noexcept`.

**Source:** [include/common/filesystem_windows.h:54](../../../include/common/filesystem_windows.h#L54)

<a id="destructor-destructor-filesystem-file"></a>

## ~filesystem_file

🟠 **DESTRUCTOR** · *common::filesystem_file*

```cpp
~filesystem_file();
```

Destroys the filesystem_file object.

**Source:** [include/common/filesystem_windows.h:59](../../../include/common/filesystem_windows.h#L59)

<a id="operator-operator"></a>

## operator=

🟠 **OPERATOR** · *common::filesystem_file*

```cpp
filesystem_file& operator=(const filesystem_file&) = delete;
```

Disallows this operator= operation.

**Parameters:**

- **filesystem_file:** Source object to copy or move. Declaration: `const filesystem_file&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/common/filesystem_windows.h:63](../../../include/common/filesystem_windows.h#L63)

<a id="operator-operator-2"></a>

## operator=

🟠 **OPERATOR** · *common::filesystem_file*

```cpp
filesystem_file& operator=(filesystem_file&& in) noexcept;
```

Implements operator= for filesystem_file.

**Parameters:**

- **in:** Input value to move or inspect. Declaration: `filesystem_file&& in`.

**Returns:** A reference to the updated object.

**Errors:** Declared `noexcept`.

**Source:** [include/common/filesystem_windows.h:64](../../../include/common/filesystem_windows.h#L64)

<a id="method-open"></a>

## open

🟠 **METHOD** · *common::filesystem_file*

```cpp
bool open(const std::filesystem::path& root, std::string_view path,
            std::error_code& error);
```

Opens the named resource.

**Parameters:**

- **root:** Root filesystem path. Declaration: `const std::filesystem::path& root`.
- **path:** Filesystem path or route path, according to the call. Declaration: `std::string_view path`.
- **error:** Error value to report or inspect. Declaration: `std::error_code& error`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/common/filesystem_windows.h:76](../../../include/common/filesystem_windows.h#L76)

<a id="method-read"></a>

## read

🟠 **METHOD** · *common::filesystem_file*

```cpp
std::size_t read(std::span<std::byte> output);
```

Reads data.

**Parameters:**

- **output:** Destination or output callback, as specified by its type. Declaration: `std::span<std::byte> output`.

**Returns:** The number of bytes processed.

**Source:** [include/common/filesystem_windows.h:103](../../../include/common/filesystem_windows.h#L103)

<a id="method-size"></a>

## size

🟠 **METHOD** · *common::filesystem_file*

```cpp
[[nodiscard]] std::size_t size() const noexcept;
```

Returns the current size.

**Returns:** The size count or size.

**Errors:** Declared `noexcept`.

**Source:** [include/common/filesystem_windows.h:126](../../../include/common/filesystem_windows.h#L126)

<a id="method-is-open"></a>

## is_open

🟠 **METHOD** · *common::filesystem_file*

```cpp
[[nodiscard]] bool is_open() const noexcept;
```

Reports whether open applies.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/filesystem_windows.h:130](../../../include/common/filesystem_windows.h#L130)

<a id="method-eof"></a>

## eof

🟠 **METHOD** · *common::filesystem_file*

```cpp
[[nodiscard]] bool eof() const noexcept;
```

Reports whether the eof condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/filesystem_windows.h:136](../../../include/common/filesystem_windows.h#L136)

<a id="method-failed"></a>

## failed

🟠 **METHOD** · *common::filesystem_file*

```cpp
[[nodiscard]] bool failed() const noexcept;
```

Reports whether the failed condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/filesystem_windows.h:140](../../../include/common/filesystem_windows.h#L140)

<a id="method-close"></a>

## close

🟠 **METHOD** · *common::filesystem_file*

```cpp
void close() noexcept;
```

Closes the requested value.

**Returns:** No value (`void`).

**Errors:** Declared `noexcept`.

**Source:** [include/common/filesystem_windows.h:144](../../../include/common/filesystem_windows.h#L144)
