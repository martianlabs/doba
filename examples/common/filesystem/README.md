# Filesystem

This example opens a regular file below a configured directory and moves its
native handle into a `reader`. It prints the contents in 4096-byte blocks without
preloading the file or copying it to temporary storage.

## Run

Build the `common_filesystem` target. From the repository root, run:

```sh
common_filesystem examples/common/filesystem README.md
```

The example prints this README. A missing file, an absolute request path, `..`,
a symbolic link, or a Windows reparse point fails with a nonzero exit code.

`filesystem_root(path, error)` resolves the trusted root once during setup.
`filesystem_file::open(root, relative, error)` expects that absolute root and a
relative path; HTTP percent decoding is not part of this API. The root is checked
without following new links at each open. Request paths use `/` separators.

The file and its cursor are move-only. `size()` records the size at open:
growth does not extend the read, and truncation before that size sets `failed()`.
Reads are synchronous. The open handle keeps the file object alive, but does
not freeze bytes against writes by another process.


## Query attributes

```sh
common_filesystem examples/common/filesystem README.md --metadata
```

This mode calls `filesystem_file::inspect(root, relative, error)` and prints the
current identity, size, modification/change times, native attributes and link
count without reading contents. It applies the same path and regular-file rules.
On Linux with openat2 support it uses an attribute-only O_PATH descriptor; Windows
and compatibility fallbacks use the existing confined open/metadata procedure.
