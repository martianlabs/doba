<a id="filesystem"></a>
<a href="../../../resources/docs/reference/common/filesystem/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/filesystem/title.svg" alt="filesystem">
</a>
<a href="../../../resources/docs/reference/common/filesystem/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/filesystem/title-dark.svg" alt="filesystem">
</a>

**Header:** [include/common/filesystem.h](../../../include/common/filesystem.h)

Selects the native filesystem class for the current platform:

- [Linux](filesystem_linux.md)
- [Windows](filesystem_windows.md)

Both provide common::filesystem. File metadata has its own declaration in
[filesystem_metadata.h](filesystem_metadata.md).

Include this header when using the filesystem from doba.
