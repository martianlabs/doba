﻿//      _       _           
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

#ifndef martianlabs_doba_platform_h
#define martianlabs_doba_platform_h

// ___________________________________________________________________________
// INCLUDES                                                         ( common )
//
#include <limits>
#include <mutex>
#include <optional>
#include <queue>
#include <system_error>
#include <thread>
#include <vector>
#include <memory>
#include <cstring>
#include <sstream>
#include <string>
#include <variant>
#include <inttypes.h>
// ___________________________________________________________________________
// INCLUDES                                                          ( win32 )
//
#ifdef _WIN32
#define _WIN32_DCOM
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <mstcpip.h>
#include <iostream>
#include <wbemidl.h>
#pragma warning(disable : 4996)
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "Mswsock.lib")
// ___________________________________________________________________________
// INCLUDES                                                          ( linux )
//
#elif __linux__
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif
// ___________________________________________________________________________
// TYPEs                                                            ( common )
//
using u8p = const uint8_t*;
using u8 = uint8_t;
using u16 = uint16_t;

#endif
