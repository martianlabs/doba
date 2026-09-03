# Reader

This example creates a non-owning `reader` over stable caller storage and reads
it in two-byte blocks. The source bytes must outlive the reader.

## Run

Run `common_reader`. It prints `doba` and exits after `eof()` becomes true.

