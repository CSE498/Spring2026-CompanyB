#include <catch2/catch_test_macros.hpp>

#include "core/BenchmarkTypes.hpp"

using namespace cse498::benchmarking;

/**
 * @brief Tests for the basic types used in the benchmarking framework, such as Params and BenchmarkResult.
 * Written with the help of AI
 */

TEST_CASE("BenchmarkTypesTest.ParamsIsValidWhenAllRequiredFieldsAreSet", "[benchmarking][types]") {
  Params params{};
  params.benchmark_name = "swarm_update";
  params.agent_count = 10;
  params.tick_count = 20;
  params.repetitions = 3;

  REQUIRE(params.IsValid());
}

TEST_CASE("BenchmarkTypesTest.ParamsIsInvalidWithMissingName", "[benchmarking][types]") {
  Params params{};
  params.benchmark_name = "";
  params.agent_count = 10;
  params.tick_count = 20;
  params.repetitions = 3;

  REQUIRE_FALSE(params.IsValid());
}

TEST_CASE("BenchmarkTypesTest.ParamsIsInvalidWithZeroAgentCount", "[benchmarking][types]") {
  Params params{};
  params.benchmark_name = "swarm_update";
  params.agent_count = 0;
  params.tick_count = 20;
  params.repetitions = 3;

  REQUIRE_FALSE(params.IsValid());
}

TEST_CASE("BenchmarkTypesTest.ParamsIsInvalidWithZeroTickCount", "[benchmarking][types]") {
  Params params{};
  params.benchmark_name = "swarm_update";
  params.agent_count = 10;
  params.tick_count = 0;
  params.repetitions = 3;

  REQUIRE_FALSE(params.IsValid());
}

TEST_CASE("BenchmarkTypesTest.ParamsIsInvalidWithZeroRepetitions", "[benchmarking][types]") {
  Params params{};
  params.benchmark_name = "swarm_update";
  params.agent_count = 10;
  params.tick_count = 20;
  params.repetitions = 0;

  REQUIRE_FALSE(params.IsValid());
}

TEST_CASE("BenchmarkTypesTest.BenchmarkResultDefaultsIncludeRssAtStopFields", "[benchmarking][types]") {
  BenchmarkResult result{};

  REQUIRE(result.avg_current_rss_at_stop_kb == 0);
  REQUIRE(result.min_current_rss_at_stop_kb == 0);
  REQUIRE(result.max_current_rss_at_stop_kb == 0);
  REQUIRE(result.stdev_current_rss_at_stop_kb == 0);
}
