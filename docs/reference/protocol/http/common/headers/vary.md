<a id="vary"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/vary/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/vary/index.svg" alt="vary">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/vary/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/vary/index-dark.svg" alt="vary">
</a>

**Header:** [include/protocol/http/common/headers/vary.h](../../../../../../include/protocol/http/common/headers/vary.h)

|                                                                      vary | | RFC 9110 S12.5.5 Vary                                                     | | The "Vary" response header field describes which parts of the request,    | | aside from the method and target URI, might have influenced the origin    | | server's process for selecting the r

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[vary](#type-vary)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::vary

[check](#method-check)

<a id="api-details"></a>
<a href="../../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-vary"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> vary

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class vary { ... };
```

|                                                                      vary | | RFC 9110 S12.5.5 Vary                                                     | | The "Vary" response header field describes which parts of the request,    | | aside from the method and target URI, might have influenced the origin    | | server's process for selecting the r

**Source:** [include/protocol/http/common/headers/vary.h:115](../../../../../../include/protocol/http/common/headers/vary.h#L115)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::vary*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP vary header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/vary.h:120](../../../../../../include/protocol/http/common/headers/vary.h#L120)
