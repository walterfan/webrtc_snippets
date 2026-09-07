#pragma once

#include <functional>
#include <iosfwd>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace practice {

enum class CheckState { not_implemented, failed, passed };

struct CheckResult {
  CheckState state{CheckState::not_implemented};
  std::string detail;
};

struct Exercise {
  int day{};
  std::string slug;
  std::string title;
  std::string difficulty;
  std::string estimate;
  std::string cpp_topics;
  std::string rtc_topics;
  std::string summary;
  std::function<int(std::ostream &)> run;
  std::function<CheckResult()> check;
};

class Catalog {
public:
  explicit Catalog(std::vector<Exercise> exercises);

  [[nodiscard]] const std::vector<Exercise> &exercises() const noexcept;
  [[nodiscard]] std::optional<std::reference_wrapper<const Exercise>>
  find(int day) const;
  [[nodiscard]] std::string validate() const;

private:
  std::vector<Exercise> exercises_;
};

[[nodiscard]] const Catalog &catalog();
[[nodiscard]] std::string_view check_state_name(CheckState state) noexcept;
void print_check_result(std::ostream &out, const CheckResult &result);

} // namespace practice
