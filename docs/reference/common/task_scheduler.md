<a id="task-scheduler"></a>
<a href="../../../resources/docs/reference/common/task_scheduler/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/task_scheduler/title.svg" alt="task_scheduler">
</a>
<a href="../../../resources/docs/reference/common/task_scheduler/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/task_scheduler/title-dark.svg" alt="task_scheduler">
</a>

**Header:** [include/common/task_scheduler.h](../../../include/common/task_scheduler.h)

Connects yield and sleep to their task_execution owner.

<a id="type-task-scheduler"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> task_scheduler

```cpp
struct task_scheduler {
  void* owner = nullptr;
  void (*post)(void*) = nullptr;
  void (*schedule)(void*, std::chrono::steady_clock::time_point) = nullptr;
};
```

task_execution installs both hooks before the first resume.
post marks the owner ready for another turn; schedule records a sleep deadline.
The hooks run inside the current resume call and receive the owner's address.
