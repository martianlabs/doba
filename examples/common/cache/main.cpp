// Copyright 2026 martianLabs. Licensed under the Apache License, Version 2.0.
#include <iostream>
#include <memory>
#include <string>
#include "common/cache.h"

int main() {
  using namespace martianlabs::doba::common;
  cache<std::string, std::string> names({.max_entries = 2, .max_cost = 64});
  unsigned loads = 0;
  auto load = [&] { ++loads; return std::make_shared<const std::string>("Mars"); };
  auto first = names.get_or_load("planet", 4, load);
  auto second = names.get_or_load("planet", 4, load);
  if (!first || !second) return 1;
  std::cout << *second << ": " << loads << " load\n";
  names.erase("planet");
  std::cout << "After invalidation: " << *first << ", retained cost "
            << names.statistics().live_cost << '\n';
  first.reset(); second.reset();
  std::cout << "After releasing consumers: " << names.statistics().live_cost << '\n';
}
