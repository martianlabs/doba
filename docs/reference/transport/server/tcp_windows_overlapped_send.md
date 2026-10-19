<a id="tcp-windows-overlapped-send-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_send/index-dark.svg">
    <img src="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_send/index.svg" alt="Index for tcp_windows_overlapped_send.h">
  </picture>
</h1>

**Header:** [include/transport/server/tcp_windows_overlapped_send.h](../../../../include/transport/server/tcp_windows_overlapped_send.h)

Provides the context transport component.

**transport::server**

- [context](#type-context) · *type*
- [overlapped_send](#type-overlapped-send) · *type*

**transport::server::overlapped_send**

- [overlapped_send](#constructor-overlapped-send) · *constructor*
- [ctx](#field-ctx) · *field*
- [buffers](#field-buffers) · *field*
- [buffer_count](#field-buffer-count) · *field*
- [submitted_size](#field-submitted-size) · *field*

<a id="type-context"></a>

## context

🟠 **TYPE** · *transport::server*

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

## overlapped_send

🟠 **TYPE** · *transport::server*

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

<a id="constructor-overlapped-send"></a>

## overlapped_send

🟠 **CONSTRUCTOR** · *transport::server::overlapped_send*

```cpp
overlapped_send(std::shared_ptr<context<ENty, CNty>> context,
                  io_type type = io_type::kSend);
```

Constructs a overlapped_send value.

**Parameters:**

- **context:** Context for this operation. Declaration: `std::shared_ptr<context<ENty, CNty>> context`.
- **type:** Type or classification to store or inspect. Declaration: `io_type type = io_type::kSend`.

**Source:** [include/transport/server/tcp_windows_overlapped_send.h:56](../../../../include/transport/server/tcp_windows_overlapped_send.h#L56)

<a id="field-ctx"></a>

## ctx

🟠 **FIELD** · *transport::server::overlapped_send*

```cpp
std::shared_ptr<context<ENty, CNty>> ctx;
```

Stores the ctx value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/transport/server/tcp_windows_overlapped_send.h:62](../../../../include/transport/server/tcp_windows_overlapped_send.h#L62)

<a id="field-buffers"></a>

## buffers

🟠 **FIELD** · *transport::server::overlapped_send*

```cpp
std::array<WSABUF, 1> buffers{};
```

Stores the buffers value.

**Value:** `{}`

**Source:** [include/transport/server/tcp_windows_overlapped_send.h:63](../../../../include/transport/server/tcp_windows_overlapped_send.h#L63)

<a id="field-buffer-count"></a>

## buffer_count

🟠 **FIELD** · *transport::server::overlapped_send*

```cpp
DWORD buffer_count{0};
```

Stores the buffer count value.

**Value:** `{0}`

**Source:** [include/transport/server/tcp_windows_overlapped_send.h:64](../../../../include/transport/server/tcp_windows_overlapped_send.h#L64)

<a id="field-submitted-size"></a>

## submitted_size

🟠 **FIELD** · *transport::server::overlapped_send*

```cpp
std::size_t submitted_size{0};
```

Stores the submitted size value.

**Value:** `{0}`

**Source:** [include/transport/server/tcp_windows_overlapped_send.h:65](../../../../include/transport/server/tcp_windows_overlapped_send.h#L65)
