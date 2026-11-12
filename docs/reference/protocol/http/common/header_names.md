<a id="header-names"></a>
<a href="../../../../../resources/docs/reference/protocol/http/common/header_names/title.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/common/header_names/title.svg" alt="header_names">
</a>
<a href="../../../../../resources/docs/reference/protocol/http/common/header_names/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/common/header_names/title-dark.svg" alt="header_names">
</a>

**Header:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

Provides the http 1.1 header names specification.

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http

[header_names](#type-header-names)

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::header_names

[kConnection](#constant-kconnection) ·
[kDate](#constant-kdate) ·
[kVia](#constant-kvia) ·
[kCacheControl](#constant-kcachecontrol) ·
[kPragma](#constant-kpragma) ·
[kWarning](#constant-kwarning) ·
[kHost](#constant-khost) ·
[kUserAgent](#constant-kuseragent) ·
[kAccept](#constant-kaccept) ·
[kAcceptEncoding](#constant-kacceptencoding) ·
[kAcceptLanguage](#constant-kacceptlanguage) ·
[kAuthorization](#constant-kauthorization) ·
[kExpect](#constant-kexpect) ·
[kFrom](#constant-kfrom) ·
[kIfMatch](#constant-kifmatch) ·
[kIfNoneMatch](#constant-kifnonematch) ·
[kIfModifiedSince](#constant-kifmodifiedsince) ·
[kIfUnmodifiedSince](#constant-kifunmodifiedsince) ·
[kRange](#constant-krange) ·
[kReferer](#constant-kreferer) ·
[kTe](#constant-kte) ·
[kUpgrade](#constant-kupgrade) ·
[kCookie](#constant-kcookie) ·
[kLocation](#constant-klocation) ·
[kServer](#constant-kserver) ·
[kVary](#constant-kvary) ·
[kWwwAuthenticate](#constant-kwwwauthenticate) ·
[kContentLength](#constant-kcontentlength) ·
[kContentType](#constant-kcontenttype) ·
[kContentEncoding](#constant-kcontentencoding) ·
[kContentLanguage](#constant-kcontentlanguage) ·
[kContentRange](#constant-kcontentrange) ·
[kTrailer](#constant-ktrailer) ·
[kTransferEncoding](#constant-ktransferencoding) ·
[kAllow](#constant-kallow) ·
[kRetryAfter](#constant-kretryafter) ·
[kAcceptRanges](#constant-kacceptranges) ·
[kETag](#constant-ketag) ·
[kLastModified](#constant-klastmodified)

<a id="api"></a>
<a href="../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-header-names"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> header_names

**TYPE** · *protocol::http*

**Declaration synopsis:**

```cpp
struct header_names { ... };
```

Provides the http 1.1 header names specification.

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kconnection"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kConnection

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kConnection[] = "Connection";
```

Constants for the HTTP field names listed below. Each value is the field name
without a colon or field value.

**Value:** `"Connection"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kdate"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kDate

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kDate[] = "Date";
```

Defines the date constant.

**Value:** `"Date"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kvia"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kVia

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kVia[] = "Via";
```

Defines the via constant.

**Value:** `"Via"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kcachecontrol"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kCacheControl

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kCacheControl[] = "Cache-Control";
```

Defines the cache control constant.

**Value:** `"Cache-Control"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kpragma"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kPragma

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kPragma[] = "Pragma";
```

Defines the pragma constant.

**Value:** `"Pragma"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kwarning"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kWarning

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kWarning[] = "Warning";
```

Defines the warning constant.

**Value:** `"Warning"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-khost"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kHost

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kHost[] = "Host";
```

Defines the host constant.

**Value:** `"Host"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kuseragent"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kUserAgent

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kUserAgent[] = "User-agent";
```

Defines the user agent constant.

**Value:** `"User-agent"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kaccept"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kAccept

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kAccept[] = "Accept";
```

Defines the accept constant.

**Value:** `"Accept"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kacceptencoding"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kAcceptEncoding

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kAcceptEncoding[] = "Accept-Encoding";
```

Defines the accept encoding constant.

**Value:** `"Accept-Encoding"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kacceptlanguage"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kAcceptLanguage

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kAcceptLanguage[] = "Accept-Language";
```

Defines the accept language constant.

**Value:** `"Accept-Language"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kauthorization"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kAuthorization

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kAuthorization[] = "Authorization";
```

Defines the authorization constant.

**Value:** `"Authorization"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kexpect"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kExpect

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kExpect[] = "Expect";
```

Defines the expect constant.

**Value:** `"Expect"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kfrom"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kFrom

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kFrom[] = "From";
```

Defines the from constant.

**Value:** `"From"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kifmatch"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kIfMatch

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kIfMatch[] = "If-Match";
```

Defines the if match constant.

**Value:** `"If-Match"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kifnonematch"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kIfNoneMatch

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kIfNoneMatch[] = "If-None-Match";
```

Defines the if none match constant.

**Value:** `"If-None-Match"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kifmodifiedsince"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kIfModifiedSince

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kIfModifiedSince[] = "If-Modified-Since";
```

Defines the if modified since constant.

**Value:** `"If-Modified-Since"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kifunmodifiedsince"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kIfUnmodifiedSince

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kIfUnmodifiedSince[] = "If-Unmodified-Since";
```

Defines the if unmodified since constant.

**Value:** `"If-Unmodified-Since"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-krange"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kRange

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kRange[] = "Range";
```

Defines the range constant.

**Value:** `"Range"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kreferer"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kReferer

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kReferer[] = "Referer";
```

Defines the referer constant.

**Value:** `"Referer"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kte"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kTe

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kTe[] = "TE";
```

Defines the te constant.

**Value:** `"TE"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kupgrade"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kUpgrade

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kUpgrade[] = "Upgrade";
```

Defines the upgrade constant.

**Value:** `"Upgrade"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kcookie"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kCookie

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kCookie[] = "Cookie";
```

Defines the cookie constant.

**Value:** `"Cookie"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-klocation"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kLocation

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kLocation[] = "Location";
```

Defines the location constant.

**Value:** `"Location"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kserver"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kServer

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kServer[] = "Server";
```

Defines the server constant.

**Value:** `"Server"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kvary"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kVary

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kVary[] = "Vary";
```

Defines the vary constant.

**Value:** `"Vary"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kwwwauthenticate"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kWwwAuthenticate

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kWwwAuthenticate[] = "WWW-Authenticate";
```

Defines the www authenticate constant.

**Value:** `"WWW-Authenticate"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kcontentlength"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kContentLength

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kContentLength[] = "Content-Length";
```

Defines the content length constant.

**Value:** `"Content-Length"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kcontenttype"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kContentType

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kContentType[] = "Content-Type";
```

Defines the content type constant.

**Value:** `"Content-Type"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kcontentencoding"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kContentEncoding

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kContentEncoding[] = "Content-Encoding";
```

Defines the content encoding constant.

**Value:** `"Content-Encoding"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kcontentlanguage"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kContentLanguage

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kContentLanguage[] = "Content-Language";
```

Defines the content language constant.

**Value:** `"Content-Language"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kcontentrange"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kContentRange

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kContentRange[] = "Content-Range";
```

Defines the content range constant.

**Value:** `"Content-Range"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-ktrailer"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kTrailer

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kTrailer[] = "Trailer";
```

Defines the trailer constant.

**Value:** `"Trailer"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-ktransferencoding"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kTransferEncoding

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kTransferEncoding[] = "Transfer-Encoding";
```

Defines the transfer encoding constant.

**Value:** `"Transfer-Encoding"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kallow"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kAllow

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kAllow[] = "Allow";
```

Defines the allow constant.

**Value:** `"Allow"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kretryafter"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kRetryAfter

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kRetryAfter[] = "Retry-After";
```

Defines the retry after constant.

**Value:** `"Retry-After"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-kacceptranges"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kAcceptRanges

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kAcceptRanges[] = "Accept-Ranges";
```

Defines the accept ranges constant.

**Value:** `"Accept-Ranges"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-ketag"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kETag

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kETag[] = "ETAG";
```

Defines the etag constant.

**Value:** `"ETAG"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)

<a id="constant-klastmodified"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kLastModified

**CONSTANT** · *protocol::http::header_names*

```cpp
static constexpr char kLastModified[] = "Last-Modified";
```

Defines the last modified constant.

**Value:** `"Last-Modified"`

**Source:** [include/protocol/http/common/header_names.h](../../../../../include/protocol/http/common/header_names.h)
