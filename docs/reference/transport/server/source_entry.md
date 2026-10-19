<a id="source-entry"></a>
<a href="../../../../resources/docs/reference/transport/server/source_entry/index.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/source_entry/index.svg" alt="source_entry">
</a>
<a href="../../../../resources/docs/reference/transport/server/source_entry/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/source_entry/index-dark.svg" alt="source_entry">
</a>

**Header:** [include/transport/server/send_state.h](../../../../include/transport/server/send_state.h)

**Declared in:** [send_state](send_state.md#type-send-state)

Provides the source entry transport component.

### <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transport::server::send_state

[source_entry](#type-source-entry)

### <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transport::server::send_state::source_entry

[prefix](#field-prefix) ·
[source](#field-source)

<a id="type-source-entry"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> source_entry

**TYPE** · *transport::server::send_state*

**Declaration synopsis:**

```cpp
struct source_entry { ... };
```

Provides the source entry transport component.

**Source:** [include/transport/server/send_state.h:57](../../../../include/transport/server/send_state.h#L57)

<a id="field-prefix"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> prefix

**FIELD** · *transport::server::send_state::source_entry*

```cpp
std::string prefix;
```

Prefix data to be sent before reading from the source (memory based)

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/transport/server/send_state.h:59](../../../../include/transport/server/send_state.h#L59)

<a id="field-source"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> source

**FIELD** · *transport::server::send_state::source_entry*

```cpp
std::unique_ptr<common::reader> source;
```

Source reader for additional data (stream based)

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/transport/server/send_state.h:61](../../../../include/transport/server/send_state.h#L61)
