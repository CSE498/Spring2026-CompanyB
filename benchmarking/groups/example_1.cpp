#include <iostream>
#include <string>

#include "core/BenchRunner.hpp"
#include "core/MetricCollector.hpp"

using cse498::benchmarking::BENCH_START;
using cse498::benchmarking::BENCH_STOP;
using cse498::benchmarking::BenchRunner;
using cse498::benchmarking::MetricCollectorError;
using cse498::benchmarking::Params;
using cse498::benchmarking::ReportFormat;
using cse498::benchmarking::RunAndReportStatus;

int main() {
  BenchRunner runner{};
  const std::string benchmark_id = "example_1";

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

  if (registration != cse498::benchmarking::RegistrationResult::Inserted) {
    std::cerr << "Failed to register benchmark" << std::endl;
    return 1;
  }

  Params params{};
  params.benchmark_name = benchmark_id;
  params.agent_count = 100;
  params.tick_count = 50000;
  params.warmup_ticks = 1;
  params.repetitions = 10;
  params.seed = 42;

  const auto run_result = runner.RunBenchmarkAndWriteReport(
      benchmark_id,
      params,
      "example_1",
      "../benchmarking/groups/results",
      ReportFormat::Csv);

  if (run_result.status != RunAndReportStatus::Ok) {
    std::cerr << "Benchmark run or report write failed" << std::endl;
    return 1;
  }

  std::cout << "Benchmark completed: " << benchmark_id << std::endl;
  std::cout << "Report written to: " << run_result.output_path.string() << std::endl;

  return 0;
}