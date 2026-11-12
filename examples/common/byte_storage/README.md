<a name="byte-storage"></a>
<h1>
  <a href="../../../resources/examples/common/byte_storage/h1-byte-storage.svg#gh-light-mode-only">
    <img src="../../../resources/examples/common/byte_storage/h1-byte-storage.svg" alt="Byte storage">
  </a>
  <a href="../../../resources/examples/common/byte_storage/h1-byte-storage-dark.svg#gh-dark-mode-only">
    <img src="../../../resources/examples/common/byte_storage/h1-byte-storage-dark.svg" alt="Byte storage">
  </a>
</h1>

This example writes 12 bytes to `byte_storage` with a four-byte spill
threshold. The second write moves the accumulated storage to a temporary file
without changing the read API.

<a name="run"></a>
<h2>
  <a href="../../../resources/examples/common/byte_storage/h2-run.svg#gh-light-mode-only">
    <img src="../../../resources/examples/common/byte_storage/h2-run.svg" alt="Run">
  </a>
  <a href="../../../resources/examples/common/byte_storage/h2-run-dark.svg#gh-dark-mode-only">
    <img src="../../../resources/examples/common/byte_storage/h2-run-dark.svg" alt="Run">
  </a>
</h2>

Run `common_byte_storage`. It prints:

```text
doba storage
```

