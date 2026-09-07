#include "practice_web/web_server.hpp"

#include "practice_web/exercise_repository.hpp"
#include "practice_web/test_runner.hpp"

#include <crow.h>

#include <fstream>
#include <iterator>
#include <string>

namespace practice_web {
namespace {

[[nodiscard]] std::string read_web_asset(const std::filesystem::path &web_root,
                                         const char *filename) {
  std::ifstream input(web_root / filename, std::ios::binary);
  if (!input) {
    return {};
  }
  return {std::istreambuf_iterator<char>(input),
          std::istreambuf_iterator<char>()};
}

[[nodiscard]] crow::response json_response(int status,
                                           crow::json::wvalue body) {
  crow::response response(status, std::move(body));
  response.set_header("Content-Type", "application/json; charset=utf-8");
  response.set_header("Cache-Control", "no-store");
  return response;
}

[[nodiscard]] crow::response error_response(int status, const char *message) {
  crow::json::wvalue body;
  body["ok"] = false;
  body["error"] = message;
  return json_response(status, std::move(body));
}

[[nodiscard]] crow::json::wvalue
summary_to_json(const ExerciseSummary &summary) {
  crow::json::wvalue body;
  body["day"] = summary.day;
  body["title"] = summary.title;
  body["cpp_topics"] = summary.cpp_topics;
  body["rtc_topics"] = summary.rtc_topics;
  body["summary"] = summary.summary;
  return body;
}

} // namespace

int run_web_server(const ServerOptions &options) {
  ExerciseRepository repository(options.exercises_root);
  TestRunner runner(options.build_directory, options.cmake_command,
                    options.ctest_command);
  crow::SimpleApp app;
  app.loglevel(crow::LogLevel::Warning);

  CROW_ROUTE(app, "/")
  ([web_root = options.web_root]() {
    const std::string page = read_web_asset(web_root, "index.html");
    if (page.empty()) {
      return crow::response(500, "Learning site assets are unavailable.");
    }
    crow::response response(page);
    response.set_header("Content-Type", "text/html; charset=utf-8");
    response.set_header("Cache-Control", "no-store");
    return response;
  });

  CROW_ROUTE(app, "/app.js")
  ([web_root = options.web_root]() {
    const std::string script = read_web_asset(web_root, "app.js");
    if (script.empty()) {
      return crow::response(404);
    }
    crow::response response(script);
    response.set_header("Content-Type",
                        "application/javascript; charset=utf-8");
    response.set_header("Cache-Control", "no-store");
    return response;
  });

  CROW_ROUTE(app, "/vendor/prism.min.css")
  ([web_root = options.web_root]() {
    const std::string stylesheet =
        read_web_asset(web_root / "vendor", "prism.min.css");
    if (stylesheet.empty()) {
      return crow::response(404);
    }
    crow::response response(stylesheet);
    response.set_header("Content-Type", "text/css; charset=utf-8");
    response.set_header("Cache-Control", "public, max-age=86400");
    return response;
  });

  CROW_ROUTE(app, "/vendor/prism.js")([web_root = options.web_root]() {
    const std::string script = read_web_asset(web_root / "vendor", "prism.js");
    if (script.empty()) {
      return crow::response(404);
    }
    crow::response response(script);
    response.set_header("Content-Type",
                        "application/javascript; charset=utf-8");
    response.set_header("Cache-Control", "public, max-age=86400");
    return response;
  });

  CROW_ROUTE(app, "/vendor/prism-c.min.js")([web_root = options.web_root]() {
    const std::string script =
        read_web_asset(web_root / "vendor", "prism-c.min.js");
    if (script.empty()) {
      return crow::response(404);
    }
    crow::response response(script);
    response.set_header("Content-Type",
                        "application/javascript; charset=utf-8");
    response.set_header("Cache-Control", "public, max-age=86400");
    return response;
  });

  CROW_ROUTE(app, "/vendor/prism-cpp.min.js")([web_root = options.web_root]() {
    const std::string script =
        read_web_asset(web_root / "vendor", "prism-cpp.min.js");
    if (script.empty()) {
      return crow::response(404);
    }
    crow::response response(script);
    response.set_header("Content-Type",
                        "application/javascript; charset=utf-8");
    response.set_header("Cache-Control", "public, max-age=86400");
    return response;
  });

  CROW_ROUTE(app, "/health")([]() {
    crow::json::wvalue body;
    body["ok"] = true;
    return json_response(200, std::move(body));
  });

  CROW_ROUTE(app, "/api/exercises")([&repository]() {
    crow::json::wvalue::list entries;
    for (const ExerciseSummary &summary : repository.list()) {
      entries.emplace_back(summary_to_json(summary));
    }
    crow::json::wvalue body;
    body["exercises"] = std::move(entries);
    return json_response(200, std::move(body));
  });

  CROW_ROUTE(app, "/api/exercises/<int>")([&repository](int day) {
    const auto detail = repository.detail(day);
    if (!detail.has_value()) {
      return error_response(404, "Exercise not found.");
    }
    crow::json::wvalue body = summary_to_json(detail->summary);
    body["readme"] = detail->readme;
    body["header"] = detail->header;
    body["source"] = detail->source;
    body["source_path"] = detail->source_path;
    return json_response(200, std::move(body));
  });

  CROW_ROUTE(app, "/api/exercises/<int>/check")
      .methods(crow::HTTPMethod::POST)([&runner](int day) {
        if (!ExerciseRepository::is_valid_day(day)) {
          return error_response(404, "Exercise not found.");
        }
        const TestRunResult result = runner.run(day);
        crow::json::wvalue body;
        body["day"] = day;
        body["state"] = std::string(test_state_name(result.state));
        body["exit_code"] = result.exit_code;
        body["elapsed_ms"] = result.elapsed.count();
        body["output"] = result.output;
        return json_response(200, std::move(body));
      });

  CROW_ROUTE(app, "/api/exercises/<int>/run")
      .methods(crow::HTTPMethod::POST)([&runner](int day) {
        if (!ExerciseRepository::is_valid_day(day)) {
          return error_response(404, "Exercise not found.");
        }
        const TestRunResult result = runner.run_solution(day);
        crow::json::wvalue body;
        body["day"] = day;
        body["state"] = std::string(test_state_name(result.state));
        body["exit_code"] = result.exit_code;
        body["elapsed_ms"] = result.elapsed.count();
        body["output"] = result.output;
        return json_response(200, std::move(body));
      });

  app.bindaddr("127.0.0.1").port(options.port).multithreaded().run();
  return 0;
}

} // namespace practice_web
