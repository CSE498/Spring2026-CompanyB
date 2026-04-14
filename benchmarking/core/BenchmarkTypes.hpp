#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace cse498::benchmarking {

struct Params {
  std::string benchmark_name{};
  std::uint32_t agent_count{1};
  std::uint32_t tick_count{1};
  std::uint32_t warmup_ticks{1};
  std::uint32_t repetitions{1};
  std::uint32_t seed{0};

  [[nodiscard]] bool IsValid() const noexcept {
    return !benchmark_name.empty() && agent_count >= 1 && tick_count >= 1 &&
           repetitions >= 1;
  }
};

struct MetricSample {
  std::uint64_t wall_time_ns{0};
  std::uint64_t memory_usage_kb{0};
  bool success{true};
  std::string error_message{};
};

struct BenchmarkResult {
  std::uint32_t agent_count{0};
  std::uint64_t avg_wall_time_ns{0};
  std::uint64_t min_wall_time_ns{0};
  std::uint64_t max_wall_time_ns{0};
  std::uint64_t stdev_wall_time_ns{0};
  std::uint64_t avg_memory_usage_kb{0};
  std::uint64_t min_memory_usage_kb{0};
  std::uint64_t max_memory_usage_kb{0};
  std::uint64_t stdev_memory_usage_kb{0};
  std::uint32_t sample_count{0};
};

enum class RegistrationResult {
  Inserted,
  Replaced,
  InvalidId,
  InvalidCallable,
};

enum class RunStatus {
  Ok,
  UnknownBenchmark,
  InvalidParams,
  BenchmarkFailure,
  MetricFailure,
};

enum class MetricCollectorError
{
  Ok,
  TimingNotStarted,
  MemoryReadFailure,
  ResourceExhaustion,
};

}  // namespace cse498::benchmarking