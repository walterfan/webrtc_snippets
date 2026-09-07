#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace practice_web {

struct ExerciseSummary {
  int day{};
  std::string title;
  std::string cpp_topics;
  std::string rtc_topics;
  std::string summary;
};

struct ExerciseDetail {
  ExerciseSummary summary;
  std::string readme;
  std::string header;
  std::string source;
  std::string source_path;
};

class ExerciseRepository {
public:
  explicit ExerciseRepository(std::filesystem::path exercises_root);

  [[nodiscard]] std::vector<ExerciseSummary> list() const;
  [[nodiscard]] std::optional<ExerciseDetail> detail(int day) const;
  [[nodiscard]] static bool is_valid_day(int day) noexcept;

private:
  [[nodiscard]] std::optional<std::filesystem::path>
  exercise_directory(int day) const;
  [[nodiscard]] std::optional<std::string>
  read_allowed_file(const std::filesystem::path &directory,
                    const char *filename) const;

  std::filesystem::path exercises_root_;
};

} // namespace practice_web
