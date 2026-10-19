<a id="filesystem"></a>
<a href="../../../resources/docs/reference/common/filesystem/index.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/filesystem/index.svg" alt="filesystem">
</a>
<a href="../../../resources/docs/reference/common/filesystem/index-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/filesystem/index-dark.svg" alt="filesystem">
</a>

**Header:** [include/common/filesystem.h](../../../include/common/filesystem.h)

This header declares the types and operations listed below.

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common

[filesystem_root](#function-filesystem-root)

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common::detail

[filesystem_relative_path](#function-filesystem-relative-path)

<a id="api-details"></a>
<a href="../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="function-filesystem-root"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> filesystem_root

**FUNCTION** · *common*

```cpp
inline std::filesystem::path filesystem_root(const std::filesystem::path& path,
                                             std::error_code& error);
```

Returns the canonical path of the given directory, or an empty path if the directory does not exist or is not a directory. The error code is set to indicate the reason for failure.

**Parameters:**

- **path:** Filesystem path or route path, according to the call. Declaration: `const std::filesystem::path& path`.
- **error:** Error value to report or inspect. Declaration: `std::error_code& error`.

**Returns:** A value of type `std::filesystem::path`.

**Source:** [include/common/filesystem.h:44](../../../include/common/filesystem.h#L44)

<a id="function-filesystem-relative-path"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> filesystem_relative_path

**FUNCTION** · *common::detail*

```cpp
inline bool filesystem_relative_path(std::string_view path,
                                     std::error_code& error);
```

Returns true if the given path is a valid relative path, false otherwise. The error code is set to indicate the reason for failure.

**Parameters:**

- **path:** Filesystem path or route path, according to the call. Declaration: `std::string_view path`.
- **error:** Error value to report or inspect. Declaration: `std::error_code& error`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/common/filesystem.h:65](../../../include/common/filesystem.h#L65)
