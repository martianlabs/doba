<a id="date"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/date/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/date/title.svg" alt="date">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/date/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/date/title-dark.svg" alt="date">
</a>

**Header:** [include/protocol/http/common/headers/date.h](../../../../../../include/protocol/http/common/headers/date.h)

Checks the spelling and layout of IMF-fixdate, obsolete RFC 850, or asctime dates. Day/month names and `GMT` are case-sensitive. Numeric fields are checked as digits only: calendar validity, time ranges, and agreement with the weekday are not checked.

<a id="api"></a>
<a id="type-date"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> date

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

```cpp
static constexpr bool check(std::string_view v);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.
