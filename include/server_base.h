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

#ifndef martianlabs_doba_server_base_h
#define martianlabs_doba_server_base_h

namespace martianlabs::doba {
// =============================================================================
// server                                                              ( class )
// -----------------------------------------------------------------------------
// This class holds for the base server implementation.
// -----------------------------------------------------------------------------
// Template parameters:
//    RQty - request being used.
//    RSty - response being used.
//    DEty - decoder being used.
//    TRty - transport being used.
// =============================================================================
template <typename RQty, typename RSty,
          template <typename, typename> class DEty,
          template <typename, typename,
                    template <typename, typename> typename> typename TRty>
class server_base : public TRty<RQty, RSty, DEty> {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  server_base() = default;
  server_base(const server_base&) = delete;
  server_base(server_base&&) noexcept = delete;
  ~server_base() = default;
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  server_base& operator=(const server_base&) = delete;
  server_base& operator=(server_base&&) noexcept = delete;
};
}  // namespace martianlabs::doba

#endif
