//      _       _           
//   __| | ___ | |__   __ _ 
//  / _` |/ _ \| '_ \ / _` |
// | (_| | (_) | |_) | (_| |
//  \__,_|\___/|_.__/ \__,_|
// 
// Copyright 2025 martianLabs
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef martianlabs_doba_builder_builder_h
#define martianlabs_doba_builder_builder_h

#include "builder/property.h"

namespace martianlabs::doba::builder {
// =============================================================================
// builder                                                             ( class )
// -----------------------------------------------------------------------------
// This specification holds for the builder base class implementation.
// -----------------------------------------------------------------------------
// Template parameters:
//    DEty - derived builder class being used.
//    INty - instance type being provided.
// =============================================================================
template <typename DEty, typename INty>
class base_builder {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  base_builder() {
    static_assert(
        requires(DEty d) {
          { d.build() } -> std::same_as<std::shared_ptr<INty>>;
        }, "((error)) - builder class must implement build() method!");
  }
  base_builder(const base_builder&) = delete;
  base_builder(base_builder&&) noexcept = delete;
  ~base_builder() = default;
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  base_builder& operator=(const base_builder&) = delete;
  base_builder& operator=(base_builder&&) noexcept = delete;
};
}  // namespace martianlabs::doba::builder

#endif