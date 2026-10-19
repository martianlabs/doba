<a id="from-h"></a>
<h1>
  <a href="../../../../../../resources/docs/reference/protocol/http/common/headers/from/index.svg#gh-light-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/from/index.svg" alt="Index for from.h">
  </a>
  <a href="../../../../../../resources/docs/reference/protocol/http/common/headers/from/index-dark.svg#gh-dark-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/from/index-dark.svg" alt="Index for from.h">
  </a>
</h1>

**Header:** [include/protocol/http/common/headers/from.h](../../../../../../include/protocol/http/common/headers/from.h)

|                                                                      from | | RFC 9110 S10.1.2 From                                                     | | The "From" header field contains an Internet email address for a human    | | user who controls the requesting user agent. The address ought to be      | | machine-usable, as defined by "mailb

**protocol::http::headers**

- [from](#type-from) · *type*

**protocol::http::headers::from**

- [check](#method-check) · *method*

<a id="type-from"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> from

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class from { ... };
```

|                                                                      from | | RFC 9110 S10.1.2 From                                                     | | The "From" header field contains an Internet email address for a human    | | user who controls the requesting user agent. The address ought to be      | | machine-usable, as defined by "mailb

**Source:** [include/protocol/http/common/headers/from.h:108](../../../../../../include/protocol/http/common/headers/from.h#L108)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::from*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP from header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/from.h:113](../../../../../../include/protocol/http/common/headers/from.h#L113)
