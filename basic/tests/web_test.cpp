#include "practice_web/exercise_repository.hpp"
#include "practice_web/test_runner.hpp"

#include <gtest/gtest.h>

namespace {

TEST(ExerciseRepositoryTest, ListsTheThirtyCatalogEntries) {
  const practice_web::ExerciseRepository repository(
      std::filesystem::path(PRACTICE_SOURCE_DIR) / "exercises");

  const auto exercises = repository.list();

  ASSERT_EQ(exercises.size(), 30U);
  EXPECT_EQ(exercises.front().day, 1);
  EXPECT_EQ(exercises.back().day, 30);
}

TEST(ExerciseRepositoryTest, ReadsOnlyKnownFilesForAValidDay) {
  const practice_web::ExerciseRepository repository(
      std::filesystem::path(PRACTICE_SOURCE_DIR) / "exercises");

  const auto detail = repository.detail(1);

  ASSERT_TRUE(detail.has_value());
  EXPECT_NE(detail->readme.find("Acceptance test"), std::string::npos);
  EXPECT_NE(detail->header.find("parse_request_line"), std::string::npos);
  EXPECT_NE(detail->source.find("TODO(day01)"), std::string::npos);
  EXPECT_EQ(detail->source_path, "exercises/01_sip_request_line/starter.cpp");
}

TEST(ExerciseRepositoryTest, RejectsUnknownDays) {
  const practice_web::ExerciseRepository repository(
      std::filesystem::path(PRACTICE_SOURCE_DIR) / "exercises");

  EXPECT_FALSE(repository.detail(0).has_value());
  EXPECT_FALSE(repository.detail(31).has_value());
}

TEST(TestRunnerTest, CreatesAFixedPlanForTheSelectedDay) {
  const practice_web::TestRunner runner("/safe/build", "/safe/cmake",
                                        "/safe/ctest");

  const auto plan = runner.plan_for(1);

  ASSERT_TRUE(plan.has_value());
  EXPECT_EQ(plan->build.executable, "/safe/cmake");
  EXPECT_EQ(plan->build.arguments,
            (std::vector<std::string>{"--build", "/safe/build", "--target",
                                      "practice_acceptance_test"}));
  EXPECT_EQ(plan->test.executable, "/safe/ctest");
  EXPECT_EQ(
      plan->test.arguments,
      (std::vector<std::string>{"--test-dir", "/safe/build",
                                "--output-on-failure", "-R", "^Day01\\."}));
}

TEST(TestRunnerTest, RejectsDaysOutsideTheFixedCatalog) {
  const practice_web::TestRunner runner("/safe/build", "/safe/cmake",
                                        "/safe/ctest");

  EXPECT_FALSE(runner.plan_for(-1).has_value());
  EXPECT_FALSE(runner.plan_for(31).has_value());
}

} // namespace
