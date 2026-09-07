#include "practice_web/exercise_repository.hpp"

#include "practice/exercise.hpp"

#include <fstream>
#include <iterator>
#include <system_error>

namespace practice_web {
namespace {

constexpr std::uintmax_t kMaximumFileBytes = 1024U * 1024U;

[[nodiscard]] std::string day_prefix(int day) {
  return day < 10 ? "0" + std::to_string(day) : std::to_string(day);
}

[[nodiscard]] bool is_within(const std::filesystem::path &base,
                             const std::filesystem::path &candidate) {
  auto base_it = base.begin();
  auto candidate_it = candidate.begin();
  while (base_it != base.end() && candidate_it != candidate.end() &&
         *base_it == *candidate_it) {
    ++base_it;
    ++candidate_it;
  }
  return base_it == base.end();
}

} // namespace

ExerciseRepository::ExerciseRepository(std::filesystem::path exercises_root) {
  std::error_code error;
  exercises_root_ = std::filesystem::weakly_canonical(exercises_root, error);
  if (error) {
    exercises_root_.clear();
  }
}

bool ExerciseRepository::is_valid_day(int day) noexcept {
  return day >= 1 && day <= 30;
}

std::vector<ExerciseSummary> ExerciseRepository::list() const {
  std::vector<ExerciseSummary> entries;
  entries.reserve(practice::catalog().exercises().size());
  for (const practice::Exercise &exercise : practice::catalog().exercises()) {
    entries.push_back({exercise.day, exercise.title, exercise.cpp_topics,
                       exercise.rtc_topics, exercise.summary});
  }
  return entries;
}

std::optional<ExerciseDetail> ExerciseRepository::detail(int day) const {
  const auto exercise = practice::catalog().find(day);
  const auto directory = exercise_directory(day);
  if (!exercise.has_value() || !directory.has_value()) {
    return std::nullopt;
  }

  const auto readme = read_allowed_file(*directory, "README.md");
  const auto header = read_allowed_file(*directory, "starter.hpp");
  const auto source = read_allowed_file(*directory, "starter.cpp");
  if (!readme.has_value() || !header.has_value() || !source.has_value()) {
    return std::nullopt;
  }

  const practice::Exercise &entry = exercise->get();
  return ExerciseDetail{
      {entry.day, entry.title, entry.cpp_topics, entry.rtc_topics,
       entry.summary},
      *readme,
      *header,
      *source,
      (std::filesystem::path("exercises") / directory->filename() /
       "starter.cpp")
          .generic_string(),
  };
}

std::optional<std::filesystem::path>
ExerciseRepository::exercise_directory(int day) const {
  if (!is_valid_day(day) || exercises_root_.empty()) {
    return std::nullopt;
  }

  const std::string prefix = day_prefix(day) + "_";
  std::error_code error;
  std::filesystem::directory_iterator iterator(exercises_root_, error);
  const std::filesystem::directory_iterator end;
  while (!error && iterator != end) {
    const std::filesystem::directory_entry &entry = *iterator;
    const std::string filename = entry.path().filename().string();
    if (filename.starts_with(prefix) && entry.is_directory(error)) {
      const std::filesystem::path canonical =
          std::filesystem::weakly_canonical(entry.path(), error);
      if (!error && is_within(exercises_root_, canonical)) {
        return canonical;
      }
    }
    iterator.increment(error);
  }
  return std::nullopt;
}

std::optional<std::string>
ExerciseRepository::read_allowed_file(const std::filesystem::path &directory,
                                      const char *filename) const {
  std::error_code error;
  const std::filesystem::path file =
      std::filesystem::weakly_canonical(directory / filename, error);
  if (error || !is_within(directory, file) ||
      !std::filesystem::is_regular_file(file, error) || error) {
    return std::nullopt;
  }

  const std::uintmax_t size = std::filesystem::file_size(file, error);
  if (error || size > kMaximumFileBytes) {
    return std::nullopt;
  }

  std::ifstream input(file, std::ios::binary);
  if (!input) {
    return std::nullopt;
  }
  return std::string(std::istreambuf_iterator<char>(input),
                     std::istreambuf_iterator<char>());
}

} // namespace practice_web
