<a id="etag"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/etag/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/etag/index.svg" alt="etag">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/etag/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/etag/index-dark.svg" alt="etag">
</a>

**Header:** [include/protocol/http/common/headers/etag.h](../../../../../../include/protocol/http/common/headers/etag.h)

|                                                                      etag | | RFC 9110 S8.8.3 ETag                                                      | | The "ETag" response header field provides the current entity tag for the  | | selected representation, as determined at the conclusion of handling the  | | request.

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[etag](#type-etag)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::etag

[check](#method-check)

<a id="api-details"></a>
<a href="../../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-etag"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> etag

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class etag { ... };
```

|                                                                      etag | | RFC 9110 S8.8.3 ETag                                                      | | The "ETag" response header field provides the current entity tag for the  | | selected representation, as determined at the conclusion of handling the  | | request.

**Source:** [include/protocol/http/common/headers/etag.h:100](../../../../../../include/protocol/http/common/headers/etag.h#L100)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::etag*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP etag header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/etag.h:105](../../../../../../include/protocol/http/common/headers/etag.h#L105)
