<a name="filesystem"></a>
<h1>
  <a href="../../../resources/examples/common/filesystem/h1-filesystem.svg#gh-light-mode-only">
    <img src="../../../resources/examples/common/filesystem/h1-filesystem.svg" alt="Filesystem">
  </a>
  <a href="../../../resources/examples/common/filesystem/h1-filesystem-dark.svg#gh-dark-mode-only">
    <img src="../../../resources/examples/common/filesystem/h1-filesystem-dark.svg" alt="Filesystem">
  </a>
</h1>

This example opens a regular file below a configured directory and moves its
native handle into a `reader`. It prints the contents in 4096-byte blocks without
preloading the file or copying it to temporary storage.

<a name="run"></a>
<h2>
  <a href="../../../resources/examples/common/filesystem/h2-run.svg#gh-light-mode-only">
    <img src="../../../resources/examples/common/filesystem/h2-run.svg" alt="Run">
  </a>
  <a href="../../../resources/examples/common/filesystem/h2-run-dark.svg#gh-dark-mode-only">
    <img src="../../../resources/examples/common/filesystem/h2-run-dark.svg" alt="Run">
  </a>
</h2>

Build the `common_filesystem` target. From the repository root, run:

```sh
common_filesystem examples/common/filesystem README.md
```

The example prints this README. A missing file, an absolute request path, `..`,
a symbolic link, or a Windows reparse point fails with a nonzero exit code.

`filesystem::root(path, error)` resolves the trusted root once during setup.
`filesystem::open(root, relative, error)` expects that absolute root and a
relative path; HTTP percent decoding is not part of this API. The root is checked
without following new links at each open. Request paths use `/` separators.

The file and its cursor are move-only. `size()` records the size at open:
growth does not extend the read, and truncation before that size sets `failed()`.
Reads are synchronous. The open handle keeps the file object alive, but does
not freeze bytes against writes by another process.


<a name="query-attributes"></a>
<h2>
  <a href="../../../resources/examples/common/filesystem/h2-query-attributes.svg#gh-light-mode-only">
    <img src="../../../resources/examples/common/filesystem/h2-query-attributes.svg" alt="Query attributes">
  </a>
  <a href="../../../resources/examples/common/filesystem/h2-query-attributes-dark.svg#gh-dark-mode-only">
    <img src="../../../resources/examples/common/filesystem/h2-query-attributes-dark.svg" alt="Query attributes">
  </a>
</h2>

```sh
common_filesystem examples/common/filesystem README.md --metadata
```

This mode calls `filesystem::inspect(root, relative, error)` and prints the
current identity, size, modification/change times, native attributes and link
count without reading contents. It applies the same path and regular-file rules.
On Linux with openat2 support it uses an attribute-only O_PATH descriptor; Windows
and compatibility fallbacks use the existing confined open/metadata procedure.
