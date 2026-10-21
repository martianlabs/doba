// Copyright 2026 martianLabs. Licensed under the Apache License, Version 2.0.
#ifndef martianlabs_doba_common_cache_h
#define martianlabs_doba_common_cache_h

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <exception>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include <utility>

namespace martianlabs::doba::common {
// Costs are caller-defined units, not an estimate of allocator overhead.
// Zero capacity disables admission. TTL zero means no automatic expiration.
struct cache_policies {
  std::size_t max_entries = 1024;
  std::size_t max_cost = 64 * 1024 * 1024;
  std::size_t max_loads = 16;
  std::chrono::milliseconds ttl{1000};
};

struct cache_statistics {
  std::size_t entries = 0;
  std::size_t resident_cost = 0;
  std::size_t live_cost = 0;
  std::size_t loads = 0;
  std::size_t waiters = 0;
};

// Thread-safe, cost-bounded LRU with single-flight loading per key.
// Values must be immutable through ALL aliases supplied by the loader.
// The cache must outlive concurrent calls; returned values may outlive it.
// Hash/equality and clock operations must not throw or reenter the cache.
template <typename Key, typename Value, typename Clock = std::chrono::steady_clock,
          typename Hash = std::hash<Key>, typename Equal = std::equal_to<Key>>
class cache {
 public:
  using value_type = std::shared_ptr<const Value>;

  explicit cache(cache_policies policies = {}) : policies_(policies) {
    if (policies.ttl.count() < 0) throw std::invalid_argument("Negative cache TTL");
  }
  cache(const cache&) = delete;
  cache& operator=(const cache&) = delete;

  // Expired entries are removed lazily. A hit updates LRU recency.
  value_type find(const Key& key) {
    std::lock_guard lock(mutex_);
    return find_locked(key);
  }

  // Loader returns shared_ptr<const Value> (null means not found).
  // cost must upper-bound the resources retained by the returned value.
  // Null is also returned on capacity rejection or concurrent invalidation.
  // Exceptions reach all callers waiting on that load. No failures are cached.
  // Do not recursively load this cache from a loader (same-key recursion throws).
  template <typename Loader>
  value_type get_or_load(const Key& key, std::size_t cost, Loader&& loader) {
    std::shared_ptr<flight> job;
    std::shared_ptr<charge> reservation;
    {
      std::unique_lock lock(mutex_);
      if (auto hit = find_locked(key)) return hit;
      if (auto found = flights_.find(key); found != flights_.end()) {
        job = found->second;
        if (job->owner == std::this_thread::get_id())
          throw std::logic_error("Recursive cache load");
        ++waiters_;
        job->changed.wait(lock, [&] { return job->done; });
        --waiters_;
        if (job->error) std::rethrow_exception(job->error);
        return job->value;
      }
      if (!policies_.max_entries || !policies_.max_cost || !policies_.max_loads ||
          loads_ >= policies_.max_loads || cost > policies_.max_cost)
        return {};
      // Expiration and eviction release only the cache's ownership. Pins held
      // by callers stay charged and may prevent admission of a new value.
      while (!lru_.empty() &&
             (entries_.size() >= policies_.max_entries ||
              cost > policies_.max_cost - live_->load())) {
        remove_locked(entries_.find(lru_.back()));
      }
      if (cost > policies_.max_cost - live_->load()) return {};
      reservation = std::make_shared<charge>(live_, cost);
      job = std::make_shared<flight>();
      flights_.emplace(key, job);
      ++loads_;
    }

    value_type result;
    std::exception_ptr error;
    try {
      if (auto value = std::invoke(std::forward<Loader>(loader))) {
        auto owned = std::make_shared<retained>(std::move(value), reservation);
        result = value_type(owned, owned->value.get());
      }
    } catch (...) {
      error = std::current_exception();
    }
    reservation.reset();
    {
      std::lock_guard lock(mutex_);
      --loads_;
      auto current = flights_.find(key);
      const bool publish = current != flights_.end() && current->second == job;
      if (publish) {
        if (result && !error) {
          try {
            while (entries_.size() >= policies_.max_entries)
              remove_locked(entries_.find(lru_.back()));
            lru_.push_front(key);
            try {
              entries_.emplace(key, entry{result, cost, expiry(), lru_.begin()});
            } catch (...) {
              lru_.pop_front();
              throw;
            }
            resident_cost_ += cost;
          } catch (...) {
            error = std::current_exception();
            result.reset();
          }
        }
        flights_.erase(current);
        job->value = result;
        job->error = error;
        job->done = true;
        job->changed.notify_all();
      } else {
        // erase/clear already released waiters. An obsolete load cannot
        // republish, return its old value, or erase a newer flight.
        result.reset();
        error = {};
      }
    }
    if (error) std::rethrow_exception(error);
    return result;
  }

  void erase(const Key& key) {
    std::lock_guard lock(mutex_);
    if (auto i = entries_.find(key); i != entries_.end()) remove_locked(i);
    if (auto i = flights_.find(key); i != flights_.end()) {
      i->second->done = true;
      i->second->changed.notify_all();
      flights_.erase(i);
    }
  }

  void clear() {
    std::lock_guard lock(mutex_);
    entries_.clear();
    lru_.clear();
    resident_cost_ = 0;
    for (auto& [key, job] : flights_) {
      (void)key;
      job->done = true;
      job->changed.notify_all();
    }
    flights_.clear();
  }

  [[nodiscard]] cache_statistics statistics() const {
    std::lock_guard lock(mutex_);
    return {entries_.size(), resident_cost_, live_->load(), loads_, waiters_};
  }

 private:
  struct charge {
    std::shared_ptr<std::atomic<std::size_t>> live;
    std::size_t cost;
    charge(std::shared_ptr<std::atomic<std::size_t>> counter, std::size_t amount)
        : live(std::move(counter)), cost(amount) { live->fetch_add(cost); }
    ~charge() { live->fetch_sub(cost); }
  };
  struct retained {
    std::shared_ptr<charge> reservation;
    value_type value;
    retained(value_type v, std::shared_ptr<charge> r)
        : reservation(std::move(r)), value(std::move(v)) {}
  };
  struct flight {
    std::thread::id owner = std::this_thread::get_id();
    std::condition_variable changed;
    bool done = false;
    value_type value;
    std::exception_ptr error;
  };
  struct entry {
    value_type value;
    std::size_t cost;
    typename Clock::time_point expires;
    typename std::list<Key>::iterator recency;
  };
  using entries_type = std::unordered_map<Key, entry, Hash, Equal>;

  typename Clock::time_point expiry() const {
    const auto now = Clock::now();
    using duration = typename Clock::duration;
    if (!policies_.ttl.count() ||
        std::chrono::duration<long double>(policies_.ttl) >=
            std::chrono::duration<long double>(duration::max()))
      return Clock::time_point::max();
    const auto delta = std::chrono::duration_cast<duration>(policies_.ttl);
    if (now > Clock::time_point::max() - delta) return Clock::time_point::max();
    return now + delta;
  }
  value_type find_locked(const Key& key) {
    auto i = entries_.find(key);
    if (i == entries_.end()) return {};
    if (Clock::now() >= i->second.expires) {
      remove_locked(i);
      return {};
    }
    lru_.splice(lru_.begin(), lru_, i->second.recency);
    return i->second.value;
  }
  void remove_locked(typename entries_type::iterator i) {
    resident_cost_ -= i->second.cost;
    lru_.erase(i->second.recency);
    entries_.erase(i);
  }

  const cache_policies policies_;
  mutable std::mutex mutex_;
  std::list<Key> lru_;
  entries_type entries_;
  std::unordered_map<Key, std::shared_ptr<flight>, Hash, Equal> flights_;
  std::shared_ptr<std::atomic<std::size_t>> live_ =
      std::make_shared<std::atomic<std::size_t>>(0);
  std::size_t resident_cost_ = 0;
  std::size_t loads_ = 0;
  std::size_t waiters_ = 0;
};
}  // namespace martianlabs::doba::common
#endif
