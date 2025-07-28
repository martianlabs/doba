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

#ifndef martianlabs_doba_protocol_http11_router_h
#define martianlabs_doba_protocol_http11_router_h

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// router                                                              ( class )
// -----------------------------------------------------------------------------
// This class holds for the http 1.1 router implementation.
// -----------------------------------------------------------------------------
// Template parameters:
//    RQty - request being used.
//    RSty - response being used.
// =============================================================================
template <typename RQty, typename RSty>
class router {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  router() = default;
  router(const router&) = delete;
  router(router&&) noexcept = delete;
  ~router() = default;
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  router& operator=(const router&) = delete;
  router& operator=(router&&) noexcept = delete;
  // ___________________________________________________________________________
  // USINGs                                                          ( private )
  //
  using handler_fn = std::function<void(const RQty&, RSty&)>;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  void add_get(const std::string_view& route, handler_fn) {}

 private:
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  // ___________________________________________________________________________
  // CONSTANTs                                                       ( private )
  //
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
};
}  // namespace martianlabs::doba::protocol::http11

#endif
