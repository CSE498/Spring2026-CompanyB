#include <catch2/catch_test_macros.hpp>

#include <array>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "core/ReportGenerator.hpp"

using namespace cse498::benchmarking;

namespace {

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

TEST_CASE("ReportGenerator writes CSV output", "[benchmarking][report]") {
  const auto path = TempFile("benchmark_report_test.csv");
  std::filesystem::remove(path);

  ReportGenerator generator{};
  const auto status = generator.WriteCsv(path, std::array{MakeEntry()});

  REQUIRE(status == ReportStatus::Ok);
  REQUIRE(std::filesystem::exists(path));

  const auto content = ReadFile(path);
  CHECK(content.find("benchmark_name,agent_count,tick_count,warmup_ticks,repetitions,seed") != std::string::npos);
  CHECK(content.find("\"swarm_update\"") != std::string::npos);
  CHECK(content.find("1000") != std::string::npos);

  std::filesystem::remove(path);
}

TEST_CASE("ReportGenerator writes JSON output", "[benchmarking][report]") {
  const auto path = TempFile("benchmark_report_test.json");
  std::filesystem::remove(path);

  ReportGenerator generator{};
  const auto status = generator.WriteJson(path, std::array{MakeEntry()});

  REQUIRE(status == ReportStatus::Ok);
  REQUIRE(std::filesystem::exists(path));

  const auto content = ReadFile(path);
  CHECK(content.find("\"benchmarks\"") != std::string::npos);
  CHECK(content.find("\"benchmark_name\":\"swarm_update\"") != std::string::npos);
  CHECK(content.find("\"sample_count\":5") != std::string::npos);

  std::filesystem::remove(path);
}

TEST_CASE("ReportGenerator rejects empty result sets", "[benchmarking][report]") {
  ReportGenerator generator{};
  const auto status = generator.WriteCsv(TempFile("benchmark_report_empty.csv"), std::span<const BenchmarkReportEntry>{});

  CHECK(status == ReportStatus::EmptyResults);
}