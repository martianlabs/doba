<a id="verdict"></a>
<a href="../../../../../resources/docs/reference/protocol/http/v11/verdict/title.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/v11/verdict/title.svg" alt="verdict">
</a>
<a href="../../../../../resources/docs/reference/protocol/http/v11/verdict/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/v11/verdict/title-dark.svg" alt="verdict">
</a>

**Header:** [include/protocol/http/v11/verdict.h](../../../../../include/protocol/http/v11/verdict.h)

This header declares the types and operations listed below.

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11

[verdict](#enum-verdict)

<a id="api"></a>
<a href="../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="enum-verdict"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> verdict

**ENUM** · *protocol::http::v11*

```cpp
enum class verdict {
  kAccept,  // the message is semantically valid and may be served.
  kReject,  // the message must not be served.
};
```

The outcome of the semantic interpretation layer. Unlike the syntactic checkers (which only answer "is this well-formed?"), an interpreter or a transversal rule answers "should this message be served?" after taking the parsed values, the request as a whole, and the connection/policies state into account. kAccept - the message is semantically valid

**Values:**

- **kAccept:** Accept. Declaration: `kAccept`.
- **kReject:** Reject. Declaration: `kReject`.

**Source:** [include/protocol/http/v11/verdict.h:43](../../../../../include/protocol/http/v11/verdict.h#L43)
