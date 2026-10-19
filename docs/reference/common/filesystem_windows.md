<a id="filesystem-windows"></a>
<a href="../../../resources/docs/reference/common/filesystem_windows/index.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/filesystem_windows/index.svg" alt="filesystem_windows">
</a>
<a href="../../../resources/docs/reference/common/filesystem_windows/index-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/filesystem_windows/index-dark.svg" alt="filesystem_windows">
</a>

**Header:** [include/common/filesystem_windows.h](../../../include/common/filesystem_windows.h)

Windows filesystem file implementation.

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common

[filesystem_file](#type-filesystem-file)

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common::filesystem_file

[open](#method-open) ·
[read](#method-read) ·
[size](#method-size) ·
[is_open](#method-is-open) ·
[eof](#method-eof) ·
[failed](#method-failed) ·
[close](#method-close)

<a id="api-details"></a>
<a href="../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-filesystem-file"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> filesystem_file

**TYPE** · *common*

**Declaration synopsis:**

```cpp
class filesystem_file { ... };
```

Windows filesystem file implementation.

**Source:** [include/common/filesystem_windows.h:47](../../../include/common/filesystem_windows.h#L47)

<a id="method-open"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> open

**METHOD** · *common::filesystem_file*

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

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> read

**METHOD** · *common::filesystem_file*

```cpp
std::size_t read(std::span<std::byte> output);
```

Reads data.

**Parameters:**

- **output:** Destination or output callback, as specified by its type. Declaration: `std::span<std::byte> output`.

**Returns:** The number of bytes processed.

**Source:** [include/common/filesystem_windows.h:103](../../../include/common/filesystem_windows.h#L103)

<a id="method-size"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> size

**METHOD** · *common::filesystem_file*

```cpp
[[nodiscard]] std::size_t size() const noexcept;
```

Returns the current size.

**Returns:** The size count or size.

**Errors:** Declared `noexcept`.

**Source:** [include/common/filesystem_windows.h:126](../../../include/common/filesystem_windows.h#L126)

<a id="method-is-open"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_open

**METHOD** · *common::filesystem_file*

```cpp
[[nodiscard]] bool is_open() const noexcept;
```

Reports whether open applies.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/filesystem_windows.h:130](../../../include/common/filesystem_windows.h#L130)

<a id="method-eof"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> eof

**METHOD** · *common::filesystem_file*

```cpp
[[nodiscard]] bool eof() const noexcept;
```

Reports whether the eof condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/filesystem_windows.h:136](../../../include/common/filesystem_windows.h#L136)

<a id="method-failed"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> failed

**METHOD** · *common::filesystem_file*

```cpp
[[nodiscard]] bool failed() const noexcept;
```

Reports whether the failed condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/filesystem_windows.h:140](../../../include/common/filesystem_windows.h#L140)

<a id="method-close"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> close

**METHOD** · *common::filesystem_file*

```cpp
void close() noexcept;
```

Closes the requested value.

**Returns:** No value (`void`).

**Errors:** Declared `noexcept`.

**Source:** [include/common/filesystem_windows.h:144](../../../include/common/filesystem_windows.h#L144)
