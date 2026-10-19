<a id="tcp-windows-overlapped-base"></a>
<a href="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_base/index.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_base/index.svg" alt="tcp_windows_overlapped_base">
</a>
<a href="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_base/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_base/index-dark.svg" alt="tcp_windows_overlapped_base">
</a>

**Header:** [include/transport/server/tcp_windows_overlapped_base.h](../../../../include/transport/server/tcp_windows_overlapped_base.h)

Base state for Windows overlapped I/O.

### <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transport::server

[io_type](#enum-io-type) ·
[overlapped_base](#type-overlapped-base)

### <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transport::server::overlapped_base

[get_type](#method-get-type)

<a id="api-details"></a>
<a href="../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="enum-io-type"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> io_type

**ENUM** · *transport::server*

```cpp
enum class io_type : uint8_t { kAccept, kSend, kReceive, kOutput };
```

Enumerates io type values.

**Values:**

- **kAccept:** Accept. Declaration: `kAccept`.
- **kSend:** Send. Declaration: `kSend`.
- **kReceive:** Receive. Declaration: `kReceive`.
- **kOutput:** Output. Declaration: `kOutput`.

**Source:** [include/transport/server/tcp_windows_overlapped_base.h:38](../../../../include/transport/server/tcp_windows_overlapped_base.h#L38)

<a id="type-overlapped-base"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> overlapped_base

**TYPE** · *transport::server*

**Declaration synopsis:**

```cpp
struct overlapped_base : OVERLAPPED { ... };
```

Base state for Windows overlapped I/O.

**Source:** [include/transport/server/tcp_windows_overlapped_base.h:47](../../../../include/transport/server/tcp_windows_overlapped_base.h#L47)

<a id="method-get-type"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> get_type

**METHOD** · *transport::server::overlapped_base*

```cpp
io_type get_type() const;
```

Returns type.

**Returns:** A value of type `io_type`.

**Source:** [include/transport/server/tcp_windows_overlapped_base.h:55](../../../../include/transport/server/tcp_windows_overlapped_base.h#L55)
