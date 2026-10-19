<a id="date"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/date/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/date/index.svg" alt="date">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/date/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/date/index-dark.svg" alt="date">
</a>

**Header:** [include/protocol/http/common/headers/date.h](../../../../../../include/protocol/http/common/headers/date.h)

The "Date" header field represents the date and time at which the message was originated. Its field value is an HTTP-date, as defined in RFC 9110 Section 5.6.7 ("Date/Time Formats").

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[date](#type-date)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::date

[check](#method-check)

<a id="type-date"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> date

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class date { ... };
```

The "Date" header field represents the date and time at which the message was originated. Its field value is an HTTP-date, as defined in RFC 9110 Section 5.6.7 ("Date/Time Formats").

**Source:** [include/protocol/http/common/headers/date.h:109](../../../../../../include/protocol/http/common/headers/date.h#L109)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::date*

```cpp
static constexpr bool check(std::string_view v);
```

Checks whether a value follows the HTTP date header syntax.

**Parameters:**

- **v:** Text value to inspect. Declaration: `std::string_view v`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/date.h:114](../../../../../../include/protocol/http/common/headers/date.h#L114)
