#include "practice/runner.hpp"

#include <iostream>
#include <string_view>
#include <vector>

int main(int argc, char *argv[]) {
  std::vector<std::string_view> args;
  args.reserve(static_cast<std::size_t>(argc > 1 ? argc - 1 : 0));
  for (int index = 1; index < argc; ++index) {
    args.emplace_back(argv[index]);
  }
  return practice::run_cli(args, std::cin, std::cout, std::cerr);
}
