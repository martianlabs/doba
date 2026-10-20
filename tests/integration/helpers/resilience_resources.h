#pragma once

#include "platform.h"
#ifdef _WIN32
#include <psapi.h>
#endif

#include <atomic>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

namespace martianlabs::doba::tests::integration {
struct resilience_resources {
  std::size_t handles = 0;
  std::size_t memory_bytes = 0;
};

inline resilience_resources measure_resilience_resources() {
  resilience_resources result;
#ifdef _WIN32
  DWORD handles = 0;
  PROCESS_MEMORY_COUNTERS_EX memory{};
  if (!GetProcessHandleCount(GetCurrentProcess(), &handles) ||
      !K32GetProcessMemoryInfo(GetCurrentProcess(),
          reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&memory), sizeof(memory)))
    throw std::runtime_error("Cannot measure process resources");
  result.handles = handles;
  result.memory_bytes = memory.PrivateUsage;
#else
  for ([[maybe_unused]] const auto& entry : std::filesystem::directory_iterator("/proc/self/fd"))
    ++result.handles;
  std::ifstream statm("/proc/self/statm");
  std::size_t virtual_pages = 0, resident_pages = 0;
  if (!(statm >> virtual_pages >> resident_pages))
    throw std::runtime_error("Cannot measure resident memory");
  result.memory_bytes = resident_pages * static_cast<std::size_t>(::sysconf(_SC_PAGESIZE));
#endif
  return result;
}

// Tests execute sequentially in this runner. Change the environment only before
// starting server/client threads and restore it after their destruction.
class resilience_temp_directory {
 public:
  resilience_temp_directory() {
    const auto* old = std::getenv(variable());
    present_ = old != nullptr;
    if (old) previous_ = old;
    static std::atomic<unsigned> sequence{0};
#ifdef _WIN32
    const auto pid = GetCurrentProcessId();
#else
    const auto pid = ::getpid();
#endif
    path = std::filesystem::temp_directory_path() /
        ("doba_resilience_" + std::to_string(pid) + "_" + std::to_string(sequence++));
    if (!std::filesystem::create_directory(path))
      throw std::runtime_error("Cannot create isolated resilience temporary directory");
    set(path.string().c_str());
  }
  ~resilience_temp_directory() {
    set(present_ ? previous_.c_str() : nullptr);
    // A leaked file is evidence: leave a nonempty directory for inspection.
    std::error_code ignored;
    std::filesystem::remove(path, ignored);
  }
  resilience_temp_directory(const resilience_temp_directory&) = delete;
  resilience_temp_directory& operator=(const resilience_temp_directory&) = delete;
  bool empty() const { return std::filesystem::is_empty(path); }
  std::filesystem::path path;

 private:
  static const char* variable() {
#ifdef _WIN32
    return "TMP";
#else
    return "TMPDIR";
#endif
  }
  static void set(const char* value) {
#ifdef _WIN32
    _putenv_s(variable(), value ? value : "");
#else
    if (value) ::setenv(variable(), value, 1);
    else ::unsetenv(variable());
#endif
  }
  bool present_ = false;
  std::string previous_;
};
}  // namespace martianlabs::doba::tests::integration
