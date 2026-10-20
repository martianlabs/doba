<a id="filesystem"></a>
<a href="../../../resources/docs/reference/common/filesystem/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/filesystem/title.svg" alt="filesystem">
</a>
<a href="../../../resources/docs/reference/common/filesystem/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/filesystem/title-dark.svg" alt="filesystem">
</a>

**Header:** [include/common/filesystem.h](../../../include/common/filesystem.h)

This header declares the types and operations listed below.

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common

[filesystem_root](#function-filesystem-root)

<a id="api"></a>
<a href="../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/api-dark.svg" alt="API">
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
