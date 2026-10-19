<a id="tcp-windows-overlapped-receive"></a>
<a href="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_receive/index.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_receive/index.svg" alt="tcp_windows_overlapped_receive">
</a>
<a href="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_receive/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_receive/index-dark.svg" alt="tcp_windows_overlapped_receive">
</a>

**Header:** [include/transport/server/tcp_windows_overlapped_receive.h](../../../../include/transport/server/tcp_windows_overlapped_receive.h)

Provides the context transport component.

### <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transport::server

[context](#type-context) ·
[overlapped_receive](#type-overlapped-receive)

### <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transport::server::overlapped_receive

[ctx](#field-ctx)

<a id="type-context"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> context

**TYPE** · *transport::server*

**Declaration synopsis:**

```cpp
template <protocol::contracts::engine ENty, typename CNty>
struct context { ... };
```

Provides the context transport component.

**Template parameters:**

- **ENty:** `protocol::contracts::engine ENty`.
- **CNty:** `typename CNty`.

**Source:** [include/transport/server/tcp_windows_overlapped_receive.h:40](../../../../include/transport/server/tcp_windows_overlapped_receive.h#L40)

<a id="type-overlapped-receive"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> overlapped_receive

**TYPE** · *transport::server*

**Declaration synopsis:**

```cpp
template <protocol::contracts::engine ENty, typename CNty>
struct overlapped_receive : overlapped_base { ... };
```

Windows overlapped receive operation.

**Template parameters:**

- **ENty:** `protocol::contracts::engine ENty`.
- **CNty:** `typename CNty`.

**Source:** [include/transport/server/tcp_windows_overlapped_receive.h:50](../../../../include/transport/server/tcp_windows_overlapped_receive.h#L50)

<a id="field-ctx"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> ctx

**FIELD** · *transport::server::overlapped_receive*

```cpp
std::shared_ptr<context<ENty, CNty>> ctx;
```

Stores the ctx value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/transport/server/tcp_windows_overlapped_receive.h:59](../../../../include/transport/server/tcp_windows_overlapped_receive.h#L59)
