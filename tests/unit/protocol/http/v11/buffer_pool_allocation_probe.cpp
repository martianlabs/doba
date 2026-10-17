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
// Copyright 2025 martianLabs
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

#include <cstddef>
#include <initializer_list>
#include <new>
#include <utility>

#include "protocol/http/v11/pool.h"
#include "protocol/http/v11/policies.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::detail::buffer_pool;
using martianlabs::doba::protocol::http::v11::policies;

thread_local int fail_array_after = -1;

void check_array_allocation() {
  if (fail_array_after == 0) {
    fail_array_after = -1;
    throw std::bad_alloc();
  }
  if (fail_array_after > 0) fail_array_after--;
}
}  // namespace

#ifdef __linux__
// Keep sanitizer-owned allocation operators intact.
extern "C" void* __real__Znam(std::size_t size);
extern "C" void* __wrap__Znam(std::size_t size) {
  check_array_allocation();
  return __real__Znam(size);
}
#else
// Replacements are isolated from the main unit-test executable.
void* operator new[](std::size_t size) {
  check_array_allocation();
  return ::operator new(size);
}

void* operator new[](std::size_t size, const std::nothrow_t&) noexcept {
  try {
    return ::operator new[](size);
  } catch (...) {
    return nullptr;
  }
}

void operator delete[](void* address) noexcept {
  ::operator delete(address);
}

void operator delete[](void* address, std::size_t) noexcept {
  ::operator delete(address);
}

void operator delete[](void* address, const std::nothrow_t&) noexcept {
  ::operator delete(address);
}
#endif

// +===========================================================================+
// | [>] preserves available pairs after allocation failure      ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("buffer pool preserves available pairs after allocation failure") {
  const policies configuration;
  for (const bool request : {false, true}) {
    auto first = buffer_pool::acquire(configuration);
    const auto* pair = first.get();
    buffer_pool::release(std::move(first));
    auto other = configuration;
    other.request_head_buffer_capacity++;
    other.response_head_buffer_capacity++;
    fail_array_after = request ? 0 : 1;
    bool failed = false;
    try {
      auto value = buffer_pool::acquire(other);
    } catch (const std::bad_alloc&) {
      failed = true;
    }
    fail_array_after = -1;
    DOBA_EXPECT(failed);
    auto second = buffer_pool::acquire(configuration);
    DOBA_EXPECT(second.get() == pair);
    buffer_pool::release(std::move(second));
  }
}
