<a id="platform"></a>
<a href="../../resources/docs/reference/platform/title.svg#gh-light-mode-only">
  <img src="../../resources/docs/reference/platform/title.svg" alt="platform">
</a>
<a href="../../resources/docs/reference/platform/title-dark.svg#gh-dark-mode-only">
  <img src="../../resources/docs/reference/platform/title-dark.svg" alt="platform">
</a>

**Header:** [include/platform.h](../../include/platform.h)

Selects or forwards the platform-specific declarations included by this header.

### <a href="../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> Exports

[Exports](#exports)

<a id="api"></a>
<a href="../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="exports"></a>

## <a href="../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> Exports

**HEADER**

This header configures platform includes and defines no C++ declarations of its own.

Windows inclusion defines `_WIN32_DCOM` and `NOMINMAX`, includes Winsock headers,
and contains MSVC pragmas disabling warning 4996 and requesting `Ws2_32.lib`,
`wbemuuid.lib` and `Mswsock.lib`. These are visible effects on the including
translation unit, not just declarations local to doba.

Linux inclusion supplies the socket, epoll, eventfd, file-descriptor and TCP
headers used by the native implementation. Platform selectors support Windows
and Linux; they do not provide another backend or a dedicated unsupported-OS
diagnostic branch.
