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


#ifndef martianlabs_doba_protocol_http11_verdict_h
#define martianlabs_doba_protocol_http11_verdict_h

namespace martianlabs::doba::protocol::http11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] verdict                                                ( enum-class ) |
// +---------------------------------------------------------------------------+
// | The outcome of the semantic interpretation layer. Unlike the syntactic    |
// | checkers (which only answer "is this well-formed?"), an interpreter or a  |
// | transversal rule answers "should this message be served?" after taking    |
// | the parsed values, the request as a whole, and the connection/policies    |
// | state into account.                                                       |
// +---------------------------------------------------------------------------+
// | kAccept - the message is semantically valid and may be served.            |
// | kReject - the message must not be served.                                 |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
enum class verdict {
  kAccept,  // the message is semantically valid and may be served.
  kReject,  // the message must not be served.
};
}  // namespace martianlabs::doba::protocol::http11

#endif
