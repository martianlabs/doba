<a id="tcp-windows-overlapped-send-h"></a>
<h1>
  <a href="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_send/index.svg#gh-light-mode-only">
    <img src="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_send/index.svg" alt="Index for tcp_windows_overlapped_send.h">
  </a>
  <a href="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_send/index-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_send/index-dark.svg" alt="Index for tcp_windows_overlapped_send.h">
  </a>
</h1>

**Header:** [include/transport/server/tcp_windows_overlapped_send.h](../../../../include/transport/server/tcp_windows_overlapped_send.h)

Provides the context transport component.

**transport::server**

- [context](#type-context) · *type*
- [overlapped_send](#type-overlapped-send) · *type*

**transport::server::overlapped_send**

- [ctx](#field-ctx) · *field*
- [buffers](#field-buffers) · *field*
- [buffer_count](#field-buffer-count) · *field*
- [submitted_size](#field-submitted-size) · *field*

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

**Source:** [include/transport/server/tcp_windows_overlapped_send.h:42](../../../../include/transport/server/tcp_windows_overlapped_send.h#L42)

<a id="type-overlapped-send"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> overlapped_send

**TYPE** · *transport::server*

**Declaration synopsis:**

```cpp
template <protocol::contracts::engine ENty, typename CNty>
struct overlapped_send : overlapped_base { ... };
```

Windows overlapped send operation.

**Template parameters:**

- **ENty:** `protocol::contracts::engine ENty`.
- **CNty:** `typename CNty`.

**Source:** [include/transport/server/tcp_windows_overlapped_send.h:52](../../../../include/transport/server/tcp_windows_overlapped_send.h#L52)

<a id="field-ctx"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> ctx

**FIELD** · *transport::server::overlapped_send*

```cpp
std::shared_ptr<context<ENty, CNty>> ctx;
```

Stores the ctx value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/transport/server/tcp_windows_overlapped_send.h:62](../../../../include/transport/server/tcp_windows_overlapped_send.h#L62)

<a id="field-buffers"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> buffers

**FIELD** · *transport::server::overlapped_send*

```cpp
std::array<WSABUF, 1> buffers{};
```

Stores the buffers value.

**Value:** `{}`

**Source:** [include/transport/server/tcp_windows_overlapped_send.h:63](../../../../include/transport/server/tcp_windows_overlapped_send.h#L63)

<a id="field-buffer-count"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> buffer_count

**FIELD** · *transport::server::overlapped_send*

```cpp
DWORD buffer_count{0};
```

Stores the buffer count value.

**Value:** `{0}`

**Source:** [include/transport/server/tcp_windows_overlapped_send.h:64](../../../../include/transport/server/tcp_windows_overlapped_send.h#L64)

<a id="field-submitted-size"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> submitted_size

**FIELD** · *transport::server::overlapped_send*

```cpp
std::size_t submitted_size{0};
```

Stores the submitted size value.

**Value:** `{0}`

**Source:** [include/transport/server/tcp_windows_overlapped_send.h:65](../../../../include/transport/server/tcp_windows_overlapped_send.h#L65)
