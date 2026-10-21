<a id="filesystem-windows"></a>
<a href="../../../resources/docs/reference/common/filesystem_windows/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/filesystem_windows/title.svg" alt="filesystem_windows">
</a>
<a href="../../../resources/docs/reference/common/filesystem_windows/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/filesystem_windows/title-dark.svg" alt="filesystem_windows">
</a>

**Header:** [include/common/filesystem_windows.h](../../../include/common/filesystem_windows.h)

Windows filesystem file implementation.

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common

[filesystem_file](#type-filesystem-file)

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common::filesystem_file

[open](#method-open) ·
[read](#method-read) ·
[select](#method-select) ·
[metadata](#method-metadata) ·
[size](#method-size) ·
[is_open](#method-is-open) ·
[eof](#method-eof) ·
[failed](#method-failed) ·
[close](#method-close)

<a id="api"></a>
<a href="../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/api-dark.svg" alt="API">
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

**Source:** [include/common/filesystem_windows.h:110](../../../include/common/filesystem_windows.h#L110)

<a id="method-is-open"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_open

**METHOD** · *common::filesystem_file*

```cpp
[[nodiscard]] bool is_open() const noexcept;
```

Reports whether open applies.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/filesystem_windows.h:167](../../../include/common/filesystem_windows.h#L167)

<a id="method-eof"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> eof

**METHOD** · *common::filesystem_file*

```cpp
[[nodiscard]] bool eof() const noexcept;
```

Reports whether the eof condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/filesystem_windows.h:173](../../../include/common/filesystem_windows.h#L173)

<a id="method-failed"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> failed

**METHOD** · *common::filesystem_file*

```cpp
[[nodiscard]] bool failed() const noexcept;
```

Reports whether the failed condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/filesystem_windows.h:177](../../../include/common/filesystem_windows.h#L177)

<a id="method-close"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> close

**METHOD** · *common::filesystem_file*

```cpp
void close() noexcept;
```

Closes the requested value.

**Returns:** No value (`void`).

**Errors:** Declared `noexcept`.

**Source:** [include/common/filesystem_windows.h:59](../../../include/common/filesystem_windows.h#L59)

<a id="method-metadata"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> metadata

```cpp
[[nodiscard]] std::optional<filesystem_metadata> metadata(
    std::error_code& error) const;
```

**Parameters:** error is cleared on success and reports a native error on failure.

**Returns:** Metadata from the open handle, or null on failure (including a closed
handle). Does not resolve the path again or change the cursor. See
[filesystem_metadata](filesystem.md#type-filesystem-metadata).

<a id="method-select"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> select

```cpp
bool select(std::size_t offset, std::size_t length, std::error_code& error);
```

**Parameters:** offset/length select a bounded slice before moving an open file into
a reader. Bounds must fit the currently declared length. error reports invalid
bounds or a native seek failure. Offset is absolute within the file.

**Returns:** true on success, false on failure. Resets the cursor to that offset,
size() to length, and failed state. Reads stop at the selected end. No transport
change is involved. Instances remain single-consumer objects; independent files
have independent cursors.
