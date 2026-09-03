# Byte storage

This example writes 12 bytes to `byte_storage` with a four-byte spill
threshold. The second write moves the accumulated storage to a temporary file
without changing the read API.

## Run

Run `common_byte_storage`. It prints:

```text
doba storage
```

