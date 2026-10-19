<a id="verdict-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../resources/docs/reference/protocol/http/v11/verdict/index-dark.svg">
    <img src="../../../../../resources/docs/reference/protocol/http/v11/verdict/index.svg" alt="Index for verdict.h">
  </picture>
</h1>

**Header:** [include/protocol/http/v11/verdict.h](../../../../../include/protocol/http/v11/verdict.h)

This header declares the types and operations listed below.

**protocol::http::v11**

- [verdict](#enum-verdict) · *enum*

<a id="enum-verdict"></a>

## verdict

🟠 **ENUM** · *protocol::http::v11*

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
