#include <cstdint>
#include <iostream>
#include <string>

#include "Agents/SwarmingAgent.hpp"
#include "Worlds/StepMazeWorld.hpp"
#include "core/BenchRunner.hpp"
#include "core/MetricCollector.hpp"

using cse498::Direction;
using cse498::SwarmingAgent;
using cse498::TrafficData;
using cse498::WorldPosition;
using cse498::benchmarking::BENCH_START;
using cse498::benchmarking::BENCH_STOP;
using cse498::benchmarking::BenchRunner;
using cse498::benchmarking::MetricCollectorError;
using cse498::benchmarking::Params;
using cse498::benchmarking::RegistrationResult;
using cse498::benchmarking::ReportFormat;
using cse498::benchmarking::RunAndReportStatus;

int main() {
  BenchRunner runner{};
  const std::string benchmark_id = "step_maze_world";

  const auto registration = runner.Register(benchmark_id, [](Params& params) {
    if (BENCH_START() != MetricCollectorError::Ok) {
      return;
    }

    cse498::StepMazeWorld world{};

    for (std::uint32_t i = 0; i < params.agent_count; ++i) {
      TrafficData data{
          WorldPosition{7, 5},
          WorldPosition{3, 1},
          Direction::North,
          true,
      };
      world.AddAgent<SwarmingAgent<TrafficData>>(data);
    }

    for (std::uint32_t tick = 0; tick < params.tick_count; ++tick) {
      world.RunAgents();
      world.UpdateWorld();
    }

    (void)BENCH_STOP();
  });

  if (registration != RegistrationResult::Inserted) {
    std::cerr << "Failed to register benchmark" << std::endl;
    return 1;
  }

  Params params{};
  params.benchmark_name = benchmark_id;
  params.agent_count = 100;
  params.tick_count = 1000;
  params.warmup_ticks = 1;
  params.repetitions = 10;
  params.seed = 42;

  const auto run_result = runner.RunBenchmarkAndWriteReport(
      benchmark_id,
      params,
      "StepMazeWorld",
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
