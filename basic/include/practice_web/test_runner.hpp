#pragma once

#include <chrono>
#include <filesystem>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace practice_web {

enum class TestState { passed, failed, skipped, build_error, timed_out };

struct Command {
  std::string executable;
  std::vector<std::string> arguments;
};

struct RunPlan {
  Command build;
  Command test;
};

struct TestRunResult {
  TestState state{TestState::failed};
  int exit_code{};
  std::chrono::milliseconds elapsed{};
  std::string output;
};

class TestRunner {
public:
  TestRunner(std::filesystem::path build_directory, std::string cmake_command,
             std::string ctest_command,
             std::chrono::milliseconds timeout = std::chrono::seconds(30));

  [[nodiscard]] std::optional<RunPlan> plan_for(int day) const;
  [[nodiscard]] TestRunResult run(int day) const;
  [[nodiscard]] TestRunResult run_solution(int day) const;

private:
  [[nodiscard]] TestRunResult run_command(const Command &command) const;

  std::filesystem::path build_directory_;
  std::string cmake_command_;
  std::string ctest_command_;
  std::chrono::milliseconds timeout_;
  mutable std::mutex mutex_;
};

[[nodiscard]] std::string_view test_state_name(TestState state) noexcept;

} // namespace practice_web
