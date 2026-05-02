#include <catch2/catch_test_macros.hpp>

#include <array>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "core/ReportGenerator.hpp"

using namespace cse498::benchmarking;

namespace {

/**
 * @brief Tests for the ReportGenerator class, which is responsible for generating benchmark reports in various formats.
 * Written with the help of AI
 */

BenchmarkReportEntry MakeEntry() {
  BenchmarkReportEntry entry{};
  entry.benchmark_name = "swarm_update";
  entry.params.benchmark_name = "swarm_update";
  entry.params.agent_count = 42;
  entry.params.tick_count = 100;
  entry.params.warmup_ticks = 3;
  entry.params.repetitions = 5;
  entry.params.seed = 7;
  entry.result.agent_count = 42;
  entry.result.avg_wall_time_ns = 1000;
  entry.result.min_wall_time_ns = 900;
  entry.result.max_wall_time_ns = 1200;
  entry.result.stdev_wall_time_ns = 100;
  entry.result.avg_memory_usage_kb = 12;
  entry.result.min_memory_usage_kb = 10;
  entry.result.max_memory_usage_kb = 15;
  entry.result.stdev_memory_usage_kb = 2;
  entry.result.avg_current_rss_at_stop_kb = 256;
  entry.result.min_current_rss_at_stop_kb = 240;
  entry.result.max_current_rss_at_stop_kb = 272;
  entry.result.stdev_current_rss_at_stop_kb = 8;
  entry.result.sample_count = 5;
  return entry;
}

std::filesystem::path TempFile(const char* name) {
  return std::filesystem::temp_directory_path() / name;
}

std::string ReadFile(const std::filesystem::path& path) {
  std::ifstream input(path);
  std::stringstream buffer;
  buffer << input.rdbuf();
  return buffer.str();
}

}  // namespace

TEST_CASE("ReportGeneratorTest.WriteCsvProducesExpectedContent", "[benchmarking][report]") {
  const auto path = TempFile("benchmark_report_test.csv");
  std::filesystem::remove(path);

  ReportGenerator generator{};
  const auto status = generator.WriteCsv(path, std::array{MakeEntry()});

  REQUIRE(status == ReportStatus::Ok);
  REQUIRE(std::filesystem::exists(path));

  const auto content = ReadFile(path);
  CHECK(content.find("benchmark_name,agent_count,tick_count,warmup_ticks,repetitions,seed") != std::string::npos);
  CHECK(content.find("avg_current_rss_at_stop_kb") != std::string::npos);
  CHECK(content.find("\"swarm_update\"") != std::string::npos);
  CHECK(content.find("1000") != std::string::npos);
  CHECK(content.find("256") != std::string::npos);

  std::filesystem::remove(path);
}

TEST_CASE("ReportGeneratorTest.WriteJsonProducesExpectedContent", "[benchmarking][report]") {
  const auto path = TempFile("benchmark_report_test.json");
  std::filesystem::remove(path);

  ReportGenerator generator{};
  const auto status = generator.WriteJson(path, std::array{MakeEntry()});

  REQUIRE(status == ReportStatus::Ok);
  REQUIRE(std::filesystem::exists(path));

  const auto content = ReadFile(path);
  CHECK(content.find("\"benchmarks\"") != std::string::npos);
  CHECK(content.find("\"benchmark_name\":\"swarm_update\"") != std::string::npos);
  CHECK(content.find("\"avg_current_rss_at_stop_kb\":256") != std::string::npos);
  CHECK(content.find("\"sample_count\":5") != std::string::npos);

  std::filesystem::remove(path);
}

TEST_CASE("ReportGeneratorTest.WriteReturnsUnsupportedFormatForInvalidValue", "[benchmarking][report]") {
  const auto invalid_format = static_cast<ReportFormat>(999);
  ReportGenerator generator{};
  const auto status = generator.Write(
      TempFile("benchmark_report_invalid.txt"),
      invalid_format,
      std::array{MakeEntry()});

  REQUIRE(status == ReportStatus::UnsupportedFormat);
}

TEST_CASE("ReportGeneratorTest.WriteRejectsEmptyResultSet", "[benchmarking][report]") {
  ReportGenerator generator{};
  const auto status = generator.WriteCsv(
      TempFile("benchmark_report_empty.csv"),
      std::span<const BenchmarkReportEntry>{});

  REQUIRE(status == ReportStatus::EmptyResults);
}

TEST_CASE("ReportGeneratorTest.WriteCsvEscapesQuotedNames", "[benchmarking][report]") {
  auto entry = MakeEntry();
  entry.benchmark_name = "swarm\"update";

  const auto path = TempFile("benchmark_report_escape.csv");
  std::filesystem::remove(path);

  ReportGenerator generator{};
  const auto status = generator.WriteCsv(path, std::array{entry});

  REQUIRE(status == ReportStatus::Ok);
  const auto content = ReadFile(path);
  CHECK(content.find("\"swarm\"\"update\"") != std::string::npos);

  std::filesystem::remove(path);
}

TEST_CASE("ReportGeneratorTest.WriteJsonEscapesControlCharacters", "[benchmarking][report]") {
  auto entry = MakeEntry();
  entry.benchmark_name = "swarm\\update\nline\"quoted\"";
  entry.params.benchmark_name = entry.benchmark_name;

  const auto path = TempFile("benchmark_report_escape.json");
  std::filesystem::remove(path);

  ReportGenerator generator{};
  const auto status = generator.WriteJson(path, std::array{entry});

  REQUIRE(status == ReportStatus::Ok);
  const auto content = ReadFile(path);
  CHECK(content.find("swarm\\\\update") != std::string::npos);
  CHECK(content.find("line\\nline") == std::string::npos);
  CHECK(content.find("\\nline") != std::string::npos);
  CHECK(content.find("\\\"quoted\\\"") != std::string::npos);

  std::filesystem::remove(path);
}

TEST_CASE("ReportGeneratorTest.WriteCsvReturnsOpenFailedForMissingParentDirectory", "[benchmarking][report]") {
  const auto missing_parent =
      std::filesystem::temp_directory_path() / "benchmarking_report_missing_parent" / "out.csv";
  std::filesystem::remove_all(missing_parent.parent_path());

  ReportGenerator generator{};
  const auto status = generator.WriteCsv(missing_parent, std::array{MakeEntry()});

  REQUIRE(status == ReportStatus::OpenFailed);
}
