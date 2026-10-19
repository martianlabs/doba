<a id="policies"></a>
<a href="../../../../../resources/docs/reference/protocol/http/v11/policies/index.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/v11/policies/index.svg" alt="policies">
</a>
<a href="../../../../../resources/docs/reference/protocol/http/v11/policies/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/v11/policies/index-dark.svg" alt="policies">
</a>

**Header:** [include/protocol/http/v11/policies.h](../../../../../include/protocol/http/v11/policies.h)

HTTP/1.1 resource limits, memory thresholds and feature switches. The defaults below preserve the standard server behavior. A limit of 0 means unlimited unless noted.

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11

[policies](#type-policies)

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::policies

[kDefaultMaxQueryParameters](#constant-kdefaultmaxqueryparameters) ·
[kDefaultMaxChunkExtensionSize](#constant-kdefaultmaxchunkextensionsize) ·
[kDefaultMaxTrailerSectionSize](#constant-kdefaultmaxtrailersectionsize) ·
[kDefaultMaxRequestHeadSize](#constant-kdefaultmaxrequestheadsize) ·
[kDefaultRequestBodyMemoryThreshold](#constant-kdefaultrequestbodymemorythreshold) ·
[kDefaultMaxResponseHeadSize](#constant-kdefaultmaxresponseheadsize) ·
[kDefaultResponseBodyMemoryThreshold](#constant-kdefaultresponsebodymemorythreshold) ·
[max_request_body_size](#field-max-request-body-size) ·
[max_forwarding_hops](#field-max-forwarding-hops) ·
[max_transfer_codings](#field-max-transfer-codings) ·
[allow_chunked](#field-allow-chunked) ·
[allow_upgrade](#field-allow-upgrade) ·
[max_pending_requests](#field-max-pending-requests) ·
[max_query_parameters](#field-max-query-parameters) ·
[max_chunk_extension_size](#field-max-chunk-extension-size) ·
[max_trailer_section_size](#field-max-trailer-section-size) ·
[max_request_head_size](#field-max-request-head-size) ·
[request_body_memory_threshold](#field-request-body-memory-threshold) ·
[max_response_head_size](#field-max-response-head-size) ·
[response_body_memory_threshold](#field-response-body-memory-threshold)

<a id="api-details"></a>
<a href="../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-policies"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> policies

**TYPE** · *protocol::http::v11*

**Declaration synopsis:**

```cpp
struct policies { ... };
```

HTTP/1.1 resource limits, memory thresholds and feature switches. The defaults below preserve the standard server behavior. A limit of 0 means unlimited unless noted.

**Source:** [include/protocol/http/v11/policies.h:43](../../../../../include/protocol/http/v11/policies.h#L43)

<a id="constant-kdefaultmaxqueryparameters"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kDefaultMaxQueryParameters

**CONSTANT** · *protocol::http::v11::policies*

```cpp
static constexpr std::size_t kDefaultMaxQueryParameters = 128;
```

Defines the default max query parameters constant.

**Value:** `128`

**Source:** [include/protocol/http/v11/policies.h:48](../../../../../include/protocol/http/v11/policies.h#L48)

<a id="constant-kdefaultmaxchunkextensionsize"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kDefaultMaxChunkExtensionSize

**CONSTANT** · *protocol::http::v11::policies*

```cpp
static constexpr std::size_t kDefaultMaxChunkExtensionSize = 1024;
```

Default maximum chunk extension bytes per chunk.

**Value:** `1024`

**Source:** [include/protocol/http/v11/policies.h:50](../../../../../include/protocol/http/v11/policies.h#L50)

<a id="constant-kdefaultmaxtrailersectionsize"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kDefaultMaxTrailerSectionSize

**CONSTANT** · *protocol::http::v11::policies*

```cpp
static constexpr std::size_t kDefaultMaxTrailerSectionSize = 4096;
```

Default maximum trailer section bytes per chunked body.

**Value:** `4096`

**Source:** [include/protocol/http/v11/policies.h:52](../../../../../include/protocol/http/v11/policies.h#L52)

<a id="constant-kdefaultmaxrequestheadsize"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kDefaultMaxRequestHeadSize

**CONSTANT** · *protocol::http::v11::policies*

```cpp
static constexpr std::size_t kDefaultMaxRequestHeadSize = 4 * 1024;
```

Maximum accepted request head bytes.

**Value:** `4 * 1024`

**Source:** [include/protocol/http/v11/policies.h:54](../../../../../include/protocol/http/v11/policies.h#L54)

<a id="constant-kdefaultrequestbodymemorythreshold"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kDefaultRequestBodyMemoryThreshold

**CONSTANT** · *protocol::http::v11::policies*

```cpp
static constexpr std::size_t kDefaultRequestBodyMemoryThreshold = 16 * 1024;
```

Request body wire bytes retained before spilling to disk.

**Value:** `16 * 1024`

**Source:** [include/protocol/http/v11/policies.h:56](../../../../../include/protocol/http/v11/policies.h#L56)

<a id="constant-kdefaultmaxresponseheadsize"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kDefaultMaxResponseHeadSize

**CONSTANT** · *protocol::http::v11::policies*

```cpp
static constexpr std::size_t kDefaultMaxResponseHeadSize = 4 * 1024;
```

Maximum serialized response head bytes.

**Value:** `4 * 1024`

**Source:** [include/protocol/http/v11/policies.h:58](../../../../../include/protocol/http/v11/policies.h#L58)

<a id="constant-kdefaultresponsebodymemorythreshold"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kDefaultResponseBodyMemoryThreshold

**CONSTANT** · *protocol::http::v11::policies*

```cpp
static constexpr std::size_t kDefaultResponseBodyMemoryThreshold = 16 * 1024;
```

Response body bytes kept inline before using a writer.

**Value:** `16 * 1024`

**Source:** [include/protocol/http/v11/policies.h:60](../../../../../include/protocol/http/v11/policies.h#L60)

<a id="field-max-request-body-size"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> max_request_body_size

**FIELD** · *protocol::http::v11::policies*

```cpp
std::size_t max_request_body_size = 16 * 1024 * 1024;
```

Maximum accepted body size, in octets (0 means unlimited).

**Value:** `16 * 1024 * 1024`

**Source:** [include/protocol/http/v11/policies.h:62](../../../../../include/protocol/http/v11/policies.h#L62)

<a id="field-max-forwarding-hops"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> max_forwarding_hops

**FIELD** · *protocol::http::v11::policies*

```cpp
std::size_t max_forwarding_hops = 32;
```

Maximum number of forwarding hops accepted across Via / Forwarded / X-Forwarded-For (0 means unlimited).

**Value:** `32`

**Source:** [include/protocol/http/v11/policies.h:65](../../../../../include/protocol/http/v11/policies.h#L65)

<a id="field-max-transfer-codings"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> max_transfer_codings

**FIELD** · *protocol::http::v11::policies*

```cpp
std::size_t max_transfer_codings = 4;
```

Maximum number of transfer-codings accepted in Transfer-Encoding (0 means unlimited).

**Value:** `4`

**Source:** [include/protocol/http/v11/policies.h:68](../../../../../include/protocol/http/v11/policies.h#L68)

<a id="field-allow-chunked"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> allow_chunked

**FIELD** · *protocol::http::v11::policies*

```cpp
bool allow_chunked = true;
```

Whether the server allows requests carrying a chunked Transfer-Encoding.

**Value:** `true`

**Source:** [include/protocol/http/v11/policies.h:70](../../../../../include/protocol/http/v11/policies.h#L70)

<a id="field-allow-upgrade"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> allow_upgrade

**FIELD** · *protocol::http::v11::policies*

```cpp
bool allow_upgrade = true;
```

Whether the server allows protocol upgrades offered via Upgrade. NOTE: no protocol upgrade is ever completed. This flag only decides whether an offer is rejected or ignored: true accepts the request and serves it over HTTP/1.1, discarding the offer (RFC 9110 S7.8 allows a server to ignore Upgrade); false rejects the request outright. Honouring an u

**Value:** `true`

**Source:** [include/protocol/http/v11/policies.h:78](../../../../../include/protocol/http/v11/policies.h#L78)

<a id="field-max-pending-requests"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> max_pending_requests

**FIELD** · *protocol::http::v11::policies*

```cpp
std::size_t max_pending_requests = 32;
```

Maximum requests awaiting ordered response delivery (0 means unlimited).

**Value:** `32`

**Source:** [include/protocol/http/v11/policies.h:80](../../../../../include/protocol/http/v11/policies.h#L80)

<a id="field-max-query-parameters"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> max_query_parameters

**FIELD** · *protocol::http::v11::policies*

```cpp
std::size_t max_query_parameters = kDefaultMaxQueryParameters;
```

Maximum query parameter pairs (0 means unlimited).

**Value:** `kDefaultMaxQueryParameters`

**Source:** [include/protocol/http/v11/policies.h:82](../../../../../include/protocol/http/v11/policies.h#L82)

<a id="field-max-chunk-extension-size"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> max_chunk_extension_size

**FIELD** · *protocol::http::v11::policies*

```cpp
std::size_t max_chunk_extension_size = kDefaultMaxChunkExtensionSize;
```

Maximum chunk extension bytes (0 means unlimited).

**Value:** `kDefaultMaxChunkExtensionSize`

**Source:** [include/protocol/http/v11/policies.h:84](../../../../../include/protocol/http/v11/policies.h#L84)

<a id="field-max-trailer-section-size"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> max_trailer_section_size

**FIELD** · *protocol::http::v11::policies*

```cpp
std::size_t max_trailer_section_size = kDefaultMaxTrailerSectionSize;
```

Maximum trailer section bytes (0 means unlimited).

**Value:** `kDefaultMaxTrailerSectionSize`

**Source:** [include/protocol/http/v11/policies.h:86](../../../../../include/protocol/http/v11/policies.h#L86)

<a id="field-max-request-head-size"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> max_request_head_size

**FIELD** · *protocol::http::v11::policies*

```cpp
std::size_t max_request_head_size = kDefaultMaxRequestHeadSize;
```

Maximum request head bytes (must be positive).

**Value:** `kDefaultMaxRequestHeadSize`

**Source:** [include/protocol/http/v11/policies.h:88](../../../../../include/protocol/http/v11/policies.h#L88)

<a id="field-request-body-memory-threshold"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> request_body_memory_threshold

**FIELD** · *protocol::http::v11::policies*

```cpp
std::size_t request_body_memory_threshold =
      kDefaultRequestBodyMemoryThreshold;
```

Request body wire bytes kept in memory (0 spills immediately).

**Value:** `kDefaultRequestBodyMemoryThreshold`

**Source:** [include/protocol/http/v11/policies.h:90](../../../../../include/protocol/http/v11/policies.h#L90)

<a id="field-max-response-head-size"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> max_response_head_size

**FIELD** · *protocol::http::v11::policies*

```cpp
std::size_t max_response_head_size = kDefaultMaxResponseHeadSize;
```

Maximum response head bytes (must be positive).

**Value:** `kDefaultMaxResponseHeadSize`

**Source:** [include/protocol/http/v11/policies.h:93](../../../../../include/protocol/http/v11/policies.h#L93)

<a id="field-response-body-memory-threshold"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> response_body_memory_threshold

**FIELD** · *protocol::http::v11::policies*

```cpp
std::size_t response_body_memory_threshold =
      kDefaultResponseBodyMemoryThreshold;
```

Response body bytes kept inline (0 disables inline storage).

**Value:** `kDefaultResponseBodyMemoryThreshold`

**Source:** [include/protocol/http/v11/policies.h:95](../../../../../include/protocol/http/v11/policies.h#L95)
