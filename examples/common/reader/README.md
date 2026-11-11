<a name="reader"></a>
<h1>
  <a href="../../../resources/examples/common/reader/h1-reader.svg#gh-light-mode-only">
    <img src="../../../resources/examples/common/reader/h1-reader.svg" alt="Reader">
  </a>
  <a href="../../../resources/examples/common/reader/h1-reader-dark.svg#gh-dark-mode-only">
    <img src="../../../resources/examples/common/reader/h1-reader-dark.svg" alt="Reader">
  </a>
</h1>

This example creates a non-owning `reader` over stable caller storage and reads
it in two-byte blocks. The source bytes must outlive the reader.

<a name="run"></a>
<h2>
  <a href="../../../resources/examples/common/reader/h2-run.svg#gh-light-mode-only">
    <img src="../../../resources/examples/common/reader/h2-run.svg" alt="Run">
  </a>
  <a href="../../../resources/examples/common/reader/h2-run-dark.svg#gh-dark-mode-only">
    <img src="../../../resources/examples/common/reader/h2-run-dark.svg" alt="Run">
  </a>
</h2>

Run `common_reader`. It prints `doba` and exits after `eof()` becomes true.

