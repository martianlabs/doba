//                              _       _
//                           __| | ___ | |__   __ _
//                          / _` |/ _ \| '_ \ / _` |
//                         | (_| | (_) | |_) | (_| |
//                          \__,_|\___/|_.__/ \__,_|
//
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/LICENSE-2.0
//
// Copyright 2026 martianLabs
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied. See the License for the specific language governing
// permissions and limitations under the License.

#ifndef martianlabs_doba_common_cache_h
#define martianlabs_doba_common_cache_h

#include <cstddef>
#include <functional>
#include <limits>
#include <list>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <utility>

#include "common/cache_entry.h"
#include "common/cache_policies.h"

namespace martianlabs::doba::common {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] cache                                                       ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename VAty>
class cache {
 public:
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using value_type = std::shared_ptr<const VAty>;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  explicit cache(cache_policies policies = {}) : policies_(policies) {}
  cache(const cache&) = delete;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  cache& operator=(const cache&) = delete;
  // +=========================================================================+
  // | [>] find                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  value_type find(const std::string& key) {
    value_type value;
    {
      std::shared_lock lock(mutex_);
      auto found = entries_.find(key);
      if (found == entries_.end()) return {};
      value = found->second.value;
    }
    // Don't wait just to update the LRU order.
    std::unique_lock lock(mutex_, std::try_to_lock);
    if (lock.owns_lock()) {
      auto found = entries_.find(key);
      if (found != entries_.end() && found->second.value == value) {
        lru_.splice(lru_.begin(), lru_, found->second.recency);
      }
    }
    return value;
  }
  // +=========================================================================+
  // | [>] try_get_or_load                                          ( public ) |
  // +-------------------------------------------------------------------------+
  // | Load here, without holding the cache lock.                              |
  // | If this key is already loading or a limit is hit, return null.          |
  // | Loader errors go back to this caller.                                   |
  // +-------------------------------------------------------------------------+
  template <typename LDty>
  value_type try_get_or_load(const std::string& key, std::size_t cost,
                             LDty&& loader) {
    std::size_t id;
    {
      std::lock_guard lock(mutex_);
      if (auto found = entries_.find(key); found != entries_.end()) {
        lru_.splice(lru_.begin(), lru_, found->second.recency);
        return found->second.value;
      }
      if (!policies_.max_entries || !policies_.max_cost ||
          loads_ >= policies_.max_loads || cost > policies_.max_cost ||
          loading_.contains(key) ||
          next_load_ == std::numeric_limits<std::size_t>::max()) {
        return {};
      }
      id = next_load_++;
      loading_.emplace(key, id);
      ++loads_;
    }
    value_type result;
    try {
      result = std::invoke(std::forward<LDty>(loader));
    } catch (...) {
      std::lock_guard lock(mutex_);
      finish_load_locked(key, id);
      throw;
    }
    std::lock_guard lock(mutex_);
    if (!finish_load_locked(key, id) || !result) return {};
    // Make room. Responses can keep using the values we drop.
    while (entries_.size() >= policies_.max_entries ||
           cost > policies_.max_cost - resident_cost_)
      remove_locked(entries_.find(lru_.back()));
    lru_.push_front(key);
    try {
      entries_.emplace(key, cache_entry<VAty>{result, cost, lru_.begin()});
    } catch (...) {
      lru_.pop_front();
      throw;
    }
    resident_cost_ += cost;
    return result;
  }
  // +=========================================================================+
  // | [>] erase                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  void erase(const std::string& key) {
    std::lock_guard lock(mutex_);
    if (auto i = entries_.find(key); i != entries_.end()) remove_locked(i);
    loading_.erase(key);
  }
  // +=========================================================================+
  // | [>] erase                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // | Remove it only if it's still the value we saw.                          |
  // +-------------------------------------------------------------------------+
  bool erase(const std::string& key, const value_type& expected) {
    std::lock_guard lock(mutex_);
    auto found = entries_.find(key);
    if (found == entries_.end() || found->second.value != expected) {
      return false;
    }
    remove_locked(found);
    return true;
  }
  // +=========================================================================+
  // | [>] clear                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  void clear() {
    std::lock_guard lock(mutex_);
    entries_.clear();
    lru_.clear();
    resident_cost_ = 0;
    loading_.clear();
  }

 private:
  // +=========================================================================+
  // | [>] USINGs                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  using entries_type = std::unordered_map<std::string, cache_entry<VAty>>;
  // +=========================================================================+
  // | [>] finish_load_locked                                      ( private ) |
  // +-------------------------------------------------------------------------+
  bool finish_load_locked(const std::string& key, std::size_t id) {
    loads_--;
    auto current = loading_.find(key);
    if (current == loading_.end() || current->second != id) return false;
    loading_.erase(current);
    return true;
  }
  // +=========================================================================+
  // | [>] remove_locked                                           ( private ) |
  // +-------------------------------------------------------------------------+
  void remove_locked(typename entries_type::iterator i) {
    resident_cost_ -= i->second.cost;
    lru_.erase(i->second.recency);
    entries_.erase(i);
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  const cache_policies policies_;
  std::shared_mutex mutex_;
  std::list<std::string> lru_;
  entries_type entries_;
  std::unordered_map<std::string, std::size_t> loading_;
  std::size_t next_load_ = 0;
  std::size_t resident_cost_ = 0;
  std::size_t loads_ = 0;
};
}  // namespace martianlabs::doba::common

#endif
