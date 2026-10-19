<a id="rejection-reason-h"></a>
<h1>
  <a href="../../../../../resources/docs/reference/protocol/http/v11/rejection_reason/index.svg#gh-light-mode-only">
    <img src="../../../../../resources/docs/reference/protocol/http/v11/rejection_reason/index.svg" alt="Index for rejection_reason.h">
  </a>
  <a href="../../../../../resources/docs/reference/protocol/http/v11/rejection_reason/index-dark.svg#gh-dark-mode-only">
    <img src="../../../../../resources/docs/reference/protocol/http/v11/rejection_reason/index-dark.svg" alt="Index for rejection_reason.h">
  </a>
</h1>

**Header:** [include/protocol/http/v11/rejection_reason.h](../../../../../include/protocol/http/v11/rejection_reason.h)

This header declares the types and operations listed below.

**protocol::http::v11**

- [rejection_reason](#enum-rejection-reason) · *enum*

<a id="enum-rejection-reason"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> rejection_reason

**ENUM** · *protocol::http::v11*

```cpp
enum class rejection_reason {
  kNone,                  // no rejection (or reason not tracked yet).
  kSyntax,                // malformed message; maps to 400 Bad Request.
  kPayloadTooLarge,       // policy size limit exceeded; maps to 413.
  kUnsupportedFeature,    // valid feature disabled by policy; maps to 501.
  kVersionNotSupported,   // HTTP version above what is spoken; maps to 505.
  kUriTooLong,            // request-target too long; maps to 414.
  kHeaderFieldsTooLarge,  // header section too large; maps to 431.
  kHandlerError,          // user handler threw an exception; maps to 500.
  kExpectationFailed,     // unsupported Expect expectation; maps to 417.
};
```

Why a verdict::kReject (or a syntactic kInvalidSource) happened, at the granularity the HTTP/1.1 layer needs to pick a status code. kNone                 | no rejection (or reason not tracked yet). kSyntax               | malformed message; maps to 400 Bad Request. kPayloadTooLarge      | a policy limit on message size was exceeded; maps | to 413

**Values:**

- **kNone:** None. Declaration: `kNone`.
- **kSyntax:** Syntax. Declaration: `kSyntax`.
- **kPayloadTooLarge:** Payload too large. Declaration: `kPayloadTooLarge`.
- **kUnsupportedFeature:** Unsupported feature. Declaration: `kUnsupportedFeature`.
- **kVersionNotSupported:** Version not supported. Declaration: `kVersionNotSupported`.
- **kUriTooLong:** Uri too long. Declaration: `kUriTooLong`.
- **kHeaderFieldsTooLarge:** Header fields too large. Declaration: `kHeaderFieldsTooLarge`.
- **kHandlerError:** Handler error. Declaration: `kHandlerError`.
- **kExpectationFailed:** Expectation failed. Declaration: `kExpectationFailed`.

**Source:** [include/protocol/http/v11/rejection_reason.h:64](../../../../../include/protocol/http/v11/rejection_reason.h#L64)
