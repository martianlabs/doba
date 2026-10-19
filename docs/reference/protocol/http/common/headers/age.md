<a id="age"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/age/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/age/index.svg" alt="age">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/age/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/age/index-dark.svg" alt="age">
</a>

**Header:** [include/protocol/http/common/headers/age.h](../../../../../../include/protocol/http/common/headers/age.h)

|                                                                       age | | RFC 9111 S5.1 Age                                                         | | The "Age" response header field conveys the sender's estimate of the time | | since the response was generated or successfully validated at the origin  | | server. Age values are calculated as

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[age](#type-age)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::age

[check](#method-check)

<a id="api-details"></a>
<a href="../../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-age"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> age

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class age { ... };
```

|                                                                       age | | RFC 9111 S5.1 Age                                                         | | The "Age" response header field conveys the sender's estimate of the time | | since the response was generated or successfully validated at the origin  | | server. Age values are calculated as

**Source:** [include/protocol/http/common/headers/age.h:90](../../../../../../include/protocol/http/common/headers/age.h#L90)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::age*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP age header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/age.h:95](../../../../../../include/protocol/http/common/headers/age.h#L95)
