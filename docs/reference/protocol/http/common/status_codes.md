<a id="status-codes-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../resources/docs/reference/protocol/http/common/status_codes/index-dark.svg">
    <img src="../../../../../resources/docs/reference/protocol/http/common/status_codes/index.svg" alt="Index for status_codes.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/status_codes.h](../../../../../include/protocol/http/common/status_codes.h)

Defines the preprocessor values listed below.

**protocol::http**

- [SC_100_CONTINUE](#macro-sc-100-continue) · *macro*
- [SC_101_SWITCHING_PROTOCOLS](#macro-sc-101-switching-protocols) · *macro*
- [SC_200_OK](#macro-sc-200-ok) · *macro*
- [SC_201_CREATED](#macro-sc-201-created) · *macro*
- [SC_202_ACCEPTED](#macro-sc-202-accepted) · *macro*
- [SC_203_NON_AUTHORITATIVE_INFORMATION](#macro-sc-203-non-authoritative-information) · *macro*
- [SC_204_NO_CONTENT](#macro-sc-204-no-content) · *macro*
- [SC_205_RESET_CONTENT](#macro-sc-205-reset-content) · *macro*
- [SC_206_PARTIAL_CONTENT](#macro-sc-206-partial-content) · *macro*
- [SC_300_MULTIPLE_CHOICES](#macro-sc-300-multiple-choices) · *macro*
- [SC_301_MOVED_PERMANENTLY](#macro-sc-301-moved-permanently) · *macro*
- [SC_302_FOUND](#macro-sc-302-found) · *macro*
- [SC_303_SEE_OTHER](#macro-sc-303-see-other) · *macro*
- [SC_304_NOT_MODIFIED](#macro-sc-304-not-modified) · *macro*
- [SC_305_USE_PROXY](#macro-sc-305-use-proxy) · *macro*
- [SC_306_UNUSED](#macro-sc-306-unused) · *macro*
- [SC_307_TEMPORARY_REDIRECT](#macro-sc-307-temporary-redirect) · *macro*
- [SC_308_PERMANENT_REDIRECT](#macro-sc-308-permanent-redirect) · *macro*
- [SC_400_BAD_REQUEST](#macro-sc-400-bad-request) · *macro*
- [SC_401_UNAUTHORIZED](#macro-sc-401-unauthorized) · *macro*
- [SC_402_PAYMENT_REQUIRED](#macro-sc-402-payment-required) · *macro*
- [SC_403_FORBIDDEN](#macro-sc-403-forbidden) · *macro*
- [SC_404_NOT_FOUND](#macro-sc-404-not-found) · *macro*
- [SC_405_METHOD_NOT_ALLOWED](#macro-sc-405-method-not-allowed) · *macro*
- [SC_406_NOT_ACCEPTABLE](#macro-sc-406-not-acceptable) · *macro*
- [SC_407_PROXY_AUTHENTICATION_REQUIRED](#macro-sc-407-proxy-authentication-required) · *macro*
- [SC_408_REQUEST_TIMEOUT](#macro-sc-408-request-timeout) · *macro*
- [SC_409_CONFLICT](#macro-sc-409-conflict) · *macro*
- [SC_410_GONE](#macro-sc-410-gone) · *macro*
- [SC_411_LENGTH_REQUIRED](#macro-sc-411-length-required) · *macro*
- [SC_412_PRECONDITION_FAILED](#macro-sc-412-precondition-failed) · *macro*
- [SC_413_CONTENT_TOO_LARGE](#macro-sc-413-content-too-large) · *macro*
- [SC_414_URI_TOO_LONG](#macro-sc-414-uri-too-long) · *macro*
- [SC_415_UNSUPPORTED_MEDIA_TYPE](#macro-sc-415-unsupported-media-type) · *macro*
- [SC_416_RANGE_NOT_SATISFIABLE](#macro-sc-416-range-not-satisfiable) · *macro*
- [SC_417_EXPECTATION_FAILED](#macro-sc-417-expectation-failed) · *macro*
- [SC_418_IM_A_TEAPOT](#macro-sc-418-im-a-teapot) · *macro*
- [SC_421_MISDIRECTED_REQUEST](#macro-sc-421-misdirected-request) · *macro*
- [SC_422_UNPROCESSABLE_CONTENT](#macro-sc-422-unprocessable-content) · *macro*
- [SC_426_UPGRADE_REQUIRED](#macro-sc-426-upgrade-required) · *macro*
- [SC_431_REQUEST_HEADER_FIELDS_TOO_LARGE](#macro-sc-431-request-header-fields-too-large) · *macro*
- [SC_500_INTERNAL_SERVER_ERROR](#macro-sc-500-internal-server-error) · *macro*
- [SC_501_NOT_IMPLEMENTED](#macro-sc-501-not-implemented) · *macro*
- [SC_502_BAD_GATEWAY](#macro-sc-502-bad-gateway) · *macro*
- [SC_503_SERVICE_UNAVAILABLE](#macro-sc-503-service-unavailable) · *macro*
- [SC_504_GATEWAY_TIMEOUT](#macro-sc-504-gateway-timeout) · *macro*
- [SC_505_HTTP_VERSION_NOT_SUPPORTED](#macro-sc-505-http-version-not-supported) · *macro*

<a id="macro-sc-100-continue"></a>

## SC_100_CONTINUE

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_100_CONTINUE 100
```

Provides the http 1.1 status codes. Code | Reason Phrase 100  | Continue 101  | Switching Protocols 200  | OK 201  | Created 202  | Accepted 203  | Non-Authoritative Information 204  | No Content 205  | Reset Content 206  | Partial Content 300  | Multiple Choices 301  | Moved Permanently 302  | Found 303  | See Other 304  | Not

**Value:** `100`

**Source:** [include/protocol/http/common/status_codes.h:87](../../../../../include/protocol/http/common/status_codes.h#L87)

<a id="macro-sc-101-switching-protocols"></a>

## SC_101_SWITCHING_PROTOCOLS

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_101_SWITCHING_PROTOCOLS 101
```

Defines the HTTP 101 switching protocols status code.

**Value:** `101`

**Source:** [include/protocol/http/common/status_codes.h:88](../../../../../include/protocol/http/common/status_codes.h#L88)

<a id="macro-sc-200-ok"></a>

## SC_200_OK

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_200_OK 200
```

Defines the HTTP 200 ok status code.

**Value:** `200`

**Source:** [include/protocol/http/common/status_codes.h:89](../../../../../include/protocol/http/common/status_codes.h#L89)

<a id="macro-sc-201-created"></a>

## SC_201_CREATED

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_201_CREATED 201
```

Defines the HTTP 201 created status code.

**Value:** `201`

**Source:** [include/protocol/http/common/status_codes.h:90](../../../../../include/protocol/http/common/status_codes.h#L90)

<a id="macro-sc-202-accepted"></a>

## SC_202_ACCEPTED

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_202_ACCEPTED 202
```

Defines the HTTP 202 accepted status code.

**Value:** `202`

**Source:** [include/protocol/http/common/status_codes.h:91](../../../../../include/protocol/http/common/status_codes.h#L91)

<a id="macro-sc-203-non-authoritative-information"></a>

## SC_203_NON_AUTHORITATIVE_INFORMATION

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_203_NON_AUTHORITATIVE_INFORMATION 203
```

Defines the HTTP 203 non authoritative information status code.

**Value:** `203`

**Source:** [include/protocol/http/common/status_codes.h:92](../../../../../include/protocol/http/common/status_codes.h#L92)

<a id="macro-sc-204-no-content"></a>

## SC_204_NO_CONTENT

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_204_NO_CONTENT 204
```

Defines the HTTP 204 no content status code.

**Value:** `204`

**Source:** [include/protocol/http/common/status_codes.h:93](../../../../../include/protocol/http/common/status_codes.h#L93)

<a id="macro-sc-205-reset-content"></a>

## SC_205_RESET_CONTENT

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_205_RESET_CONTENT 205
```

Defines the HTTP 205 reset content status code.

**Value:** `205`

**Source:** [include/protocol/http/common/status_codes.h:94](../../../../../include/protocol/http/common/status_codes.h#L94)

<a id="macro-sc-206-partial-content"></a>

## SC_206_PARTIAL_CONTENT

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_206_PARTIAL_CONTENT 206
```

Defines the HTTP 206 partial content status code.

**Value:** `206`

**Source:** [include/protocol/http/common/status_codes.h:95](../../../../../include/protocol/http/common/status_codes.h#L95)

<a id="macro-sc-300-multiple-choices"></a>

## SC_300_MULTIPLE_CHOICES

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_300_MULTIPLE_CHOICES 300
```

Defines the HTTP 300 multiple choices status code.

**Value:** `300`

**Source:** [include/protocol/http/common/status_codes.h:96](../../../../../include/protocol/http/common/status_codes.h#L96)

<a id="macro-sc-301-moved-permanently"></a>

## SC_301_MOVED_PERMANENTLY

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_301_MOVED_PERMANENTLY 301
```

Defines the HTTP 301 moved permanently status code.

**Value:** `301`

**Source:** [include/protocol/http/common/status_codes.h:97](../../../../../include/protocol/http/common/status_codes.h#L97)

<a id="macro-sc-302-found"></a>

## SC_302_FOUND

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_302_FOUND 302
```

Defines the HTTP 302 found status code.

**Value:** `302`

**Source:** [include/protocol/http/common/status_codes.h:98](../../../../../include/protocol/http/common/status_codes.h#L98)

<a id="macro-sc-303-see-other"></a>

## SC_303_SEE_OTHER

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_303_SEE_OTHER 303
```

Defines the HTTP 303 see other status code.

**Value:** `303`

**Source:** [include/protocol/http/common/status_codes.h:99](../../../../../include/protocol/http/common/status_codes.h#L99)

<a id="macro-sc-304-not-modified"></a>

## SC_304_NOT_MODIFIED

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_304_NOT_MODIFIED 304
```

Defines the HTTP 304 not modified status code.

**Value:** `304`

**Source:** [include/protocol/http/common/status_codes.h:100](../../../../../include/protocol/http/common/status_codes.h#L100)

<a id="macro-sc-305-use-proxy"></a>

## SC_305_USE_PROXY

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_305_USE_PROXY 305
```

Defines the HTTP 305 use proxy status code.

**Value:** `305`

**Source:** [include/protocol/http/common/status_codes.h:101](../../../../../include/protocol/http/common/status_codes.h#L101)

<a id="macro-sc-306-unused"></a>

## SC_306_UNUSED

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_306_UNUSED 306
```

Defines the HTTP 306 unused status code.

**Value:** `306`

**Source:** [include/protocol/http/common/status_codes.h:102](../../../../../include/protocol/http/common/status_codes.h#L102)

<a id="macro-sc-307-temporary-redirect"></a>

## SC_307_TEMPORARY_REDIRECT

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_307_TEMPORARY_REDIRECT 307
```

Defines the HTTP 307 temporary redirect status code.

**Value:** `307`

**Source:** [include/protocol/http/common/status_codes.h:103](../../../../../include/protocol/http/common/status_codes.h#L103)

<a id="macro-sc-308-permanent-redirect"></a>

## SC_308_PERMANENT_REDIRECT

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_308_PERMANENT_REDIRECT 308
```

Defines the HTTP 308 permanent redirect status code.

**Value:** `308`

**Source:** [include/protocol/http/common/status_codes.h:104](../../../../../include/protocol/http/common/status_codes.h#L104)

<a id="macro-sc-400-bad-request"></a>

## SC_400_BAD_REQUEST

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_400_BAD_REQUEST 400
```

Defines the HTTP 400 bad request status code.

**Value:** `400`

**Source:** [include/protocol/http/common/status_codes.h:105](../../../../../include/protocol/http/common/status_codes.h#L105)

<a id="macro-sc-401-unauthorized"></a>

## SC_401_UNAUTHORIZED

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_401_UNAUTHORIZED 401
```

Defines the HTTP 401 unauthorized status code.

**Value:** `401`

**Source:** [include/protocol/http/common/status_codes.h:106](../../../../../include/protocol/http/common/status_codes.h#L106)

<a id="macro-sc-402-payment-required"></a>

## SC_402_PAYMENT_REQUIRED

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_402_PAYMENT_REQUIRED 402
```

Defines the HTTP 402 payment required status code.

**Value:** `402`

**Source:** [include/protocol/http/common/status_codes.h:107](../../../../../include/protocol/http/common/status_codes.h#L107)

<a id="macro-sc-403-forbidden"></a>

## SC_403_FORBIDDEN

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_403_FORBIDDEN 403
```

Defines the HTTP 403 forbidden status code.

**Value:** `403`

**Source:** [include/protocol/http/common/status_codes.h:108](../../../../../include/protocol/http/common/status_codes.h#L108)

<a id="macro-sc-404-not-found"></a>

## SC_404_NOT_FOUND

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_404_NOT_FOUND 404
```

Defines the HTTP 404 not found status code.

**Value:** `404`

**Source:** [include/protocol/http/common/status_codes.h:109](../../../../../include/protocol/http/common/status_codes.h#L109)

<a id="macro-sc-405-method-not-allowed"></a>

## SC_405_METHOD_NOT_ALLOWED

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_405_METHOD_NOT_ALLOWED 405
```

Defines the HTTP 405 method not allowed status code.

**Value:** `405`

**Source:** [include/protocol/http/common/status_codes.h:110](../../../../../include/protocol/http/common/status_codes.h#L110)

<a id="macro-sc-406-not-acceptable"></a>

## SC_406_NOT_ACCEPTABLE

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_406_NOT_ACCEPTABLE 406
```

Defines the HTTP 406 not acceptable status code.

**Value:** `406`

**Source:** [include/protocol/http/common/status_codes.h:111](../../../../../include/protocol/http/common/status_codes.h#L111)

<a id="macro-sc-407-proxy-authentication-required"></a>

## SC_407_PROXY_AUTHENTICATION_REQUIRED

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_407_PROXY_AUTHENTICATION_REQUIRED 407
```

Defines the HTTP 407 proxy authentication required status code.

**Value:** `407`

**Source:** [include/protocol/http/common/status_codes.h:112](../../../../../include/protocol/http/common/status_codes.h#L112)

<a id="macro-sc-408-request-timeout"></a>

## SC_408_REQUEST_TIMEOUT

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_408_REQUEST_TIMEOUT 408
```

Defines the HTTP 408 request timeout status code.

**Value:** `408`

**Source:** [include/protocol/http/common/status_codes.h:113](../../../../../include/protocol/http/common/status_codes.h#L113)

<a id="macro-sc-409-conflict"></a>

## SC_409_CONFLICT

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_409_CONFLICT 409
```

Defines the HTTP 409 conflict status code.

**Value:** `409`

**Source:** [include/protocol/http/common/status_codes.h:114](../../../../../include/protocol/http/common/status_codes.h#L114)

<a id="macro-sc-410-gone"></a>

## SC_410_GONE

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_410_GONE 410
```

Defines the HTTP 410 gone status code.

**Value:** `410`

**Source:** [include/protocol/http/common/status_codes.h:115](../../../../../include/protocol/http/common/status_codes.h#L115)

<a id="macro-sc-411-length-required"></a>

## SC_411_LENGTH_REQUIRED

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_411_LENGTH_REQUIRED 411
```

Defines the HTTP 411 length required status code.

**Value:** `411`

**Source:** [include/protocol/http/common/status_codes.h:116](../../../../../include/protocol/http/common/status_codes.h#L116)

<a id="macro-sc-412-precondition-failed"></a>

## SC_412_PRECONDITION_FAILED

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_412_PRECONDITION_FAILED 412
```

Defines the HTTP 412 precondition failed status code.

**Value:** `412`

**Source:** [include/protocol/http/common/status_codes.h:117](../../../../../include/protocol/http/common/status_codes.h#L117)

<a id="macro-sc-413-content-too-large"></a>

## SC_413_CONTENT_TOO_LARGE

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_413_CONTENT_TOO_LARGE 413
```

Defines the HTTP 413 content too large status code.

**Value:** `413`

**Source:** [include/protocol/http/common/status_codes.h:118](../../../../../include/protocol/http/common/status_codes.h#L118)

<a id="macro-sc-414-uri-too-long"></a>

## SC_414_URI_TOO_LONG

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_414_URI_TOO_LONG 414
```

Defines the HTTP 414 uri too long status code.

**Value:** `414`

**Source:** [include/protocol/http/common/status_codes.h:119](../../../../../include/protocol/http/common/status_codes.h#L119)

<a id="macro-sc-415-unsupported-media-type"></a>

## SC_415_UNSUPPORTED_MEDIA_TYPE

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_415_UNSUPPORTED_MEDIA_TYPE 415
```

Defines the HTTP 415 unsupported media type status code.

**Value:** `415`

**Source:** [include/protocol/http/common/status_codes.h:120](../../../../../include/protocol/http/common/status_codes.h#L120)

<a id="macro-sc-416-range-not-satisfiable"></a>

## SC_416_RANGE_NOT_SATISFIABLE

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_416_RANGE_NOT_SATISFIABLE 416
```

Defines the HTTP 416 range not satisfiable status code.

**Value:** `416`

**Source:** [include/protocol/http/common/status_codes.h:121](../../../../../include/protocol/http/common/status_codes.h#L121)

<a id="macro-sc-417-expectation-failed"></a>

## SC_417_EXPECTATION_FAILED

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_417_EXPECTATION_FAILED 417
```

Defines the HTTP 417 expectation failed status code.

**Value:** `417`

**Source:** [include/protocol/http/common/status_codes.h:122](../../../../../include/protocol/http/common/status_codes.h#L122)

<a id="macro-sc-418-im-a-teapot"></a>

## SC_418_IM_A_TEAPOT

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_418_IM_A_TEAPOT 418
```

Defines the HTTP 418 im a teapot status code.

**Value:** `418`

**Source:** [include/protocol/http/common/status_codes.h:123](../../../../../include/protocol/http/common/status_codes.h#L123)

<a id="macro-sc-421-misdirected-request"></a>

## SC_421_MISDIRECTED_REQUEST

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_421_MISDIRECTED_REQUEST 421
```

Defines the HTTP 421 misdirected request status code.

**Value:** `421`

**Source:** [include/protocol/http/common/status_codes.h:124](../../../../../include/protocol/http/common/status_codes.h#L124)

<a id="macro-sc-422-unprocessable-content"></a>

## SC_422_UNPROCESSABLE_CONTENT

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_422_UNPROCESSABLE_CONTENT 422
```

Defines the HTTP 422 unprocessable content status code.

**Value:** `422`

**Source:** [include/protocol/http/common/status_codes.h:125](../../../../../include/protocol/http/common/status_codes.h#L125)

<a id="macro-sc-426-upgrade-required"></a>

## SC_426_UPGRADE_REQUIRED

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_426_UPGRADE_REQUIRED 426
```

Defines the HTTP 426 upgrade required status code.

**Value:** `426`

**Source:** [include/protocol/http/common/status_codes.h:126](../../../../../include/protocol/http/common/status_codes.h#L126)

<a id="macro-sc-431-request-header-fields-too-large"></a>

## SC_431_REQUEST_HEADER_FIELDS_TOO_LARGE

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_431_REQUEST_HEADER_FIELDS_TOO_LARGE 431
```

Defines the HTTP 431 request header fields too large status code.

**Value:** `431`

**Source:** [include/protocol/http/common/status_codes.h:127](../../../../../include/protocol/http/common/status_codes.h#L127)

<a id="macro-sc-500-internal-server-error"></a>

## SC_500_INTERNAL_SERVER_ERROR

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_500_INTERNAL_SERVER_ERROR 500
```

Defines the HTTP 500 internal server error status code.

**Value:** `500`

**Source:** [include/protocol/http/common/status_codes.h:128](../../../../../include/protocol/http/common/status_codes.h#L128)

<a id="macro-sc-501-not-implemented"></a>

## SC_501_NOT_IMPLEMENTED

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_501_NOT_IMPLEMENTED 501
```

Defines the HTTP 501 not implemented status code.

**Value:** `501`

**Source:** [include/protocol/http/common/status_codes.h:129](../../../../../include/protocol/http/common/status_codes.h#L129)

<a id="macro-sc-502-bad-gateway"></a>

## SC_502_BAD_GATEWAY

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_502_BAD_GATEWAY 502
```

Defines the HTTP 502 bad gateway status code.

**Value:** `502`

**Source:** [include/protocol/http/common/status_codes.h:130](../../../../../include/protocol/http/common/status_codes.h#L130)

<a id="macro-sc-503-service-unavailable"></a>

## SC_503_SERVICE_UNAVAILABLE

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_503_SERVICE_UNAVAILABLE 503
```

Defines the HTTP 503 service unavailable status code.

**Value:** `503`

**Source:** [include/protocol/http/common/status_codes.h:131](../../../../../include/protocol/http/common/status_codes.h#L131)

<a id="macro-sc-504-gateway-timeout"></a>

## SC_504_GATEWAY_TIMEOUT

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_504_GATEWAY_TIMEOUT 504
```

Defines the HTTP 504 gateway timeout status code.

**Value:** `504`

**Source:** [include/protocol/http/common/status_codes.h:132](../../../../../include/protocol/http/common/status_codes.h#L132)

<a id="macro-sc-505-http-version-not-supported"></a>

## SC_505_HTTP_VERSION_NOT_SUPPORTED

🟠 **MACRO** · *protocol::http*

```cpp
#define SC_505_HTTP_VERSION_NOT_SUPPORTED 505
```

Defines the HTTP 505 http version not supported status code.

**Value:** `505`

**Source:** [include/protocol/http/common/status_codes.h:133](../../../../../include/protocol/http/common/status_codes.h#L133)
