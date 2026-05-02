#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "core/BenchRunner.hpp"
#include "core/MetricCollector.hpp"
#include "core/ReportGenerator.hpp"

using cse498::benchmarking::BENCH_START;
using cse498::benchmarking::BENCH_STOP;
using cse498::benchmarking::BenchRunner;
using cse498::benchmarking::BenchmarkReportEntry;
using cse498::benchmarking::MetricCollectorError;
using cse498::benchmarking::Params;
using cse498::benchmarking::ReportFormat;
using cse498::benchmarking::ReportGenerator;
using cse498::benchmarking::ReportStatus;
using cse498::benchmarking::RunStatus;

int main() {
  // Create the benchmark runner
  BenchRunner runner{};
  const std::string benchmark_id = "example_2";

  // Register a benchmark that performs a string manipulation loop
  const auto registration = runner.Register(benchmark_id, [](Params& params) {
    if (BENCH_START() != MetricCollectorError::Ok) {
      return;
    }

    std::string value;
    value.reserve(static_cast<std::size_t>(params.agent_count) * 8U + 64U);
    const std::size_t max_len = static_cast<std::size_t>(params.agent_count) * 8U + 64U;

    for (std::uint32_t i = 0; i < params.tick_count; ++i) {
      value.append("seed");
      value.push_back(static_cast<char>('a' + (i % 26U)));

      if (value.size() > max_len) {
        value.erase(0, value.size() - max_len);
      }
    }

    volatile std::size_t sink = value.size();
    (void)sink;

    (void)BENCH_STOP();
  });

  // Check registration success
  if (registration != cse498::benchmarking::RegistrationResult::Inserted) {
    std::cerr << "Failed to register benchmark" << std::endl;
    return 1;
  }

  // Set up benchmark parameters
  Params params{};
  params.benchmark_name = benchmark_id;
  params.agent_count = 100;
  params.tick_count = 50000;
  params.warmup_ticks = 1;
  params.repetitions = 10;
  params.seed = 42;

  // Run the benchmark by id and check for success
  const RunStatus run_status = runner.RunBenchmarkById(benchmark_id, params);
  if (run_status != RunStatus::Ok) {
    std::cerr << "Benchmark run failed" << std::endl;
    return 1;
  }

  // Get the last benchmark result and check that it was captured
  const auto result = runner.GetLastResult();
  if (!result.has_value()) {
    std::cerr << "No benchmark result captured" << std::endl;
    return 1;
  }

  const BenchmarkReportEntry entry{
      benchmark_id,
      params,
      *result,
  };

  const std::filesystem::path output_dir = "../benchmarking/groups/results";
  std::error_code ec;
  std::filesystem::create_directories(output_dir, ec);

  ReportGenerator generator{};
  const std::vector<BenchmarkReportEntry> entries{entry};

  const auto csv_status =
      generator.Write(output_dir / "test_benchmark.csv", ReportFormat::Csv, entries);
  const auto json_status =
      generator.Write(output_dir / "test_benchmark.json", ReportFormat::Json, entries);

  if (csv_status != ReportStatus::Ok || json_status != ReportStatus::Ok) {
    std::cerr << "Failed to write one or more benchmark reports" << std::endl;
    return 1;
  }

  std::cout << "Benchmark completed: " << benchmark_id << std::endl;
  std::cout << "Samples: " << result->sample_count << std::endl;
  std::cout << "Avg wall time (ns): " << result->avg_wall_time_ns << std::endl;
  std::cout << "Avg memory delta (KB): " << result->avg_memory_usage_kb << std::endl;
  std::cout << "CSV: " << (output_dir / "test_benchmark.csv").string() << std::endl;
  std::cout << "JSON: " << (output_dir / "test_benchmark.json").string() << std::endl;

  return 0;
}