#include "practice_web/web_server.hpp"

#include <charconv>
#include <cstdlib>
#include <iostream>
#include <string_view>

#ifndef PRACTICE_SOURCE_DIR
#error "PRACTICE_SOURCE_DIR must be configured by CMake"
#endif

#ifndef PRACTICE_CMAKE_COMMAND
#error "PRACTICE_CMAKE_COMMAND must be configured by CMake"
#endif

#ifndef PRACTICE_CTEST_COMMAND
#error "PRACTICE_CTEST_COMMAND must be configured by CMake"
#endif

namespace {

void print_usage(std::ostream &out) {
  out << "Usage: practice_web_server [--build-dir <path>] [--port <1-65535>]\n";
}

[[nodiscard]] bool parse_port(std::string_view value, std::uint16_t &port) {
  unsigned int parsed{};
  const auto [position, error] =
      std::from_chars(value.data(), value.data() + value.size(), parsed);
  if (error != std::errc{} || position != value.data() + value.size() ||
      parsed == 0U || parsed > 65535U) {
    return false;
  }
  port = static_cast<std::uint16_t>(parsed);
  return true;
}

} // namespace

int main(int argc, char **argv) {
  practice_web::ServerOptions options{
      std::filesystem::path(PRACTICE_SOURCE_DIR) / "exercises",
      std::filesystem::path(PRACTICE_SOURCE_DIR) / "web",
      "build/basic",
      PRACTICE_CMAKE_COMMAND,
      PRACTICE_CTEST_COMMAND,
      8080,
  };

  for (int index = 1; index < argc; ++index) {
    const std::string_view argument(argv[index]);
    if (argument == "--help") {
      print_usage(std::cout);
      return EXIT_SUCCESS;
    }
    if (argument == "--build-dir" && index + 1 < argc) {
      options.build_directory = argv[++index];
      continue;
    }
    if (argument == "--port" && index + 1 < argc &&
        parse_port(argv[++index], options.port)) {
      continue;
    }
    print_usage(std::cerr);
    return EXIT_FAILURE;
  }

  return practice_web::run_web_server(options);
}
