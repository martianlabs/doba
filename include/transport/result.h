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

#ifndef martianlabs_doba_transport_result_h
#define martianlabs_doba_transport_result_h

namespace martianlabs::doba::transport {
// =============================================================================
// result                                                         ( enum-class )
// -----------------------------------------------------------------------------
// This enum class will hold for all operational states being handled.
// -----------------------------------------------------------------------------
// =============================================================================
enum class result {
  kSucceeded,                        // everything went fine.
  kAlreadyInitialized,               // resource already initialized.
  kCouldNotSetupPlaformResources,    // platform resources could not be created.
  kCouldNotCleanupPlaformResources,  // platform resources could not be deleted.
};
}  // namespace martianlabs::doba::transport

#endif