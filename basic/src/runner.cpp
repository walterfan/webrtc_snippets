#include "practice/runner.hpp"

#include "practice/exercise.hpp"

#include <charconv>
#include <cstddef>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace practice {
namespace {

constexpr std::size_t kMaxCommandLength = 256U;

void print_help(std::ostream &out) {
  out << "Commands: list | info <01-30> | run <01-30> | check <01-30> | "
         "progress | help | quit\n";
  out << "Non-interactive aliases: --list, --info, --run, --check, --help\n";
}

std::optional<int> parse_day(std::string_view text) {
  if (text.empty() || text.size() > 2U) {
    return std::nullopt;
  }
  int value = 0;
  const char *const begin = text.data();
  const char *const end = text.data() + text.size();
  const auto [position, error] = std::from_chars(begin, end, value);
  if (error != std::errc{} || position != end || value < 1 || value > 30) {
    return std::nullopt;
  }
  return value;
}

std::string two_digit(int day) {
  return day < 10 ? "0" + std::to_string(day) : std::to_string(day);
}

void print_exercise(std::ostream &out, const Exercise &exercise) {
  out << two_digit(exercise.day) << " — " << exercise.title << " ["
      << exercise.difficulty << ", " << exercise.estimate << "]\n";
  out << "C++: " << exercise.cpp_topics << '\n';
  out << "RTC: " << exercise.rtc_topics << '\n';
  out << exercise.summary << '\n';
}

int list_exercises(std::ostream &out) {
  for (const Exercise &exercise : catalog().exercises()) {
    out << two_digit(exercise.day) << "  " << exercise.title << " — "
        << exercise.cpp_topics << '\n';
  }
  return 0;
}

int run_progress(std::ostream &out) {
  std::size_t passed = 0;
  std::size_t unfinished = 0;
  std::size_t failed = 0;
  for (const Exercise &exercise : catalog().exercises()) {
    switch (exercise.check().state) {
    case CheckState::passed:
      ++passed;
      break;
    case CheckState::failed:
      ++failed;
      break;
    case CheckState::not_implemented:
      ++unfinished;
      break;
    }
  }
  out << "Progress: " << passed << "/30 passed, " << unfinished
      << " not implemented, " << failed << " failed\n";
  return 0;
}

std::optional<std::vector<std::string_view>> tokenize(std::string_view line) {
  if (line.empty() || line.size() > kMaxCommandLength) {
    return std::nullopt;
  }
  std::vector<std::string_view> tokens;
  std::size_t cursor = 0;
  while (cursor < line.size()) {
    while (cursor < line.size() && line[cursor] == ' ') {
      ++cursor;
    }
    const std::size_t start = cursor;
    while (cursor < line.size() && line[cursor] != ' ') {
      ++cursor;
    }
    if (start != cursor) {
      tokens.emplace_back(line.substr(start, cursor - start));
    }
    if (tokens.size() > 2U) {
      return std::nullopt;
    }
  }
  return tokens.empty() ? std::nullopt
                        : std::optional<std::vector<std::string_view>>(tokens);
}

std::string_view normalized_command(std::string_view command) {
  if (command.starts_with("--")) {
    return command.substr(2U);
  }
  return command;
}

} // namespace

int run_cli(const std::vector<std::string_view> &args, std::istream &in,
            std::ostream &out, std::ostream &err) {
  if (args.empty()) {
    return run_tui(in, out, err);
  }
  const std::string_view command = normalized_command(args.front());
  if (command == "help") {
    if (args.size() != 1U) {
      err << "Usage error.\n";
      return 1;
    }
    print_help(out);
    return 0;
  }
  if (command == "list") {
    if (args.size() != 1U) {
      err << "Usage error.\n";
      return 1;
    }
    return list_exercises(out);
  }
  if (command == "progress") {
    if (args.size() != 1U) {
      err << "Usage error.\n";
      return 1;
    }
    return run_progress(out);
  }
  if (command != "info" && command != "run" && command != "check") {
    err << "Unknown command. Use help for valid commands.\n";
    return 1;
  }
  if (args.size() != 2U) {
    err << "Usage error. Use " << command << " <01-30>.\n";
    return 1;
  }
  const std::optional<int> day = parse_day(args[1]);
  if (!day) {
    err << "Exercise id must be a number from 01 to 30.\n";
    return 1;
  }
  const auto exercise = catalog().find(*day);
  if (!exercise) {
    err << "Exercise is not registered.\n";
    return 1;
  }
  if (command == "info") {
    print_exercise(out, exercise->get());
    return 0;
  }
  if (command == "run") {
    print_exercise(out, exercise->get());
    int ret = exercise->get().run(out);
    out << "--- Exercise returned " << ret << " ---\n";
    return ret;
  }
  const CheckResult result = exercise->get().check();
  print_check_result(out, result);
  if (result.state == CheckState::passed) {
    return 0;
  }
  return result.state == CheckState::not_implemented ? 2 : 1;
}

int run_tui(std::istream &in, std::ostream &out, std::ostream &err) {
  out << "C++ SIP/WebRTC Practice Lab\n";
  print_help(out);
  std::string line;
  while (true) {
    out << "practice> " << std::flush;
    if (!std::getline(in, line)) {
      out << "\nGoodbye.\n";
      return 0;
    }
    const auto tokens = tokenize(line);
    if (!tokens) {
      err << "Invalid command. Use help for valid commands.\n";
      continue;
    }
    const std::string_view command = normalized_command(tokens->front());
    if (command == "quit" || command == "exit") {
      out << "Goodbye.\n";
      return 0;
    }
    if (command == "help") {
      if (tokens->size() == 1U) {
        print_help(out);
      } else {
        err << "Usage error.\n";
      }
      continue;
    }
    (void)run_cli(*tokens, in, out, err);
  }
}

} // namespace practice
