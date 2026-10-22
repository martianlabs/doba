<a id="filesystem-monitor-linux"></a>
<a href="../../../resources/docs/reference/common/filesystem_monitor_linux/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/filesystem_monitor_linux/title.svg" alt="filesystem_monitor_linux">
</a>
<a href="../../../resources/docs/reference/common/filesystem_monitor_linux/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/filesystem_monitor_linux/title-dark.svg" alt="filesystem_monitor_linux">
</a>

**Header:** [include/common/filesystem_monitor_linux.h](../../../include/common/filesystem_monitor_linux.h)

Implementation support for doba. These declarations are internal and are not part of the supported public API.

[Internal implementation](#implementation)

<a id="api"></a>
<a href="../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="implementation"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> Internal implementation

**INTERNAL HEADER**

Use [filesystem_monitor](filesystem_monitor.md). This backend owns native subscriptions and completion storage; direct dependencies on its declarations are unsupported.
