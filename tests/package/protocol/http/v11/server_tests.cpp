//                              _       _
//                           __| | ___ | |__   __ _
//                          / _` |/ _ \| '_ \ / _` |
//                         | (_| | (_) | |_) | (_| |
//                          \__,_|\___/|_.__/ \__,_|
//
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/
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

#include "common/filesystem.h"
#include "common/reader.h"
#include "protocol/http/v11/server.h"
#include "protocol/http/v11/static_file_server.h"

namespace {
using namespace martianlabs::doba::protocol::http::v11;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] package_controller                                          ( class ) |
// +---------------------------------------------------------------------------+
// | Controller implementation.                                                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class package_controller {
 public:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +=========================================================================+
  template <typename Rty>
  void register_routes(Rty& routes) {
    routes.add("GET", "/package/:id", &package_controller::get);
  }
  response get(const request&, int id) const {
    auto result = response::ok_200();
    result.set_body(id);
    return result;
  }
};
}  // namespace

int main() {
  std::error_code error;
  const auto root = martianlabs::doba::common::filesystem_root(".", error);
  if (error) return 1;
  martianlabs::doba::common::filesystem_file file;
  martianlabs::doba::common::reader input(std::move(file));
  server<> value;
  value.add_controller<package_controller>()
      .add_controller<static_file_server>("/files", root);
  return 0;
}
