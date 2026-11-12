<a id="upgrade"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/upgrade/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/upgrade/title.svg" alt="upgrade">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/upgrade/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/upgrade/title-dark.svg" alt="upgrade">
</a>

**Header:** [include/protocol/http/v11/headers/upgrade.h](../../../../../../include/protocol/http/v11/headers/upgrade.h)

HTTP/1.1 `upgrade` parsing and interpretation.

<a id="api"></a>
<a id="type-upgrade"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> upgrade

Static operations in `martianlabs::doba::protocol::http::v11::headers`.

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

```cpp
static bool check(std::string_view sv, parsed_token_list& out);
```

Appends protocol tokens with optional `/version` tokens.

Output lists are appended to, not cleared. Parsed views borrow the input; keep its backing bytes alive and unchanged. Failure may leave earlier elements appended, and allocation failures may throw. Use a fresh output for a new field. Empty comma elements are skipped; field-level whitespace trimming is the caller's responsibility.

<a id="method-interpret"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> interpret

```cpp
static constexpr verdict interpret(const parsed_token_list& token_list,
                                     v11::connection& conn,
                                     const policies& pol);
```

Appends offers to `upgrade_offer`, then rejects a nonempty offer when `allow_upgrade` is false. Acceptance does not implement a protocol switch.
