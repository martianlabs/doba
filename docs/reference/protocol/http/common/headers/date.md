<a id="date-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/date/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/date/index.svg" alt="Index for date.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/date.h](../../../../../../include/protocol/http/common/headers/date.h)

The "Date" header field represents the date and time at which the message was originated. Its field value is an HTTP-date, as defined in RFC 9110 Section 5.6.7 ("Date/Time Formats").

**protocol::http::headers**

- [date](#type-date) · *type*

**protocol::http::headers::date**

- [check](#method-check) · *method*

<a id="type-date"></a>

## date

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class date { ... };
```

The "Date" header field represents the date and time at which the message was originated. Its field value is an HTTP-date, as defined in RFC 9110 Section 5.6.7 ("Date/Time Formats").

**Source:** [include/protocol/http/common/headers/date.h:109](../../../../../../include/protocol/http/common/headers/date.h#L109)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::date*

```cpp
static constexpr bool check(std::string_view v);
```

Checks whether a value follows the HTTP date header syntax.

**Parameters:**

- **v:** Text value to inspect. Declaration: `std::string_view v`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/date.h:114](../../../../../../include/protocol/http/common/headers/date.h#L114)
