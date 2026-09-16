# Filesystem

This example opens a regular file below a configured directory and moves its
native handle into a `reader`. It prints the contents in 4096-byte blocks without
preloading the file or copying it to temporary storage.

## Run

Build the `common_filesystem` target. From the repository root, run:

```sh
common_filesystem examples/http/v11/static_file_server/public hello.txt
```

The example prints `Hello from doba.`. A missing file, an absolute request path,
`..`, a symbolic link, or a Windows reparse point fails with a nonzero exit code.

`filesystem_root(path, error)` resolves the trusted root once during setup.
`filesystem_file::open(root, relative, error)` expects that absolute root and a
relative path; HTTP percent decoding is not part of this API. The root is checked
without following new links at each open. Request paths use `/` separators.

The file and its cursor are move-only. `size()` records the size at open:
growth does not extend the read, and truncation before that size sets `failed()`.
Reads are synchronous. The open handle keeps the file object alive, but does
not freeze bytes against writes by another process.
