<a id="filesystem-windows"></a>
<a href="../../../resources/docs/reference/common/filesystem_windows/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/filesystem_windows/title.svg" alt="filesystem_windows">
</a>
<a href="../../../resources/docs/reference/common/filesystem_windows/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/filesystem_windows/title-dark.svg" alt="filesystem_windows">
</a>

**Header:** [include/common/filesystem_windows.h](../../../include/common/filesystem_windows.h)

Owns an open file and its read cursor. It can be moved, but not copied.
Moving it into a reader keeps the file open until the reader releases it.

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> filesystem

```cpp
class filesystem;
```

| Operation | Behavior |
| --- | --- |
| static root(path, error) | Resolves an existing directory to its canonical, absolute path. |
| static valid_relative_path(path, error) | Checks a request path before opening it. Rejects empty components, dot segments, backslashes, colons and control characters. |
| open(root, path, error) | Opens a regular file below the root without following links. |
| static inspect(root, path, error) | Returns current file metadata without reading its contents. |
| metadata(error) | Returns metadata from the open handle, independently of the selected read slice. |
| read(output) | Reads the next bytes, up to the size recorded at open or select. |
| select(offset, length, error) | Sets the slice that will be read. |
| size() | Returns the length of the selected slice, or the size recorded at open. |
| is_open() | Reports whether a handle is open. |
| eof() | Reports whether the cursor reached the selected size. |
| failed() | Reports a failed read, including early EOF after truncation. |
| close() | Releases the handle and resets the read state. |

root, open and inspect report failures through error. root returns an empty path
on failure; open returns false; inspect and metadata return an empty optional.
Metadata is declared in [filesystem_metadata.h](filesystem_metadata.md).

Request paths use forward slashes and must be relative. HTTP percent decoding
happens in the server. The root must already be absolute when calling open or
inspect; every path component is checked again so a replacement link is rejected.

The handle keeps the file open, but does not freeze its contents. Growth does
not extend the read; a file truncated before the recorded size makes read fail.
Reads are synchronous.

Windows checks reparse points and keeps parent directories open while resolving
the file. inspect requests FILE_READ_ATTRIBUTES; open requests GENERIC_READ.
Device names and ambiguous Windows path components are rejected. Reading uses
ReadFile and select uses SetFilePointerEx.
