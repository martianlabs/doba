<a id="location"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/location/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/location/index.svg" alt="location">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/location/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/location/index-dark.svg" alt="location">
</a>

**Header:** [include/protocol/http/common/headers/location.h](../../../../../../include/protocol/http/common/headers/location.h)

|                                                                  location | | RFC 9110 S10.2.2 Location                                                 | | The "Location" header field is used in some responses to refer to a       | | specific resource in relation to the response. Its field value consists   | | of a single URI-reference.

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[location](#type-location)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::location

[check](#method-check)

<a id="api-details"></a>
<a href="../../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-location"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> location

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class location { ... };
```

|                                                                  location | | RFC 9110 S10.2.2 Location                                                 | | The "Location" header field is used in some responses to refer to a       | | specific resource in relation to the response. Its field value consists   | | of a single URI-reference.

**Source:** [include/protocol/http/common/headers/location.h:125](../../../../../../include/protocol/http/common/headers/location.h#L125)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::location*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP location header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/location.h:130](../../../../../../include/protocol/http/common/headers/location.h#L130)
