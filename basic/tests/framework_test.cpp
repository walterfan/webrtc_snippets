#include "practice/exercise.hpp"
#include "practice/runner.hpp"

#include <iostream>
#include <sstream>
#include <string_view>
#include <vector>

namespace {

bool require(bool condition, std::string_view message) {
  if (!condition) {
    std::cerr << "FAILED: " << message << '\n';
  }
  return condition;
}

} // namespace

int main() {
  bool ok = true;
  const practice::Catalog &exercises = practice::catalog();
  ok &= require(exercises.exercises().size() == 30, "catalog has 30 exercises");
  ok &=
      require(exercises.validate().empty(), "catalog is contiguous and valid");
  ok &= require(exercises.find(1).has_value(), "day 01 exists");
  ok &= require(!exercises.find(31).has_value(), "day 31 is rejected");
  const practice::Catalog duplicate_catalog({
      {1, "one", "One", "starter", "1 min", "C++", "RTC", "", {}, {}},
      {1,
       "duplicate",
       "Duplicate",
       "starter",
       "1 min",
       "C++",
       "RTC",
       "",
       {},
       {}},
  });
  ok &= require(!duplicate_catalog.validate().empty(),
                "duplicate catalog is rejected");

  std::ostringstream rendered_states;
  practice::print_check_result(rendered_states,
                               {practice::CheckState::not_implemented, "todo"});
  practice::print_check_result(rendered_states,
                               {practice::CheckState::failed, "failed"});
  practice::print_check_result(rendered_states,
                               {practice::CheckState::passed, "passed"});
  ok &= require(rendered_states.str().find("not_implemented") !=
                        std::string::npos &&
                    rendered_states.str().find("failed") != std::string::npos &&
                    rendered_states.str().find("passed") != std::string::npos,
                "all check states are rendered");

  std::istringstream input;
  std::ostringstream output;
  std::ostringstream errors;
  ok &= require(practice::run_cli({"list"}, input, output, errors) == 0,
                "list succeeds");
  ok &= require(output.str().find("01") != std::string::npos,
                "list prints day 01");

  output.str({});
  output.clear();
  errors.str({});
  errors.clear();
  ok &= require(practice::run_cli({"check", "01"}, input, output, errors) == 2,
                "unfinished check returns 2");
  ok &= require(output.str().find("not_implemented") != std::string::npos,
                "unfinished state is visible");

  std::istringstream tui_input("info 01\nrun 01\ninvalid\nquit\n");
  output.str({});
  output.clear();
  errors.str({});
  errors.clear();
  ok &= require(practice::run_tui(tui_input, output, errors) == 0,
                "TUI exits cleanly");
  ok &= require(output.str().find("Parse SIP Request-Line") !=
                        std::string::npos,
                "TUI runs selected exercise");
  ok &= require(errors.str().find("Unknown command") != std::string::npos,
                "TUI recovers from invalid command");
  return ok ? 0 : 1;
}
