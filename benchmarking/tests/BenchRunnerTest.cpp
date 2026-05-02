#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <thread>

#include "core/BenchRunner.hpp"
#include "core/MetricCollector.hpp"

using namespace cse498::benchmarking;

namespace {

/**
 * @brief Tests for the BenchRunner class, which manages benchmark registration and execution.
 * These tests cover registration of benchmarks, handling of invalid input, and execution of benchmarks with proper
 * Written with the help of AI
 */

Params MakeValidParams() {
  Params params{};
  params.benchmark_name = "bench_1";
  params.agent_count = 16;
  params.tick_count = 20;
  params.warmup_ticks = 1;
  params.repetitions = 2;
  params.seed = 42;
  return params;
}

void RegisterSimpleTimedBenchmark(BenchRunner& runner, const std::string& id) {
  const auto registration_status = runner.Register(id, [](Params&) {
    REQUIRE(BENCH_START() == MetricCollectorError::Ok);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    REQUIRE(BENCH_STOP() == MetricCollectorError::Ok);
  });
  REQUIRE((registration_status == RegistrationResult::Inserted ||
           registration_status == RegistrationResult::Replaced));
}

}  // namespace

TEST_CASE("BenchRunnerTest.RegisterRejectsEmptyId", "[benchmarking][runner]") {
  BenchRunner runner{};
  const auto status = runner.Register("", [](Params&) {});

  REQUIRE(status == RegistrationResult::InvalidId);
}

TEST_CASE("BenchRunnerTest.RegisterRejectsInvalidCallable", "[benchmarking][runner]") {
  BenchRunner runner{};
  const auto status = runner.Register("bench", {});

  REQUIRE(status == RegistrationResult::InvalidCallable);
}

TEST_CASE("BenchRunnerTest.RegisterAndListBenchmarks", "[benchmarking][runner]") {
  BenchRunner runner{};

  REQUIRE(runner.Register("b1", [](Params&) {}) == RegistrationResult::Inserted);
  REQUIRE(runner.Register("b2", [](Params&) {}) == RegistrationResult::Inserted);

  REQUIRE(runner.HasBenchmark("b1"));
  REQUIRE(runner.HasBenchmark("b2"));
  REQUIRE(runner.Size() == 2U);

  const auto ids = runner.ListBenchmarkIds();
  REQUIRE(ids.size() == 2U);
  CHECK(ids[0] == "b1");
  CHECK(ids[1] == "b2");
}

TEST_CASE("BenchRunnerTest.RegisterReturnsReplacedForExistingId", "[benchmarking][runner]") {
  BenchRunner runner{};
  REQUIRE(runner.Register("b1", [](Params&) {}) == RegistrationResult::Inserted);

  const auto status = runner.Register("b1", [](Params&) {});
  REQUIRE(status == RegistrationResult::Replaced);
  REQUIRE(runner.Size() == 1U);
}

TEST_CASE("BenchRunnerTest.HasBenchmarkReturnsFalseForEmptyId", "[benchmarking][runner]") {
  BenchRunner runner{};
  REQUIRE_FALSE(runner.HasBenchmark(""));
}

TEST_CASE("BenchRunnerTest.RunBenchmarkReturnsUnknownForEmptyId", "[benchmarking][runner]") {
  BenchRunner runner{};
  const auto status = runner.RunBenchmarkById("", MakeValidParams());

  REQUIRE(status == RunStatus::UnknownBenchmark);
  REQUIRE_FALSE(runner.GetLastResult().has_value());
}

TEST_CASE("BenchRunnerTest.RunBenchmarkReturnsUnknownForMissingId", "[benchmarking][runner]") {
  BenchRunner runner{};
  const auto status = runner.RunBenchmarkById("missing", MakeValidParams());

  REQUIRE(status == RunStatus::UnknownBenchmark);
  REQUIRE_FALSE(runner.GetLastResult().has_value());
}

TEST_CASE("BenchRunnerTest.RunBenchmarkReturnsInvalidParams", "[benchmarking][runner]") {
  BenchRunner runner{};
  REQUIRE(runner.Register("bench_1", [](Params&) {}) == RegistrationResult::Inserted);

  auto invalid_params = MakeValidParams();
  invalid_params.benchmark_name.clear();

  const auto status = runner.RunBenchmarkById("bench_1", invalid_params);

  REQUIRE(status == RunStatus::InvalidParams);
  REQUIRE_FALSE(runner.GetLastResult().has_value());
}

TEST_CASE("BenchRunnerTest.RunBenchmarkReturnsMetricFailureWhenNoSampleIsRecorded", "[benchmarking][runner]") {
  BenchRunner runner{};
  REQUIRE(runner.Register("bench_1", [](Params&) {
    // Intentionally skip BENCH_START/BENCH_STOP to simulate benchmark misuse.
  }) == RegistrationResult::Inserted);

  const auto status = runner.RunBenchmarkById("bench_1", MakeValidParams());

  REQUIRE(status == RunStatus::MetricFailure);
  REQUIRE_FALSE(runner.GetLastResult().has_value());
}

TEST_CASE("BenchRunnerTest.RunBenchmarkReturnsFailureWhenCallableThrows", "[benchmarking][runner]") {
  BenchRunner runner{};
  REQUIRE(runner.Register("bench_1", [](Params&) {
    throw std::runtime_error("intentional failure for test");
  }) == RegistrationResult::Inserted);

  const auto status = runner.RunBenchmarkById("bench_1", MakeValidParams());

  REQUIRE(status == RunStatus::BenchmarkFailure);
  REQUIRE_FALSE(runner.GetLastResult().has_value());
}

TEST_CASE("BenchRunnerTest.RunBenchmarkProducesLastResult", "[benchmarking][runner]") {
  BenchRunner runner{};
  RegisterSimpleTimedBenchmark(runner, "bench_1");

  const auto params = MakeValidParams();
  const auto status = runner.RunBenchmarkById("bench_1", params);

  REQUIRE(status == RunStatus::Ok);
  const auto result = runner.GetLastResult();
  REQUIRE(result.has_value());
  CHECK(result->agent_count == params.agent_count);
  CHECK(result->sample_count == params.repetitions);
  CHECK(result->avg_wall_time_ns > 0);
  CHECK(result->avg_current_rss_at_stop_kb > 0);
  CHECK(result->min_current_rss_at_stop_kb > 0);
  CHECK(result->max_current_rss_at_stop_kb >= result->min_current_rss_at_stop_kb);
}

TEST_CASE("BenchRunnerTest.RunAndWriteReportRejectsInvalidFileName", "[benchmarking][runner]") {
  BenchRunner runner{};
  RegisterSimpleTimedBenchmark(runner, "bench_1");

  const auto run_and_report = runner.RunBenchmarkAndWriteReport(
      "bench_1",
      MakeValidParams(),
      "nested/file.csv");

  REQUIRE(run_and_report.status == RunAndReportStatus::InvalidOutputFileName);
}

TEST_CASE("BenchRunnerTest.RunAndWriteReportReturnsRunFailedForUnknownBenchmark", "[benchmarking][runner]") {
  BenchRunner runner{};

  const auto run_and_report = runner.RunBenchmarkAndWriteReport(
      "missing",
      MakeValidParams(),
      "bench_report.csv");

  REQUIRE(run_and_report.status == RunAndReportStatus::RunFailed);
  REQUIRE(run_and_report.run_status == RunStatus::UnknownBenchmark);
}

TEST_CASE("BenchRunnerTest.RunAndWriteReportReturnsDirectoryCreateFailed", "[benchmarking][runner]") {
  BenchRunner runner{};
  RegisterSimpleTimedBenchmark(runner, "bench_1");

  const auto temp_file = std::filesystem::temp_directory_path() / "benchmarking_runner_not_a_dir";
  std::filesystem::remove(temp_file);
  {
    std::ofstream output(temp_file);
    REQUIRE(output.is_open());
    output << "x";
  }

  const auto run_and_report = runner.RunBenchmarkAndWriteReport(
      "bench_1",
      MakeValidParams(),
      "bench_report.csv",
      temp_file,
      ReportFormat::Csv);

  REQUIRE(run_and_report.status == RunAndReportStatus::DirectoryCreateFailed);
  std::filesystem::remove(temp_file);
}

TEST_CASE("BenchRunnerTest.RunAndWriteReportReturnsReportWriteFailedForUnsupportedFormat", "[benchmarking][runner]") {
  BenchRunner runner{};
  RegisterSimpleTimedBenchmark(runner, "bench_1");

  const auto temp_output_dir =
      std::filesystem::temp_directory_path() / "benchmarking_runner_report_fail";
  std::filesystem::remove_all(temp_output_dir);

  const auto run_and_report = runner.RunBenchmarkAndWriteReport(
      "bench_1",
      MakeValidParams(),
      "bench_report",
      temp_output_dir,
      static_cast<ReportFormat>(999));

  REQUIRE(run_and_report.status == RunAndReportStatus::ReportWriteFailed);
  REQUIRE(run_and_report.run_status == RunStatus::Ok);
  REQUIRE(run_and_report.report_status == ReportStatus::UnsupportedFormat);

  std::filesystem::remove_all(temp_output_dir);
}

TEST_CASE("BenchRunnerTest.RunAndWriteReportCreatesOutputFile", "[benchmarking][runner]") {
  BenchRunner runner{};
  RegisterSimpleTimedBenchmark(runner, "bench_1");

  const auto temp_output_dir =
      std::filesystem::temp_directory_path() / "benchmarking_runner_test";
  std::filesystem::remove_all(temp_output_dir);

  const auto run_and_report = runner.RunBenchmarkAndWriteReport(
      "bench_1",
      MakeValidParams(),
      "bench_report",
      temp_output_dir,
      ReportFormat::Csv);

  REQUIRE(run_and_report.status == RunAndReportStatus::Ok);
  REQUIRE(run_and_report.run_status == RunStatus::Ok);
  REQUIRE(run_and_report.report_status == ReportStatus::Ok);
  REQUIRE(std::filesystem::exists(run_and_report.output_path));
  CHECK(run_and_report.output_path.extension() == ".csv");

  std::filesystem::remove_all(temp_output_dir);
}
