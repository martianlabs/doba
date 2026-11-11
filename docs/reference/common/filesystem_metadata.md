# <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> filesystem_metadata

**Header:** [include/common/filesystem_metadata.h](../../../include/common/filesystem_metadata.h)

File details used to check whether a cached response can still be reused.
All fields start at zero.

```cpp
struct filesystem_metadata { ... };
```

| Field | Type | Meaning |
| --- | --- | --- |
| size | std::uintmax_t | Actual file length in bytes, independent of a read slice. |
| modified_seconds | std::int64_t | Last modification time in Unix seconds. |
| modified_nanoseconds | std::uint32_t | Subsecond modification time. |
| device | std::uint64_t | Device or volume identifier. |
| identity | std::uint64_t | Inode or file index within that device. |
| changed_seconds | std::int64_t | Metadata change time in Unix seconds. |
| changed_nanoseconds | std::uint32_t | Subsecond metadata change time. |
| attributes | std::uint32_t | Native Windows file flags, or Linux file type and permission bits. |
| links | std::uint64_t | Number of hard links to the opened file. |

Equality compares all fields. This is a change detector, not an immutable snapshot
or a content hash. Precision and identity depend on the filesystem. Windows
FILETIME uses 100 ns units; Linux fstat provides nanoseconds.

The current Windows conversion does not normalize fractional timestamps before
the Unix epoch into a seconds/nanoseconds pair. Treat those fields as native
change-detection tokens rather than reconstructing pre-1970 timestamps from them.
