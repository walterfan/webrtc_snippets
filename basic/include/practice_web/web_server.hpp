#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

namespace practice_web {

struct ServerOptions {
  std::filesystem::path exercises_root;
  std::filesystem::path web_root;
  std::filesystem::path build_directory;
  std::string cmake_command;
  std::string ctest_command;
  std::uint16_t port{8080};
};

[[nodiscard]] int run_web_server(const ServerOptions &options);

} // namespace practice_web
