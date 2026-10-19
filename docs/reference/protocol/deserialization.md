<a id="deserialization-h"></a>
<h1>
  <a href="../../../resources/docs/reference/protocol/deserialization/index.svg#gh-light-mode-only">
    <img src="../../../resources/docs/reference/protocol/deserialization/index.svg" alt="Index for deserialization.h">
  </a>
  <a href="../../../resources/docs/reference/protocol/deserialization/index-dark.svg#gh-dark-mode-only">
    <img src="../../../resources/docs/reference/protocol/deserialization/index-dark.svg" alt="Index for deserialization.h">
  </a>
</h1>

**Header:** [include/protocol/deserialization.h](../../../include/protocol/deserialization.h)

This header declares the types and operations listed below.

**protocol**

- [deserialization_status](#enum-deserialization-status) · *enum*

<a id="enum-deserialization-status"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> deserialization_status

**ENUM** · *protocol*

```cpp
enum class deserialization_status {
  kSucceeded,        // everything went fine.
  kInvalidSource,    // source data is invalid.
  kMoreBytesNeeded,  // more bytes are needed to perform de-serialization.
};
```

This enum class holds the result code on protocol deserialization.

**Values:**

- **kSucceeded:** Succeeded. Declaration: `kSucceeded`.
- **kInvalidSource:** Invalid source. Declaration: `kInvalidSource`.
- **kMoreBytesNeeded:** More bytes needed. Declaration: `kMoreBytesNeeded`.

**Source:** [include/protocol/deserialization.h:36](../../../include/protocol/deserialization.h#L36)
