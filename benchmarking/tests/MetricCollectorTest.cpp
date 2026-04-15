#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <thread>

#include "core/MetricCollector.hpp"

using namespace cse498::benchmarking;

/**
 * @brief Tests for the MetricCollector class, which is responsible for collecting timing and memory usage metrics during benchmark execution.
 * Written with the help of AI
 */

TEST_CASE("MetricCollectorTest.StopBeforeStartReturnsTimingNotStarted", "[benchmarking][metric]") {
  MetricCollector collector{};
  const auto status = collector.BENCH_STOP();

  REQUIRE(status == MetricCollectorError::TimingNotStarted);
}

TEST_CASE("MetricCollectorTest.GetSampleFailsIfNeverStarted", "[benchmarking][metric]") {
  MetricCollector collector{};
  const auto sample = collector.GetSample();

  REQUIRE_FALSE(sample.success);
}

TEST_CASE("MetricCollectorTest.StartStopProducesSuccessfulSample", "[benchmarking][metric]") {
  MetricCollector collector{};

  const auto start_status = collector.BENCH_START();
  REQUIRE(start_status == MetricCollectorError::Ok);

  std::this_thread::sleep_for(std::chrono::milliseconds(1));

  const auto stop_status = collector.BENCH_STOP();
  REQUIRE(stop_status == MetricCollectorError::Ok);

  const auto sample = collector.GetSample();
  REQUIRE(sample.success);
  CHECK(sample.wall_time_ns > 0);
}

TEST_CASE("MetricCollectorTest.ThreadLocalFunctionsFailWithoutActiveCollector", "[benchmarking][metric]") {
  const auto start_status = BENCH_START();
  const auto stop_status = BENCH_STOP();
  const auto sample = GetLastSample();

  REQUIRE(start_status == MetricCollectorError::TimingNotStarted);
  REQUIRE(stop_status == MetricCollectorError::TimingNotStarted);
  REQUIRE_FALSE(sample.success);
}
