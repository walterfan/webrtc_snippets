#pragma once

#include "practice/exercise.hpp"

#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace practice {

inline int check_ok(bool ok, std::string_view expr, std::string_view file,
                    int line, std::ostream &out) {
  if (ok) {
    return 0;
  }
  out << file << ':' << line << ": failed: " << expr << '\n';
  return 1;
}

template <typename Actual, typename Expected>
inline int check_eq(const Actual &actual, const Expected &expected,
                    std::string_view actual_expr,
                    std::string_view expected_expr, std::string_view file,
                    int line, std::ostream &out) {
  bool equal = false;
  if constexpr (std::is_integral_v<Actual> && std::is_integral_v<Expected>) {
    equal = std::cmp_equal(actual, expected);
  } else {
    equal = actual == expected;
  }
  if (equal) {
    return 0;
  }
  out << file << ':' << line << ": failed: " << actual_expr
      << " == " << expected_expr << "\n  actual:   " << actual
      << "\n  expected: " << expected << '\n';
  return 1;
}

inline CheckResult todo_check(std::string_view day, std::string_view sample) {
  return {CheckState::not_implemented,
          "Day " + std::string(day) +
              " is a starter skeleton. Implement the TODO and verify: " +
              std::string(sample)};
}

inline int render_todo(std::ostream &out, std::string_view day,
                       std::string_view title, std::string_view sample) {
  out << "Day " << day << ": " << title << '\n';
  out << "TODO: edit this day's starter.cpp, then run the GoogleTest "
         "acceptance "
         "case for day "
      << day << ".\n";
  out << "Sample: " << sample << '\n';
  return 0;
}


inline std::string_view trim(std::string_view text) {
  const auto first = text.find_first_not_of(" \t");
  if (first == std::string_view::npos) {
    return {};
  }
  const auto last = text.find_last_not_of(" \t");
  return text.substr(first, last - first + 1);
}

} // namespace practice

// Expands at the call site so __LINE__ and #cond capture the written check,
// including the expected value. Requires an `out` ostream in scope.
#define is_ok(cond)                                                            \
  ::practice::check_ok(static_cast<bool>(cond), #cond, __FILE__, __LINE__,     \
                       out)

#define is_eq(actual, expected)                                                \
  ::practice::check_eq((actual), (expected), #actual, #expected, __FILE__,     \
                       __LINE__, out)
