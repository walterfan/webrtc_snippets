#include "practice/runner.hpp"

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#ifndef PRACTICE_DAY
#error "PRACTICE_DAY must be set by CMake"
#endif

#define PRACTICE_STRINGIFY_VALUE(value) #value
#define PRACTICE_STRINGIFY(value) PRACTICE_STRINGIFY_VALUE(value)

int main() {
  const std::string day = PRACTICE_STRINGIFY(PRACTICE_DAY);
  const std::vector<std::string_view> args{"run", day};
  return practice::run_cli(args, std::cin, std::cout, std::cerr);
}
