#ifndef martianlabs_doba_tests_test_watchdog_h
#define martianlabs_doba_tests_test_watchdog_h

#include <charconv>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <string_view>
#include <system_error>
#include <thread>

namespace martianlabs::doba::tests {
inline bool parse_test_timeout(std::string_view text,
                               std::chrono::milliseconds& timeout) {
  long long value = 0;
  const auto parsed = std::from_chars(text.data(), text.data() + text.size(),
                                     value);
  if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size() ||
      value <= 0 || value > 3600000) return false;
  timeout = std::chrono::milliseconds(value);
  return true;
}

// One watchdog per runner, including the cleanup performed inside each test.
class test_watchdog {
 public:
  explicit test_watchdog(std::chrono::milliseconds timeout)
      : timeout_(timeout), worker_([this] { watch(); }) {}
  test_watchdog(const test_watchdog&) = delete;
  test_watchdog& operator=(const test_watchdog&) = delete;
  ~test_watchdog() {
    {
      std::lock_guard lock(mutex_);
      stopping_ = true;
      changed_.notify_one();
    }
    worker_.join();
  }

  void arm(std::string_view file, int line, std::string_view name) {
    std::lock_guard lock(mutex_);
    file_ = file;
    line_ = line;
    name_ = name;
    deadline_ = std::chrono::steady_clock::now() + timeout_;
    ++generation_;
    active_ = true;
    changed_.notify_one();
  }
  void disarm() {
    std::lock_guard lock(mutex_);
    active_ = false;
    changed_.notify_one();
  }

 private:
  void watch() {
    std::unique_lock lock(mutex_);
    while (!stopping_) {
      changed_.wait(lock, [this] { return active_ || stopping_; });
      if (stopping_) return;
      const auto generation = generation_;
      const auto deadline = deadline_;
      if (changed_.wait_until(lock, deadline, [this, generation] {
            return stopping_ || !active_ || generation_ != generation;
          })) continue;
      std::fprintf(stderr, "Test timeout after %lld ms: %.*s:%d - %.*s\n",
                   static_cast<long long>(timeout_.count()),
                   static_cast<int>(file_.size()), file_.data(), line_,
                   static_cast<int>(name_.size()), name_.data());
      std::fflush(stderr);
      // A hung test cannot safely unwind. Avoid abort dialogs and core dumps.
      std::_Exit(124);
    }
  }

  std::chrono::milliseconds timeout_;
  std::mutex mutex_;
  std::condition_variable changed_;
  bool stopping_{false};
  bool active_{false};
  unsigned long long generation_{0};
  std::chrono::steady_clock::time_point deadline_{};
  std::string_view file_;
  int line_{0};
  std::string_view name_;
  std::thread worker_;
};
}  // namespace martianlabs::doba::tests
#endif
