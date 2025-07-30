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

#ifndef martianlabs_doba_protocol_http11_method_h
#define martianlabs_doba_protocol_http11_method_h

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// method                                                         ( enum-class )
// -----------------------------------------------------------------------------
// This enum class holds for the http 1.1 method implementation.
// -----------------------------------------------------------------------------
// =============================================================================
enum class method {
  kUnknown,
  kGet,
  kHead,
  kPost,
  kPut,
  kDelete,
  kConnect,
  kOptions,
  kTrace
};
}  // namespace martianlabs::doba::protocol::http11

#endif
