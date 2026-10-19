<a id="reason-phrases"></a>
<a href="../../../../../resources/docs/reference/protocol/http/v11/reason_phrases/index.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/v11/reason_phrases/index.svg" alt="reason_phrases">
</a>
<a href="../../../../../resources/docs/reference/protocol/http/v11/reason_phrases/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/v11/reason_phrases/index-dark.svg" alt="reason_phrases">
</a>

**Header:** [include/protocol/http/v11/reason_phrases.h](../../../../../include/protocol/http/v11/reason_phrases.h)

Defines the preprocessor values listed below.

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11

[RP_100_CONTINUE](#macro-rp-100-continue) ·
[RP_101_SWITCHING_PROTOCOLS](#macro-rp-101-switching-protocols) ·
[RP_200_OK](#macro-rp-200-ok) ·
[RP_201_CREATED](#macro-rp-201-created) ·
[RP_202_ACCEPTED](#macro-rp-202-accepted) ·
[RP_203_NON_AUTHORITATIVE_INFORMATION](#macro-rp-203-non-authoritative-information) ·
[RP_204_NO_CONTENT](#macro-rp-204-no-content) ·
[RP_205_RESET_CONTENT](#macro-rp-205-reset-content) ·
[RP_206_PARTIAL_CONTENT](#macro-rp-206-partial-content) ·
[RP_300_MULTIPLE_CHOICES](#macro-rp-300-multiple-choices) ·
[RP_301_MOVED_PERMANENTLY](#macro-rp-301-moved-permanently) ·
[RP_302_FOUND](#macro-rp-302-found) ·
[RP_303_SEE_OTHER](#macro-rp-303-see-other) ·
[RP_304_NOT_MODIFIED](#macro-rp-304-not-modified) ·
[RP_305_USE_PROXY](#macro-rp-305-use-proxy) ·
[RP_306_UNUSED](#macro-rp-306-unused) ·
[RP_307_TEMPORARY_REDIRECT](#macro-rp-307-temporary-redirect) ·
[RP_308_PERMANENT_REDIRECT](#macro-rp-308-permanent-redirect) ·
[RP_400_BAD_REQUEST](#macro-rp-400-bad-request) ·
[RP_401_UNAUTHORIZED](#macro-rp-401-unauthorized) ·
[RP_402_PAYMENT_REQUIRED](#macro-rp-402-payment-required) ·
[RP_403_FORBIDDEN](#macro-rp-403-forbidden) ·
[RP_404_NOT_FOUND](#macro-rp-404-not-found) ·
[RP_405_METHOD_NOT_ALLOWED](#macro-rp-405-method-not-allowed) ·
[RP_406_NOT_ACCEPTABLE](#macro-rp-406-not-acceptable) ·
[RP_407_PROXY_AUTHENTICATION_REQUIRED](#macro-rp-407-proxy-authentication-required) ·
[RP_408_REQUEST_TIMEOUT](#macro-rp-408-request-timeout) ·
[RP_409_CONFLICT](#macro-rp-409-conflict) ·
[RP_410_GONE](#macro-rp-410-gone) ·
[RP_411_LENGTH_REQUIRED](#macro-rp-411-length-required) ·
[RP_412_PRECONDITION_FAILED](#macro-rp-412-precondition-failed) ·
[RP_413_CONTENT_TOO_LARGE](#macro-rp-413-content-too-large) ·
[RP_414_URI_TOO_LONG](#macro-rp-414-uri-too-long) ·
[RP_415_UNSUPPORTED_MEDIA_TYPE](#macro-rp-415-unsupported-media-type) ·
[RP_416_RANGE_NOT_SATISFIABLE](#macro-rp-416-range-not-satisfiable) ·
[RP_417_EXPECTATION_FAILED](#macro-rp-417-expectation-failed) ·
[RP_418_IM_A_TEAPOT](#macro-rp-418-im-a-teapot) ·
[RP_421_MISDIRECTED_REQUEST](#macro-rp-421-misdirected-request) ·
[RP_422_UNPROCESSABLE_CONTENT](#macro-rp-422-unprocessable-content) ·
[RP_426_UPGRADE_REQUIRED](#macro-rp-426-upgrade-required) ·
[RP_431_REQUEST_HEADER_FIELDS_TOO_LARGE](#macro-rp-431-request-header-fields-too-large) ·
[RP_500_INTERNAL_SERVER_ERROR](#macro-rp-500-internal-server-error) ·
[RP_501_NOT_IMPLEMENTED](#macro-rp-501-not-implemented) ·
[RP_502_BAD_GATEWAY](#macro-rp-502-bad-gateway) ·
[RP_503_SERVICE_UNAVAILABLE](#macro-rp-503-service-unavailable) ·
[RP_504_GATEWAY_TIMEOUT](#macro-rp-504-gateway-timeout) ·
[RP_505_HTTP_VERSION_NOT_SUPPORTED](#macro-rp-505-http-version-not-supported)

<a id="api-details"></a>
<a href="../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="macro-rp-100-continue"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_100_CONTINUE

**MACRO** · *protocol::http::v11*

```cpp
#define RP_100_CONTINUE Continue
```

Code | Reason Phrase 100  | Continue 101  | Switching Protocols 200  | OK 201  | Created 202  | Accepted 203  | Non-Authoritative Information 204  | No Content 205  | Reset Content 206  | Partial Content 300  | Multiple Choices 301  | Moved Permanently 302  | Found 303  | See Other 304  | Not Modified 305  | Use Proxy 306  | (Unused) 307  |

**Value:** `Continue`

**Source:** [include/protocol/http/v11/reason_phrases.h:83](../../../../../include/protocol/http/v11/reason_phrases.h#L83)

<a id="macro-rp-101-switching-protocols"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_101_SWITCHING_PROTOCOLS

**MACRO** · *protocol::http::v11*

```cpp
#define RP_101_SWITCHING_PROTOCOLS Switching Protocols
```

Expands to the HTTP 101 switching protocols reason-phrase tokens.

**Value:** `Switching Protocols`

**Source:** [include/protocol/http/v11/reason_phrases.h:84](../../../../../include/protocol/http/v11/reason_phrases.h#L84)

<a id="macro-rp-200-ok"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_200_OK

**MACRO** · *protocol::http::v11*

```cpp
#define RP_200_OK OK
```

Expands to the HTTP 200 ok reason-phrase tokens.

**Value:** `OK`

**Source:** [include/protocol/http/v11/reason_phrases.h:85](../../../../../include/protocol/http/v11/reason_phrases.h#L85)

<a id="macro-rp-201-created"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_201_CREATED

**MACRO** · *protocol::http::v11*

```cpp
#define RP_201_CREATED Created
```

Expands to the HTTP 201 created reason-phrase tokens.

**Value:** `Created`

**Source:** [include/protocol/http/v11/reason_phrases.h:86](../../../../../include/protocol/http/v11/reason_phrases.h#L86)

<a id="macro-rp-202-accepted"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_202_ACCEPTED

**MACRO** · *protocol::http::v11*

```cpp
#define RP_202_ACCEPTED Accepted
```

Expands to the HTTP 202 accepted reason-phrase tokens.

**Value:** `Accepted`

**Source:** [include/protocol/http/v11/reason_phrases.h:87](../../../../../include/protocol/http/v11/reason_phrases.h#L87)

<a id="macro-rp-203-non-authoritative-information"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_203_NON_AUTHORITATIVE_INFORMATION

**MACRO** · *protocol::http::v11*

```cpp
#define RP_203_NON_AUTHORITATIVE_INFORMATION Non-Authoritative Information
```

Expands to the HTTP 203 non authoritative information reason-phrase tokens.

**Value:** `Non-Authoritative Information`

**Source:** [include/protocol/http/v11/reason_phrases.h:88](../../../../../include/protocol/http/v11/reason_phrases.h#L88)

<a id="macro-rp-204-no-content"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_204_NO_CONTENT

**MACRO** · *protocol::http::v11*

```cpp
#define RP_204_NO_CONTENT No Content
```

Expands to the HTTP 204 no content reason-phrase tokens.

**Value:** `No Content`

**Source:** [include/protocol/http/v11/reason_phrases.h:89](../../../../../include/protocol/http/v11/reason_phrases.h#L89)

<a id="macro-rp-205-reset-content"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_205_RESET_CONTENT

**MACRO** · *protocol::http::v11*

```cpp
#define RP_205_RESET_CONTENT Reset Content
```

Expands to the HTTP 205 reset content reason-phrase tokens.

**Value:** `Reset Content`

**Source:** [include/protocol/http/v11/reason_phrases.h:90](../../../../../include/protocol/http/v11/reason_phrases.h#L90)

<a id="macro-rp-206-partial-content"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_206_PARTIAL_CONTENT

**MACRO** · *protocol::http::v11*

```cpp
#define RP_206_PARTIAL_CONTENT Partial Content
```

Expands to the HTTP 206 partial content reason-phrase tokens.

**Value:** `Partial Content`

**Source:** [include/protocol/http/v11/reason_phrases.h:91](../../../../../include/protocol/http/v11/reason_phrases.h#L91)

<a id="macro-rp-300-multiple-choices"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_300_MULTIPLE_CHOICES

**MACRO** · *protocol::http::v11*

```cpp
#define RP_300_MULTIPLE_CHOICES Multiple Choices
```

Expands to the HTTP 300 multiple choices reason-phrase tokens.

**Value:** `Multiple Choices`

**Source:** [include/protocol/http/v11/reason_phrases.h:92](../../../../../include/protocol/http/v11/reason_phrases.h#L92)

<a id="macro-rp-301-moved-permanently"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_301_MOVED_PERMANENTLY

**MACRO** · *protocol::http::v11*

```cpp
#define RP_301_MOVED_PERMANENTLY Moved Permanently
```

Expands to the HTTP 301 moved permanently reason-phrase tokens.

**Value:** `Moved Permanently`

**Source:** [include/protocol/http/v11/reason_phrases.h:93](../../../../../include/protocol/http/v11/reason_phrases.h#L93)

<a id="macro-rp-302-found"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_302_FOUND

**MACRO** · *protocol::http::v11*

```cpp
#define RP_302_FOUND Found
```

Expands to the HTTP 302 found reason-phrase tokens.

**Value:** `Found`

**Source:** [include/protocol/http/v11/reason_phrases.h:94](../../../../../include/protocol/http/v11/reason_phrases.h#L94)

<a id="macro-rp-303-see-other"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_303_SEE_OTHER

**MACRO** · *protocol::http::v11*

```cpp
#define RP_303_SEE_OTHER See Other
```

Expands to the HTTP 303 see other reason-phrase tokens.

**Value:** `See Other`

**Source:** [include/protocol/http/v11/reason_phrases.h:95](../../../../../include/protocol/http/v11/reason_phrases.h#L95)

<a id="macro-rp-304-not-modified"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_304_NOT_MODIFIED

**MACRO** · *protocol::http::v11*

```cpp
#define RP_304_NOT_MODIFIED Not Modified
```

Expands to the HTTP 304 not modified reason-phrase tokens.

**Value:** `Not Modified`

**Source:** [include/protocol/http/v11/reason_phrases.h:96](../../../../../include/protocol/http/v11/reason_phrases.h#L96)

<a id="macro-rp-305-use-proxy"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_305_USE_PROXY

**MACRO** · *protocol::http::v11*

```cpp
#define RP_305_USE_PROXY Use Proxy
```

Expands to the HTTP 305 use proxy reason-phrase tokens.

**Value:** `Use Proxy`

**Source:** [include/protocol/http/v11/reason_phrases.h:97](../../../../../include/protocol/http/v11/reason_phrases.h#L97)

<a id="macro-rp-306-unused"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_306_UNUSED

**MACRO** · *protocol::http::v11*

```cpp
#define RP_306_UNUSED Unused
```

Expands to the HTTP 306 unused reason-phrase tokens.

**Value:** `Unused`

**Source:** [include/protocol/http/v11/reason_phrases.h:98](../../../../../include/protocol/http/v11/reason_phrases.h#L98)

<a id="macro-rp-307-temporary-redirect"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_307_TEMPORARY_REDIRECT

**MACRO** · *protocol::http::v11*

```cpp
#define RP_307_TEMPORARY_REDIRECT Temporary Redirect
```

Expands to the HTTP 307 temporary redirect reason-phrase tokens.

**Value:** `Temporary Redirect`

**Source:** [include/protocol/http/v11/reason_phrases.h:99](../../../../../include/protocol/http/v11/reason_phrases.h#L99)

<a id="macro-rp-308-permanent-redirect"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_308_PERMANENT_REDIRECT

**MACRO** · *protocol::http::v11*

```cpp
#define RP_308_PERMANENT_REDIRECT Permanent Redirect
```

Expands to the HTTP 308 permanent redirect reason-phrase tokens.

**Value:** `Permanent Redirect`

**Source:** [include/protocol/http/v11/reason_phrases.h:100](../../../../../include/protocol/http/v11/reason_phrases.h#L100)

<a id="macro-rp-400-bad-request"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_400_BAD_REQUEST

**MACRO** · *protocol::http::v11*

```cpp
#define RP_400_BAD_REQUEST Bad Request
```

Expands to the HTTP 400 bad request reason-phrase tokens.

**Value:** `Bad Request`

**Source:** [include/protocol/http/v11/reason_phrases.h:101](../../../../../include/protocol/http/v11/reason_phrases.h#L101)

<a id="macro-rp-401-unauthorized"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_401_UNAUTHORIZED

**MACRO** · *protocol::http::v11*

```cpp
#define RP_401_UNAUTHORIZED Unauthorized
```

Expands to the HTTP 401 unauthorized reason-phrase tokens.

**Value:** `Unauthorized`

**Source:** [include/protocol/http/v11/reason_phrases.h:102](../../../../../include/protocol/http/v11/reason_phrases.h#L102)

<a id="macro-rp-402-payment-required"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_402_PAYMENT_REQUIRED

**MACRO** · *protocol::http::v11*

```cpp
#define RP_402_PAYMENT_REQUIRED Payment Required
```

Expands to the HTTP 402 payment required reason-phrase tokens.

**Value:** `Payment Required`

**Source:** [include/protocol/http/v11/reason_phrases.h:103](../../../../../include/protocol/http/v11/reason_phrases.h#L103)

<a id="macro-rp-403-forbidden"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_403_FORBIDDEN

**MACRO** · *protocol::http::v11*

```cpp
#define RP_403_FORBIDDEN Forbidden
```

Expands to the HTTP 403 forbidden reason-phrase tokens.

**Value:** `Forbidden`

**Source:** [include/protocol/http/v11/reason_phrases.h:104](../../../../../include/protocol/http/v11/reason_phrases.h#L104)

<a id="macro-rp-404-not-found"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_404_NOT_FOUND

**MACRO** · *protocol::http::v11*

```cpp
#define RP_404_NOT_FOUND Not Found
```

Expands to the HTTP 404 not found reason-phrase tokens.

**Value:** `Not Found`

**Source:** [include/protocol/http/v11/reason_phrases.h:105](../../../../../include/protocol/http/v11/reason_phrases.h#L105)

<a id="macro-rp-405-method-not-allowed"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_405_METHOD_NOT_ALLOWED

**MACRO** · *protocol::http::v11*

```cpp
#define RP_405_METHOD_NOT_ALLOWED Method Not Allowed
```

Expands to the HTTP 405 method not allowed reason-phrase tokens.

**Value:** `Method Not Allowed`

**Source:** [include/protocol/http/v11/reason_phrases.h:106](../../../../../include/protocol/http/v11/reason_phrases.h#L106)

<a id="macro-rp-406-not-acceptable"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_406_NOT_ACCEPTABLE

**MACRO** · *protocol::http::v11*

```cpp
#define RP_406_NOT_ACCEPTABLE Not Acceptable
```

Expands to the HTTP 406 not acceptable reason-phrase tokens.

**Value:** `Not Acceptable`

**Source:** [include/protocol/http/v11/reason_phrases.h:107](../../../../../include/protocol/http/v11/reason_phrases.h#L107)

<a id="macro-rp-407-proxy-authentication-required"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_407_PROXY_AUTHENTICATION_REQUIRED

**MACRO** · *protocol::http::v11*

```cpp
#define RP_407_PROXY_AUTHENTICATION_REQUIRED Proxy Authentication Required
```

Expands to the HTTP 407 proxy authentication required reason-phrase tokens.

**Value:** `Proxy Authentication Required`

**Source:** [include/protocol/http/v11/reason_phrases.h:108](../../../../../include/protocol/http/v11/reason_phrases.h#L108)

<a id="macro-rp-408-request-timeout"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_408_REQUEST_TIMEOUT

**MACRO** · *protocol::http::v11*

```cpp
#define RP_408_REQUEST_TIMEOUT Request Timeout
```

Expands to the HTTP 408 request timeout reason-phrase tokens.

**Value:** `Request Timeout`

**Source:** [include/protocol/http/v11/reason_phrases.h:109](../../../../../include/protocol/http/v11/reason_phrases.h#L109)

<a id="macro-rp-409-conflict"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_409_CONFLICT

**MACRO** · *protocol::http::v11*

```cpp
#define RP_409_CONFLICT Conflict
```

Expands to the HTTP 409 conflict reason-phrase tokens.

**Value:** `Conflict`

**Source:** [include/protocol/http/v11/reason_phrases.h:110](../../../../../include/protocol/http/v11/reason_phrases.h#L110)

<a id="macro-rp-410-gone"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_410_GONE

**MACRO** · *protocol::http::v11*

```cpp
#define RP_410_GONE Gone
```

Expands to the HTTP 410 gone reason-phrase tokens.

**Value:** `Gone`

**Source:** [include/protocol/http/v11/reason_phrases.h:111](../../../../../include/protocol/http/v11/reason_phrases.h#L111)

<a id="macro-rp-411-length-required"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_411_LENGTH_REQUIRED

**MACRO** · *protocol::http::v11*

```cpp
#define RP_411_LENGTH_REQUIRED Length Required
```

Expands to the HTTP 411 length required reason-phrase tokens.

**Value:** `Length Required`

**Source:** [include/protocol/http/v11/reason_phrases.h:112](../../../../../include/protocol/http/v11/reason_phrases.h#L112)

<a id="macro-rp-412-precondition-failed"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_412_PRECONDITION_FAILED

**MACRO** · *protocol::http::v11*

```cpp
#define RP_412_PRECONDITION_FAILED Precondition Failed
```

Expands to the HTTP 412 precondition failed reason-phrase tokens.

**Value:** `Precondition Failed`

**Source:** [include/protocol/http/v11/reason_phrases.h:113](../../../../../include/protocol/http/v11/reason_phrases.h#L113)

<a id="macro-rp-413-content-too-large"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_413_CONTENT_TOO_LARGE

**MACRO** · *protocol::http::v11*

```cpp
#define RP_413_CONTENT_TOO_LARGE Content Too Large
```

Expands to the HTTP 413 content too large reason-phrase tokens.

**Value:** `Content Too Large`

**Source:** [include/protocol/http/v11/reason_phrases.h:114](../../../../../include/protocol/http/v11/reason_phrases.h#L114)

<a id="macro-rp-414-uri-too-long"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_414_URI_TOO_LONG

**MACRO** · *protocol::http::v11*

```cpp
#define RP_414_URI_TOO_LONG URI Too Long
```

Expands to the HTTP 414 uri too long reason-phrase tokens.

**Value:** `URI Too Long`

**Source:** [include/protocol/http/v11/reason_phrases.h:115](../../../../../include/protocol/http/v11/reason_phrases.h#L115)

<a id="macro-rp-415-unsupported-media-type"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_415_UNSUPPORTED_MEDIA_TYPE

**MACRO** · *protocol::http::v11*

```cpp
#define RP_415_UNSUPPORTED_MEDIA_TYPE Unsupported Media Type
```

Expands to the HTTP 415 unsupported media type reason-phrase tokens.

**Value:** `Unsupported Media Type`

**Source:** [include/protocol/http/v11/reason_phrases.h:116](../../../../../include/protocol/http/v11/reason_phrases.h#L116)

<a id="macro-rp-416-range-not-satisfiable"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_416_RANGE_NOT_SATISFIABLE

**MACRO** · *protocol::http::v11*

```cpp
#define RP_416_RANGE_NOT_SATISFIABLE Range Not Satisfiable
```

Expands to the HTTP 416 range not satisfiable reason-phrase tokens.

**Value:** `Range Not Satisfiable`

**Source:** [include/protocol/http/v11/reason_phrases.h:117](../../../../../include/protocol/http/v11/reason_phrases.h#L117)

<a id="macro-rp-417-expectation-failed"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_417_EXPECTATION_FAILED

**MACRO** · *protocol::http::v11*

```cpp
#define RP_417_EXPECTATION_FAILED Expectation Failed
```

Expands to the HTTP 417 expectation failed reason-phrase tokens.

**Value:** `Expectation Failed`

**Source:** [include/protocol/http/v11/reason_phrases.h:118](../../../../../include/protocol/http/v11/reason_phrases.h#L118)

<a id="macro-rp-418-im-a-teapot"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_418_IM_A_TEAPOT

**MACRO** · *protocol::http::v11*

```cpp
#define RP_418_IM_A_TEAPOT Im a teapot
```

Expands to the HTTP 418 im a teapot reason-phrase tokens.

**Value:** `Im a teapot`

**Source:** [include/protocol/http/v11/reason_phrases.h:119](../../../../../include/protocol/http/v11/reason_phrases.h#L119)

<a id="macro-rp-421-misdirected-request"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_421_MISDIRECTED_REQUEST

**MACRO** · *protocol::http::v11*

```cpp
#define RP_421_MISDIRECTED_REQUEST Misdirected Request
```

Expands to the HTTP 421 misdirected request reason-phrase tokens.

**Value:** `Misdirected Request`

**Source:** [include/protocol/http/v11/reason_phrases.h:120](../../../../../include/protocol/http/v11/reason_phrases.h#L120)

<a id="macro-rp-422-unprocessable-content"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_422_UNPROCESSABLE_CONTENT

**MACRO** · *protocol::http::v11*

```cpp
#define RP_422_UNPROCESSABLE_CONTENT Unprocessable Content
```

Expands to the HTTP 422 unprocessable content reason-phrase tokens.

**Value:** `Unprocessable Content`

**Source:** [include/protocol/http/v11/reason_phrases.h:121](../../../../../include/protocol/http/v11/reason_phrases.h#L121)

<a id="macro-rp-426-upgrade-required"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_426_UPGRADE_REQUIRED

**MACRO** · *protocol::http::v11*

```cpp
#define RP_426_UPGRADE_REQUIRED Upgrade Required
```

Expands to the HTTP 426 upgrade required reason-phrase tokens.

**Value:** `Upgrade Required`

**Source:** [include/protocol/http/v11/reason_phrases.h:122](../../../../../include/protocol/http/v11/reason_phrases.h#L122)

<a id="macro-rp-431-request-header-fields-too-large"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_431_REQUEST_HEADER_FIELDS_TOO_LARGE

**MACRO** · *protocol::http::v11*

```cpp
#define RP_431_REQUEST_HEADER_FIELDS_TOO_LARGE Request Header Fields Too Large
```

Expands to the HTTP 431 request header fields too large reason-phrase tokens.

**Value:** `Request Header Fields Too Large`

**Source:** [include/protocol/http/v11/reason_phrases.h:123](../../../../../include/protocol/http/v11/reason_phrases.h#L123)

<a id="macro-rp-500-internal-server-error"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_500_INTERNAL_SERVER_ERROR

**MACRO** · *protocol::http::v11*

```cpp
#define RP_500_INTERNAL_SERVER_ERROR Internal Server Error
```

Expands to the HTTP 500 internal server error reason-phrase tokens.

**Value:** `Internal Server Error`

**Source:** [include/protocol/http/v11/reason_phrases.h:124](../../../../../include/protocol/http/v11/reason_phrases.h#L124)

<a id="macro-rp-501-not-implemented"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_501_NOT_IMPLEMENTED

**MACRO** · *protocol::http::v11*

```cpp
#define RP_501_NOT_IMPLEMENTED Not Implemented
```

Expands to the HTTP 501 not implemented reason-phrase tokens.

**Value:** `Not Implemented`

**Source:** [include/protocol/http/v11/reason_phrases.h:125](../../../../../include/protocol/http/v11/reason_phrases.h#L125)

<a id="macro-rp-502-bad-gateway"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_502_BAD_GATEWAY

**MACRO** · *protocol::http::v11*

```cpp
#define RP_502_BAD_GATEWAY Bad Gateway
```

Expands to the HTTP 502 bad gateway reason-phrase tokens.

**Value:** `Bad Gateway`

**Source:** [include/protocol/http/v11/reason_phrases.h:126](../../../../../include/protocol/http/v11/reason_phrases.h#L126)

<a id="macro-rp-503-service-unavailable"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_503_SERVICE_UNAVAILABLE

**MACRO** · *protocol::http::v11*

```cpp
#define RP_503_SERVICE_UNAVAILABLE Service Unavailable
```

Expands to the HTTP 503 service unavailable reason-phrase tokens.

**Value:** `Service Unavailable`

**Source:** [include/protocol/http/v11/reason_phrases.h:127](../../../../../include/protocol/http/v11/reason_phrases.h#L127)

<a id="macro-rp-504-gateway-timeout"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_504_GATEWAY_TIMEOUT

**MACRO** · *protocol::http::v11*

```cpp
#define RP_504_GATEWAY_TIMEOUT Gateway Timeout
```

Expands to the HTTP 504 gateway timeout reason-phrase tokens.

**Value:** `Gateway Timeout`

**Source:** [include/protocol/http/v11/reason_phrases.h:128](../../../../../include/protocol/http/v11/reason_phrases.h#L128)

<a id="macro-rp-505-http-version-not-supported"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> RP_505_HTTP_VERSION_NOT_SUPPORTED

**MACRO** · *protocol::http::v11*

```cpp
#define RP_505_HTTP_VERSION_NOT_SUPPORTED HTTP Version Not Supported
```

Expands to the HTTP 505 http version not supported reason-phrase tokens.

**Value:** `HTTP Version Not Supported`

**Source:** [include/protocol/http/v11/reason_phrases.h:129](../../../../../include/protocol/http/v11/reason_phrases.h#L129)
