#include "practice_web/test_runner.hpp"

#include "practice_web/exercise_repository.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cerrno>
#include <fcntl.h>
#include <signal.h>
#include <spawn.h>
#include <string_view>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <utility>

extern char **environ;

namespace practice_web {
namespace {

constexpr std::size_t kMaximumOutputBytes = 64U * 1024U;

class ScopedFd {
public:
  explicit ScopedFd(int descriptor = -1) noexcept : descriptor_(descriptor) {}
  ~ScopedFd() { reset(); }

  ScopedFd(const ScopedFd &) = delete;
  ScopedFd &operator=(const ScopedFd &) = delete;

  ScopedFd(ScopedFd &&other) noexcept
      : descriptor_(std::exchange(other.descriptor_, -1)) {}
  ScopedFd &operator=(ScopedFd &&other) noexcept {
    if (this != &other) {
      reset();
      descriptor_ = std::exchange(other.descriptor_, -1);
    }
    return *this;
  }

  [[nodiscard]] int get() const noexcept { return descriptor_; }
  void reset() noexcept {
    if (descriptor_ >= 0) {
      (void)::close(descriptor_);
      descriptor_ = -1;
    }
  }

private:
  int descriptor_;
};

class SpawnActions {
public:
  SpawnActions()
      : initialized_(posix_spawn_file_actions_init(&actions_) == 0) {}
  ~SpawnActions() {
    if (initialized_) {
      (void)posix_spawn_file_actions_destroy(&actions_);
    }
  }

  SpawnActions(const SpawnActions &) = delete;
  SpawnActions &operator=(const SpawnActions &) = delete;

  [[nodiscard]] bool valid() const noexcept { return initialized_; }
  [[nodiscard]] posix_spawn_file_actions_t *get() noexcept { return &actions_; }

private:
  posix_spawn_file_actions_t actions_{};
  bool initialized_{};
};

[[nodiscard]] std::string day_pattern(int day) {
  const std::string prefix = day < 10 ? "0" : "";
  return "^Day" + prefix + std::to_string(day) + "\\.";
}

[[nodiscard]] std::string day_name(int day) {
  return day < 10 ? "0" + std::to_string(day) : std::to_string(day);
}

void append_output(std::string &output, std::string_view chunk) {
  if (output.size() >= kMaximumOutputBytes) {
    return;
  }
  const std::size_t remaining = kMaximumOutputBytes - output.size();
  output.append(chunk.substr(0, remaining));
}

void read_available(ScopedFd &descriptor, std::string &output) {
  std::array<char, 4096> buffer{};
  while (true) {
    const ssize_t count =
        ::read(descriptor.get(), buffer.data(), buffer.size());
    if (count > 0) {
      append_output(output, std::string_view(buffer.data(),
                                             static_cast<std::size_t>(count)));
      continue;
    }
    if (count < 0 && errno == EINTR) {
      continue;
    }
    return;
  }
}

[[nodiscard]] bool contains_skipped(std::string_view output) {
  std::string normalized(output);
  std::transform(normalized.begin(), normalized.end(), normalized.begin(),
                 [](unsigned char character) {
                   return static_cast<char>(std::tolower(character));
                 });
  return normalized.find("skipped") != std::string::npos;
}

} // namespace

TestRunner::TestRunner(std::filesystem::path build_directory,
                       std::string cmake_command, std::string ctest_command,
                       std::chrono::milliseconds timeout)
    : build_directory_(std::move(build_directory)),
      cmake_command_(std::move(cmake_command)),
      ctest_command_(std::move(ctest_command)), timeout_(timeout) {}

std::optional<RunPlan> TestRunner::plan_for(int day) const {
  if (!ExerciseRepository::is_valid_day(day) || build_directory_.empty() ||
      cmake_command_.empty() || ctest_command_.empty()) {
    return std::nullopt;
  }

  const std::string build_directory = build_directory_.string();
  return RunPlan{
      {cmake_command_,
       {"--build", build_directory, "--target", "practice_acceptance_test"}},
      {ctest_command_,
       {"--test-dir", build_directory, "--output-on-failure", "-R",
        day_pattern(day)}},
  };
}

TestRunResult TestRunner::run(int day) const {
  const std::lock_guard lock(mutex_);
  const auto plan = plan_for(day);
  if (!plan.has_value()) {
    return {TestState::failed, -1, std::chrono::milliseconds::zero(),
            "Invalid exercise day or server configuration."};
  }

  TestRunResult build_result = run_command(plan->build);
  if (build_result.state == TestState::timed_out) {
    build_result.state = TestState::timed_out;
    build_result.output.insert(0, "Build timed out.\n");
    return build_result;
  }
  if (build_result.exit_code != 0) {
    build_result.state = TestState::build_error;
    build_result.output.insert(0, "Build failed.\n");
    return build_result;
  }

  TestRunResult test_result = run_command(plan->test);
  test_result.elapsed += build_result.elapsed;
  test_result.output.insert(0, "Build succeeded.\n\nTest output:\n");
  if (test_result.state == TestState::timed_out) {
    test_result.output.insert(0, "Test timed out.\n");
    return test_result;
  }
  if (test_result.exit_code != 0) {
    test_result.state = TestState::failed;
    return test_result;
  }
  test_result.state = contains_skipped(test_result.output) ? TestState::skipped
                                                           : TestState::passed;
  return test_result;
}

TestRunResult TestRunner::run_solution(int day) const {
  const std::lock_guard lock(mutex_);
  if (!ExerciseRepository::is_valid_day(day) || build_directory_.empty() ||
      cmake_command_.empty()) {
    return {TestState::failed, -1, std::chrono::milliseconds::zero(),
            "Invalid exercise day or server configuration."};
  }

  const std::string target = "practice_" + day_name(day);
  const Command build{
      cmake_command_,
      {"--build", build_directory_.string(), "--target", target}};
  TestRunResult build_result = run_command(build);
  if (build_result.state == TestState::timed_out) {
    build_result.output.insert(0, "Build timed out.\n");
    return build_result;
  }
  if (build_result.exit_code != 0) {
    build_result.state = TestState::build_error;
    build_result.output.insert(0, "Build failed.\n");
    return build_result;
  }

  const Command solution{(build_directory_ / target).string(), {}};
  TestRunResult solution_result = run_command(solution);
  solution_result.elapsed += build_result.elapsed;
  solution_result.output.insert(0, "Build succeeded.\n\nrun() output:\n");
  if (solution_result.state == TestState::timed_out) {
    solution_result.output.insert(0, "run() timed out.\n");
    return solution_result;
  }
  solution_result.state =
      solution_result.exit_code == 0 ? TestState::passed : TestState::failed;
  return solution_result;
}

TestRunResult TestRunner::run_command(const Command &command) const {
  const auto started = std::chrono::steady_clock::now();
  int descriptors[2] = {-1, -1};
  if (::pipe(descriptors) != 0) {
    return {TestState::failed, -1, std::chrono::milliseconds::zero(),
            "Unable to create the local test-output pipe."};
  }
  ScopedFd read_end(descriptors[0]);
  ScopedFd write_end(descriptors[1]);

  const int current_flags = ::fcntl(read_end.get(), F_GETFL);
  if (current_flags < 0 ||
      ::fcntl(read_end.get(), F_SETFL, current_flags | O_NONBLOCK) != 0) {
    return {TestState::failed, -1, std::chrono::milliseconds::zero(),
            "Unable to prepare local test-output capture."};
  }

  SpawnActions actions;
  if (!actions.valid() ||
      posix_spawn_file_actions_adddup2(actions.get(), write_end.get(),
                                       STDOUT_FILENO) != 0 ||
      posix_spawn_file_actions_adddup2(actions.get(), write_end.get(),
                                       STDERR_FILENO) != 0 ||
      posix_spawn_file_actions_addclose(actions.get(), read_end.get()) != 0 ||
      posix_spawn_file_actions_addclose(actions.get(), write_end.get()) != 0) {
    return {TestState::failed, -1, std::chrono::milliseconds::zero(),
            "Unable to prepare the local test process."};
  }

  std::vector<char *> arguments;
  arguments.reserve(command.arguments.size() + 2U);
  arguments.push_back(const_cast<char *>(command.executable.c_str()));
  for (const std::string &argument : command.arguments) {
    arguments.push_back(const_cast<char *>(argument.c_str()));
  }
  arguments.push_back(nullptr);

  pid_t child{};
  const int spawn_error =
      posix_spawn(&child, command.executable.c_str(), actions.get(), nullptr,
                  arguments.data(), environ);
  write_end.reset();
  if (spawn_error != 0) {
    return {TestState::failed, -1, std::chrono::milliseconds::zero(),
            "Unable to start the local CMake/CTest command."};
  }

  std::string output;
  int status{};
  bool timed_out = false;
  while (true) {
    read_available(read_end, output);
    const pid_t waited = ::waitpid(child, &status, WNOHANG);
    if (waited == child) {
      break;
    }
    if (waited < 0) {
      if (errno == EINTR) {
        continue;
      }
      (void)::kill(child, SIGKILL);
      (void)::waitpid(child, &status, 0);
      return {TestState::failed, -1, std::chrono::milliseconds::zero(),
              "Unable to collect the local test process."};
    }
    if (std::chrono::steady_clock::now() - started > timeout_) {
      timed_out = true;
      (void)::kill(child, SIGKILL);
      (void)::waitpid(child, &status, 0);
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  read_available(read_end, output);
  if (output.size() == kMaximumOutputBytes) {
    output += "\n[output truncated]";
  }

  const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - started);
  if (timed_out) {
    return {TestState::timed_out, -1, elapsed, std::move(output)};
  }
  const int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
  return {exit_code == 0 ? TestState::passed : TestState::failed, exit_code,
          elapsed, std::move(output)};
}

std::string_view test_state_name(TestState state) noexcept {
  switch (state) {
  case TestState::passed:
    return "passed";
  case TestState::failed:
    return "failed";
  case TestState::skipped:
    return "skipped";
  case TestState::build_error:
    return "build_error";
  case TestState::timed_out:
    return "timed_out";
  }
  return "unknown";
}

} // namespace practice_web
