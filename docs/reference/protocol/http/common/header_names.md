<a id="header-names-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../resources/docs/reference/protocol/http/common/header_names/index-dark.svg">
    <img src="../../../../../resources/docs/reference/protocol/http/common/header_names/index.svg" alt="Index for header_names.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

Provides the http 1.1 header names specification.

**protocol::http**

- [header_names](#type-header-names) · *type*

**protocol::http::header_names**

- [kConnection](#constant-kconnection) · *constant*
- [kDate](#constant-kdate) · *constant*
- [kVia](#constant-kvia) · *constant*
- [kCacheControl](#constant-kcachecontrol) · *constant*
- [kPragma](#constant-kpragma) · *constant*
- [kWarning](#constant-kwarning) · *constant*
- [kHost](#constant-khost) · *constant*
- [kUserAgent](#constant-kuseragent) · *constant*
- [kAccept](#constant-kaccept) · *constant*
- [kAcceptEncoding](#constant-kacceptencoding) · *constant*
- [kAcceptLanguage](#constant-kacceptlanguage) · *constant*
- [kAuthorization](#constant-kauthorization) · *constant*
- [kExpect](#constant-kexpect) · *constant*
- [kFrom](#constant-kfrom) · *constant*
- [kIfMatch](#constant-kifmatch) · *constant*
- [kIfNoneMatch](#constant-kifnonematch) · *constant*
- [kIfModifiedSince](#constant-kifmodifiedsince) · *constant*
- [kIfUnmodifiedSince](#constant-kifunmodifiedsince) · *constant*
- [kRange](#constant-krange) · *constant*
- [kReferer](#constant-kreferer) · *constant*
- [kTe](#constant-kte) · *constant*
- [kUpgrade](#constant-kupgrade) · *constant*
- [kCookie](#constant-kcookie) · *constant*
- [kLocation](#constant-klocation) · *constant*
- [kServer](#constant-kserver) · *constant*
- [kVary](#constant-kvary) · *constant*
- [kWwwAuthenticate](#constant-kwwwauthenticate) · *constant*
- [kContentLength](#constant-kcontentlength) · *constant*
- [kContentType](#constant-kcontenttype) · *constant*
- [kContentEncoding](#constant-kcontentencoding) · *constant*
- [kContentLanguage](#constant-kcontentlanguage) · *constant*
- [kContentRange](#constant-kcontentrange) · *constant*
- [kTrailer](#constant-ktrailer) · *constant*
- [kTransferEncoding](#constant-ktransferencoding) · *constant*
- [kAllow](#constant-kallow) · *constant*
- [kRetryAfter](#constant-kretryafter) · *constant*
- [kAcceptRanges](#constant-kacceptranges) · *constant*
- [kETag](#constant-ketag) · *constant*
- [kLastModified](#constant-klastmodified) · *constant*

<a id="type-header-names"></a>

## header_names

🟠 **TYPE** · *protocol::http*

**Declaration synopsis:**

```cpp
struct header_names { ... };
```

Provides the http 1.1 header names specification.

**Source:** [include/protocol/http/common/header_names.h:36](../../../../../include/protocol/http/common/header_names.h#L36)

<a id="constant-kconnection"></a>

## kConnection

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kConnection[] = "Connection";
```

Header          |       Seccion      |          Tipo Connection             | RFC 9110 S7.6.1    | General Date                   | RFC 9110 S8.6.1    | General Via                    | RFC 9110 S7.6.3    | General Cache-Control          | RFC 9111 S5.2      | General (caching) Pragma                 | RFC 9111 S5.4      | General (legacy caching)

**Value:** `"Connection"`

**Source:** [include/protocol/http/common/header_names.h:83](../../../../../include/protocol/http/common/header_names.h#L83)

<a id="constant-kdate"></a>

## kDate

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kDate[] = "Date";
```

Defines the date constant.

**Value:** `"Date"`

**Source:** [include/protocol/http/common/header_names.h:84](../../../../../include/protocol/http/common/header_names.h#L84)

<a id="constant-kvia"></a>

## kVia

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kVia[] = "Via";
```

Defines the via constant.

**Value:** `"Via"`

**Source:** [include/protocol/http/common/header_names.h:85](../../../../../include/protocol/http/common/header_names.h#L85)

<a id="constant-kcachecontrol"></a>

## kCacheControl

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kCacheControl[] = "Cache-Control";
```

Defines the cache control constant.

**Value:** `"Cache-Control"`

**Source:** [include/protocol/http/common/header_names.h:86](../../../../../include/protocol/http/common/header_names.h#L86)

<a id="constant-kpragma"></a>

## kPragma

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kPragma[] = "Pragma";
```

Defines the pragma constant.

**Value:** `"Pragma"`

**Source:** [include/protocol/http/common/header_names.h:87](../../../../../include/protocol/http/common/header_names.h#L87)

<a id="constant-kwarning"></a>

## kWarning

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kWarning[] = "Warning";
```

Defines the warning constant.

**Value:** `"Warning"`

**Source:** [include/protocol/http/common/header_names.h:88](../../../../../include/protocol/http/common/header_names.h#L88)

<a id="constant-khost"></a>

## kHost

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kHost[] = "Host";
```

Defines the host constant.

**Value:** `"Host"`

**Source:** [include/protocol/http/common/header_names.h:89](../../../../../include/protocol/http/common/header_names.h#L89)

<a id="constant-kuseragent"></a>

## kUserAgent

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kUserAgent[] = "User-agent";
```

Defines the user agent constant.

**Value:** `"User-agent"`

**Source:** [include/protocol/http/common/header_names.h:90](../../../../../include/protocol/http/common/header_names.h#L90)

<a id="constant-kaccept"></a>

## kAccept

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kAccept[] = "Accept";
```

Defines the accept constant.

**Value:** `"Accept"`

**Source:** [include/protocol/http/common/header_names.h:91](../../../../../include/protocol/http/common/header_names.h#L91)

<a id="constant-kacceptencoding"></a>

## kAcceptEncoding

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kAcceptEncoding[] = "Accept-Encoding";
```

Defines the accept encoding constant.

**Value:** `"Accept-Encoding"`

**Source:** [include/protocol/http/common/header_names.h:92](../../../../../include/protocol/http/common/header_names.h#L92)

<a id="constant-kacceptlanguage"></a>

## kAcceptLanguage

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kAcceptLanguage[] = "Accept-Language";
```

Defines the accept language constant.

**Value:** `"Accept-Language"`

**Source:** [include/protocol/http/common/header_names.h:93](../../../../../include/protocol/http/common/header_names.h#L93)

<a id="constant-kauthorization"></a>

## kAuthorization

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kAuthorization[] = "Authorization";
```

Defines the authorization constant.

**Value:** `"Authorization"`

**Source:** [include/protocol/http/common/header_names.h:94](../../../../../include/protocol/http/common/header_names.h#L94)

<a id="constant-kexpect"></a>

## kExpect

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kExpect[] = "Expect";
```

Defines the expect constant.

**Value:** `"Expect"`

**Source:** [include/protocol/http/common/header_names.h:95](../../../../../include/protocol/http/common/header_names.h#L95)

<a id="constant-kfrom"></a>

## kFrom

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kFrom[] = "From";
```

Defines the from constant.

**Value:** `"From"`

**Source:** [include/protocol/http/common/header_names.h:96](../../../../../include/protocol/http/common/header_names.h#L96)

<a id="constant-kifmatch"></a>

## kIfMatch

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kIfMatch[] = "If-Match";
```

Defines the if match constant.

**Value:** `"If-Match"`

**Source:** [include/protocol/http/common/header_names.h:97](../../../../../include/protocol/http/common/header_names.h#L97)

<a id="constant-kifnonematch"></a>

## kIfNoneMatch

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kIfNoneMatch[] = "If-None-Match";
```

Defines the if none match constant.

**Value:** `"If-None-Match"`

**Source:** [include/protocol/http/common/header_names.h:98](../../../../../include/protocol/http/common/header_names.h#L98)

<a id="constant-kifmodifiedsince"></a>

## kIfModifiedSince

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kIfModifiedSince[] = "If-Modified-Since";
```

Defines the if modified since constant.

**Value:** `"If-Modified-Since"`

**Source:** [include/protocol/http/common/header_names.h:99](../../../../../include/protocol/http/common/header_names.h#L99)

<a id="constant-kifunmodifiedsince"></a>

## kIfUnmodifiedSince

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kIfUnmodifiedSince[] = "If-Unmodified-Since";
```

Defines the if unmodified since constant.

**Value:** `"If-Unmodified-Since"`

**Source:** [include/protocol/http/common/header_names.h:100](../../../../../include/protocol/http/common/header_names.h#L100)

<a id="constant-krange"></a>

## kRange

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kRange[] = "Range";
```

Defines the range constant.

**Value:** `"Range"`

**Source:** [include/protocol/http/common/header_names.h:101](../../../../../include/protocol/http/common/header_names.h#L101)

<a id="constant-kreferer"></a>

## kReferer

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kReferer[] = "Referer";
```

Defines the referer constant.

**Value:** `"Referer"`

**Source:** [include/protocol/http/common/header_names.h:102](../../../../../include/protocol/http/common/header_names.h#L102)

<a id="constant-kte"></a>

## kTe

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kTe[] = "TE";
```

Defines the te constant.

**Value:** `"TE"`

**Source:** [include/protocol/http/common/header_names.h:103](../../../../../include/protocol/http/common/header_names.h#L103)

<a id="constant-kupgrade"></a>

## kUpgrade

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kUpgrade[] = "Upgrade";
```

Defines the upgrade constant.

**Value:** `"Upgrade"`

**Source:** [include/protocol/http/common/header_names.h:104](../../../../../include/protocol/http/common/header_names.h#L104)

<a id="constant-kcookie"></a>

## kCookie

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kCookie[] = "Cookie";
```

Defines the cookie constant.

**Value:** `"Cookie"`

**Source:** [include/protocol/http/common/header_names.h:105](../../../../../include/protocol/http/common/header_names.h#L105)

<a id="constant-klocation"></a>

## kLocation

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kLocation[] = "Location";
```

Defines the location constant.

**Value:** `"Location"`

**Source:** [include/protocol/http/common/header_names.h:106](../../../../../include/protocol/http/common/header_names.h#L106)

<a id="constant-kserver"></a>

## kServer

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kServer[] = "Server";
```

Defines the server constant.

**Value:** `"Server"`

**Source:** [include/protocol/http/common/header_names.h:107](../../../../../include/protocol/http/common/header_names.h#L107)

<a id="constant-kvary"></a>

## kVary

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kVary[] = "Vary";
```

Defines the vary constant.

**Value:** `"Vary"`

**Source:** [include/protocol/http/common/header_names.h:108](../../../../../include/protocol/http/common/header_names.h#L108)

<a id="constant-kwwwauthenticate"></a>

## kWwwAuthenticate

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kWwwAuthenticate[] = "WWW-Authenticate";
```

Defines the www authenticate constant.

**Value:** `"WWW-Authenticate"`

**Source:** [include/protocol/http/common/header_names.h:109](../../../../../include/protocol/http/common/header_names.h#L109)

<a id="constant-kcontentlength"></a>

## kContentLength

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kContentLength[] = "Content-Length";
```

Defines the content length constant.

**Value:** `"Content-Length"`

**Source:** [include/protocol/http/common/header_names.h:110](../../../../../include/protocol/http/common/header_names.h#L110)

<a id="constant-kcontenttype"></a>

## kContentType

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kContentType[] = "Content-Type";
```

Defines the content type constant.

**Value:** `"Content-Type"`

**Source:** [include/protocol/http/common/header_names.h:111](../../../../../include/protocol/http/common/header_names.h#L111)

<a id="constant-kcontentencoding"></a>

## kContentEncoding

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kContentEncoding[] = "Content-Encoding";
```

Defines the content encoding constant.

**Value:** `"Content-Encoding"`

**Source:** [include/protocol/http/common/header_names.h:112](../../../../../include/protocol/http/common/header_names.h#L112)

<a id="constant-kcontentlanguage"></a>

## kContentLanguage

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kContentLanguage[] = "Content-Language";
```

Defines the content language constant.

**Value:** `"Content-Language"`

**Source:** [include/protocol/http/common/header_names.h:113](../../../../../include/protocol/http/common/header_names.h#L113)

<a id="constant-kcontentrange"></a>

## kContentRange

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kContentRange[] = "Content-Range";
```

Defines the content range constant.

**Value:** `"Content-Range"`

**Source:** [include/protocol/http/common/header_names.h:114](../../../../../include/protocol/http/common/header_names.h#L114)

<a id="constant-ktrailer"></a>

## kTrailer

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kTrailer[] = "Trailer";
```

Defines the trailer constant.

**Value:** `"Trailer"`

**Source:** [include/protocol/http/common/header_names.h:115](../../../../../include/protocol/http/common/header_names.h#L115)

<a id="constant-ktransferencoding"></a>

## kTransferEncoding

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kTransferEncoding[] = "Transfer-Encoding";
```

Defines the transfer encoding constant.

**Value:** `"Transfer-Encoding"`

**Source:** [include/protocol/http/common/header_names.h:116](../../../../../include/protocol/http/common/header_names.h#L116)

<a id="constant-kallow"></a>

## kAllow

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kAllow[] = "Allow";
```

Defines the allow constant.

**Value:** `"Allow"`

**Source:** [include/protocol/http/common/header_names.h:117](../../../../../include/protocol/http/common/header_names.h#L117)

<a id="constant-kretryafter"></a>

## kRetryAfter

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kRetryAfter[] = "Retry-After";
```

Defines the retry after constant.

**Value:** `"Retry-After"`

**Source:** [include/protocol/http/common/header_names.h:118](../../../../../include/protocol/http/common/header_names.h#L118)

<a id="constant-kacceptranges"></a>

## kAcceptRanges

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kAcceptRanges[] = "Accept-Ranges";
```

Defines the accept ranges constant.

**Value:** `"Accept-Ranges"`

**Source:** [include/protocol/http/common/header_names.h:119](../../../../../include/protocol/http/common/header_names.h#L119)

<a id="constant-ketag"></a>

## kETag

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kETag[] = "ETAG";
```

Defines the etag constant.

**Value:** `"ETAG"`

**Source:** [include/protocol/http/common/header_names.h:120](../../../../../include/protocol/http/common/header_names.h#L120)

<a id="constant-klastmodified"></a>

## kLastModified

🟠 **CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kLastModified[] = "Last-Modified";
```

Defines the last modified constant.

**Value:** `"Last-Modified"`

**Source:** [include/protocol/http/common/header_names.h:121](../../../../../include/protocol/http/common/header_names.h#L121)
