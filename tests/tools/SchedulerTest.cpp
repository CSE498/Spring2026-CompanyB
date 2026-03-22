/**
 * @file test_scheduler.cpp
 * @brief Unit tests for the Scheduler class
 * @author Joshua Twumasi
 */

#include <cmath>
#include <limits>
#include <map>
// #include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include <catch2/catch_test_macros.hpp>

#include "../../source/tools/Scheduler.hpp"

using namespace cse498;

// Quick-start example: shows how to set up and use the Scheduler
TEST_CASE("Scheduler: Quick Start Usage Example", "[scheduler][example]") {
  // Create a scheduler (deterministic mode by default).
  Scheduler<int> scheduler;

  // Add processes with base weights (higher = more CPU time).
  REQUIRE(scheduler.AddProcess(1, 10.0).has_value());
  REQUIRE(scheduler.AddProcess(2, 5.0).has_value());
  REQUIRE(scheduler.AddProcess(3, 1.0).has_value());

  // Call GetNext() each cycle to get the next process to run.
  auto next = scheduler.GetNext();
  REQUIRE(next.has_value());

  // (Optional) Enable rebalancing to prevent starvation.
  scheduler.EnableRebalancing(true);
  REQUIRE(scheduler.SetWeightFloor(0.5).has_value());
  REQUIRE(scheduler.SetFrequencyPenalty(0.1).has_value());
  REQUIRE(scheduler.SetWaitBoostFactor(0.2).has_value());

  // (Optional) Enable failure handling with exponential backoff.
  scheduler.EnableFailureHandling(true);
  REQUIRE(scheduler.SetMaxConsecutiveFailures(3).has_value());

  // 6. Report success/failure after each execution.
  REQUIRE(scheduler.MarkProcessSuccess(*next).has_value());

  // Run a few cycles and confirm the scheduler keeps producing valid IDs.
  for (int i = 0; i < 20; ++i) {
    auto id = scheduler.GetNext();
    REQUIRE(id.has_value());
    CHECK((*id >= 1 && *id <= 3));
  }
}

// Test for Core Functionality

TEST_CASE("Scheduler: Basic Construction", "[scheduler]") {
  SECTION("Default constructor creates empty deterministic scheduler") {
    Scheduler<> scheduler;
    CHECK(scheduler.GetProcessCount() == 0);
    CHECK_FALSE(scheduler.HasProcesses());
    CHECK(scheduler.GetMode() == Scheduler<>::Mode::DETERMINISTIC);
  }

  SECTION("Constructor with mode creates empty scheduler") {
    Scheduler<size_t> prob_scheduler(Scheduler<size_t>::Mode::PROBABILISTIC);
    CHECK(prob_scheduler.GetProcessCount() == 0);
    CHECK(prob_scheduler.GetMode() == Scheduler<size_t>::Mode::PROBABILISTIC);
  }
}

TEST_CASE("Scheduler: Adding Processes", "[scheduler]") {
  Scheduler<size_t> scheduler;

  SECTION("Adding single process") {
    auto result = scheduler.AddProcess(1, 10.0);
    CHECK(result.has_value());
    CHECK(scheduler.HasProcesses());
    CHECK(scheduler.GetProcessCount() == 1);
    CHECK(scheduler.HasProcess(1));
    auto weight = scheduler.GetBaseWeight(1);
    REQUIRE(weight.has_value());
    CHECK(*weight == 10.0);
  }

  SECTION("Adding multiple processes") {
    CHECK(scheduler.AddProcess(1, 10.0).has_value());
    CHECK(scheduler.AddProcess(2, 5.0).has_value());
    CHECK(scheduler.AddProcess(3, 1.0).has_value());

    CHECK(scheduler.GetProcessCount() == 3);
    CHECK(scheduler.HasProcess(1));
    CHECK(scheduler.HasProcess(2));
    CHECK(scheduler.HasProcess(3));
    CHECK_FALSE(scheduler.HasProcess(4));
  }

  SECTION("Adding process with zero weight") {
    CHECK(scheduler.AddProcess(1, 0.0).has_value());
    auto weight = scheduler.GetBaseWeight(1);
    REQUIRE(weight.has_value());
    CHECK(*weight == 0.0);
    CHECK(scheduler.HasProcess(1));
  }

  SECTION("Adding duplicate process returns error") {
    CHECK(scheduler.AddProcess(1, 10.0).has_value());
    auto result = scheduler.AddProcess(1, 5.0);
    CHECK_FALSE(result.has_value());
    CHECK(result.error() == SchedulerError::ProcessAlreadyExists);
  }

  SECTION("Adding negative weight returns error") {
    auto result = scheduler.AddProcess(1, -5.0);
    CHECK_FALSE(result.has_value());
    CHECK(result.error() == SchedulerError::NegativeWeight);
  }

  SECTION("Adding NaN weight returns error") {
    auto result =
        scheduler.AddProcess(1, std::numeric_limits<double>::quiet_NaN());
    CHECK_FALSE(result.has_value());
    CHECK(result.error() == SchedulerError::InvalidWeight);
  }

  SECTION("Adding infinity weight returns error") {
    auto result =
        scheduler.AddProcess(1, std::numeric_limits<double>::infinity());
    CHECK_FALSE(result.has_value());
    CHECK(result.error() == SchedulerError::InvalidWeight);
  }

  SECTION("Adding negative infinity weight returns error") {
    auto result =
        scheduler.AddProcess(1, -std::numeric_limits<double>::infinity());
    CHECK_FALSE(result.has_value());
    CHECK(result.error() == SchedulerError::InvalidWeight);
  }
}

TEST_CASE("Scheduler: Removing Processes", "[scheduler]") {
  Scheduler<size_t> scheduler;
  REQUIRE(scheduler.AddProcess(1, 10.0).has_value());
  REQUIRE(scheduler.AddProcess(2, 5.0).has_value());

  SECTION("Removing existing process") {
    auto result = scheduler.RemoveProcess(1);
    CHECK(result.has_value());
    CHECK(scheduler.GetProcessCount() == 1);
    CHECK_FALSE(scheduler.HasProcess(1));
    CHECK(scheduler.HasProcess(2));
  }

  SECTION("Removing non-existent process returns error") {
    auto result = scheduler.RemoveProcess(99);
    CHECK_FALSE(result.has_value());
    CHECK(result.error() == SchedulerError::ProcessNotFound);
  }

  SECTION("Removing all processes") {
    CHECK(scheduler.RemoveProcess(1).has_value());
    CHECK(scheduler.RemoveProcess(2).has_value());
    CHECK(scheduler.GetProcessCount() == 0);
    CHECK_FALSE(scheduler.HasProcesses());
  }

  SECTION("Clear removes all processes and allows re-use") {
    scheduler.Clear();
    CHECK(scheduler.GetProcessCount() == 0);
    CHECK_FALSE(scheduler.HasProcesses());
    CHECK_FALSE(scheduler.HasProcess(1));
    CHECK_FALSE(scheduler.HasProcess(2));
    REQUIRE(scheduler.AddProcess(10, 1.0).has_value());
    REQUIRE(scheduler.AddProcess(20, 2.0).has_value());
    CHECK(scheduler.GetProcessCount() == 2);
    CHECK(scheduler.HasProcess(10));
    CHECK(scheduler.HasProcess(20));
  }
}

TEST_CASE("Scheduler: Weight Queries", "[scheduler]") {
  Scheduler<size_t> scheduler;
  REQUIRE(scheduler.AddProcess(1, 10.0).has_value());
  REQUIRE(scheduler.AddProcess(2, 20.0).has_value());
  REQUIRE(scheduler.AddProcess(3, 5.0).has_value());

  SECTION("GetBaseWeight returns correct values") {
    auto w1 = scheduler.GetBaseWeight(1);
    auto w2 = scheduler.GetBaseWeight(2);
    auto w3 = scheduler.GetBaseWeight(3);

    REQUIRE(w1.has_value());
    REQUIRE(w2.has_value());
    REQUIRE(w3.has_value());

    CHECK(*w1 == 10.0);
    CHECK(*w2 == 20.0);
    CHECK(*w3 == 5.0);
  }

  SECTION("GetBaseWeight for non-existent process returns error") {
    auto result = scheduler.GetBaseWeight(99);
    CHECK_FALSE(result.has_value());
    CHECK(result.error() == SchedulerError::ProcessNotFound);
  }

  SECTION("GetHighestWeight returns maximum") {
    auto highest = scheduler.GetHighestWeight();
    REQUIRE(highest.has_value());
    CHECK(*highest == 20.0);
  }

  SECTION("GetTotalWeight returns sum") {
    double total = scheduler.GetTotalWeight();
    CHECK(total == 35.0);
  }

  SECTION("GetHighestWeight on empty scheduler returns error") {
    Scheduler<size_t> empty_scheduler;
    auto result = empty_scheduler.GetHighestWeight();
    CHECK_FALSE(result.has_value());
    CHECK(result.error() == SchedulerError::EmptyScheduler);
  }
}

TEST_CASE("Scheduler: Deterministic GetNext", "[scheduler]") {
  Scheduler<size_t> scheduler(Scheduler<size_t>::Mode::DETERMINISTIC);
  REQUIRE(scheduler.AddProcess(1, 10.0).has_value());
  REQUIRE(scheduler.AddProcess(2, 5.0).has_value());
  REQUIRE(scheduler.AddProcess(3, 2.0).has_value());

  SECTION("GetNext returns process IDs proportional to weights") {
    std::map<size_t, size_t> counts;

    for (int i = 0; i < 170; ++i) {
      auto next = scheduler.GetNext();
      REQUIRE(next.has_value());
      counts[*next]++;
    }

    CHECK(counts[1] == 100);
    CHECK(counts[2] == 50);
    CHECK(counts[3] == 20);
  }

  SECTION("GetNext on empty scheduler returns error") {
    Scheduler<size_t> empty_scheduler;
    auto result = empty_scheduler.GetNext();
    CHECK_FALSE(result.has_value());
    CHECK(result.error() == SchedulerError::EmptyScheduler);
  }

  SECTION("Execution count is tracked correctly") {
    for (int i = 0; i < 17; ++i) {
      REQUIRE(scheduler.GetNext().has_value());
    }

    auto count1 = scheduler.GetExecutionCount(1);
    auto count2 = scheduler.GetExecutionCount(2);
    auto count3 = scheduler.GetExecutionCount(3);

    REQUIRE(count1.has_value());
    REQUIRE(count2.has_value());
    REQUIRE(count3.has_value());

    CHECK(*count1 == 10);
    CHECK(*count2 == 5);
    CHECK(*count3 == 2);
  }
}

TEST_CASE("Scheduler: Probabilistic GetNext", "[scheduler]") {
  Scheduler<size_t> scheduler(Scheduler<size_t>::Mode::PROBABILISTIC);
  REQUIRE(scheduler.AddProcess(1, 10.0).has_value());
  REQUIRE(scheduler.AddProcess(2, 5.0).has_value());
  REQUIRE(scheduler.AddProcess(3, 1.0).has_value());

  SECTION("GetNext returns valid process IDs") {
    for (int i = 0; i < 100; ++i) {
      auto next = scheduler.GetNext();
      REQUIRE(next.has_value());
      size_t id = *next;
      CHECK((id == 1 || id == 2 || id == 3));
    }
  }

  SECTION("Higher weights are selected more frequently (probabilistic test)") {
    std::map<size_t, size_t> counts;

    for (int i = 0; i < 1600; ++i) {
      auto next = scheduler.GetNext();
      REQUIRE(next.has_value());
      counts[*next]++;
    }

    CHECK(counts[1] > counts[2]);
    CHECK(counts[2] > counts[3]);
  }
}

TEST_CASE("Scheduler: Mode Switching", "[scheduler]") {
  Scheduler<size_t> scheduler(Scheduler<size_t>::Mode::DETERMINISTIC);
  REQUIRE(scheduler.AddProcess(1, 10.0).has_value());
  REQUIRE(scheduler.AddProcess(2, 5.0).has_value());

  SECTION("Mode starts as DETERMINISTIC") {
    CHECK(scheduler.GetMode() == Scheduler<size_t>::Mode::DETERMINISTIC);
  }

  SECTION("Switching to PROBABILISTIC") {
    scheduler.SetMode(Scheduler<size_t>::Mode::PROBABILISTIC);
    CHECK(scheduler.GetMode() == Scheduler<size_t>::Mode::PROBABILISTIC);
  }

  SECTION("Setting same mode does not reset round-robin state") {
    REQUIRE(scheduler.GetNext().has_value());
    REQUIRE(scheduler.GetNext().has_value());

    scheduler.SetMode(Scheduler<size_t>::Mode::DETERMINISTIC);

    auto next = scheduler.GetNext();
    REQUIRE(next.has_value());
    CHECK(*next == 2);
  }

  SECTION("Switching back to DETERMINISTIC resets current weights") {
    for (int i = 0; i < 5; ++i) {
      REQUIRE(scheduler.GetNext().has_value());
    }

    scheduler.SetMode(Scheduler<size_t>::Mode::PROBABILISTIC);
    scheduler.SetMode(Scheduler<size_t>::Mode::DETERMINISTIC);

    auto next = scheduler.GetNext();
    REQUIRE(next.has_value());
    CHECK(*next == 1);
  }
}

TEST_CASE("Scheduler: Execution Count Tracking", "[scheduler]") {
  Scheduler<size_t> scheduler;
  REQUIRE(scheduler.AddProcess(1, 5.0).has_value());
  REQUIRE(scheduler.AddProcess(2, 5.0).has_value());

  SECTION("Initial execution counts are zero") {
    auto count1 = scheduler.GetExecutionCount(1);
    auto count2 = scheduler.GetExecutionCount(2);

    REQUIRE(count1.has_value());
    REQUIRE(count2.has_value());

    CHECK(*count1 == 0);
    CHECK(*count2 == 0);
  }

  SECTION("Execution counts increment correctly") {
    for (int i = 0; i < 10; ++i) {
      REQUIRE(scheduler.GetNext().has_value());
    }

    auto count1 = scheduler.GetExecutionCount(1);
    auto count2 = scheduler.GetExecutionCount(2);

    REQUIRE(count1.has_value());
    REQUIRE(count2.has_value());

    size_t total = *count1 + *count2;
    CHECK(total == 10);
  }

  SECTION("GetExecutionCount for non-existent process returns error") {
    auto result = scheduler.GetExecutionCount(99);
    CHECK_FALSE(result.has_value());
    CHECK(result.error() == SchedulerError::ProcessNotFound);
  }
}

TEST_CASE("Scheduler: Template Instantiation with Different ID Types",
          "[scheduler]") {
  SECTION("Using std::string as ID type") {
    Scheduler<std::string> str_scheduler;
    REQUIRE(str_scheduler.AddProcess("high_priority", 10.0).has_value());
    REQUIRE(str_scheduler.AddProcess("low_priority", 1.0).has_value());

    CHECK(str_scheduler.GetProcessCount() == 2);
    CHECK(str_scheduler.HasProcess("high_priority"));
    CHECK(str_scheduler.HasProcess("low_priority"));

    auto next = str_scheduler.GetNext();
    REQUIRE(next.has_value());
    CHECK((*next == "high_priority" || *next == "low_priority"));
  }

  SECTION("Using int as ID type") {
    Scheduler<int> int_scheduler;
    REQUIRE(int_scheduler.AddProcess(-1, 5.0).has_value());
    REQUIRE(int_scheduler.AddProcess(100, 5.0).has_value());

    CHECK(int_scheduler.GetProcessCount() == 2);

    auto next = int_scheduler.GetNext();
    REQUIRE(next.has_value());
    CHECK((*next == -1 || *next == 100));
  }
}

// Tests: Dynamic Weight Adjustment

TEST_CASE("Scheduler: Rebalancing Enable/Disable", "[scheduler]") {
  Scheduler<size_t> scheduler;

  SECTION("Rebalancing disabled by default") {
    CHECK_FALSE(scheduler.IsRebalancingEnabled());
  }

  SECTION("Enable rebalancing") {
    scheduler.EnableRebalancing(true);
    CHECK(scheduler.IsRebalancingEnabled());
  }

  SECTION("Disable rebalancing") {
    scheduler.EnableRebalancing(true);
    scheduler.EnableRebalancing(false);
    CHECK_FALSE(scheduler.IsRebalancingEnabled());
  }
}

TEST_CASE("Scheduler: Rebalancing Configuration", "[scheduler]") {
  Scheduler<size_t> scheduler;

  SECTION("Default configuration values") {
    Scheduler<size_t>::Config defaults;
    CHECK(scheduler.GetWeightFloor() == defaults.weight_floor);
    CHECK(scheduler.GetWaitBoostFactor() == defaults.wait_boost_factor);
    CHECK(scheduler.GetFrequencyPenalty() == defaults.frequency_penalty);
  }

  SECTION("Set minimum weight") {
    auto result = scheduler.SetWeightFloor(0.5);
    CHECK(result.has_value());
    CHECK(scheduler.GetWeightFloor() == 0.5);
  }

  SECTION("Set wait boost factor") {
    auto result = scheduler.SetWaitBoostFactor(0.2);
    CHECK(result.has_value());
    CHECK(scheduler.GetWaitBoostFactor() == 0.2);
  }

  SECTION("Set frequency penalty") {
    auto result = scheduler.SetFrequencyPenalty(0.1);
    CHECK(result.has_value());
    CHECK(scheduler.GetFrequencyPenalty() == 0.1);
  }

  SECTION("Invalid minimum weight returns error") {
    auto result = scheduler.SetWeightFloor(-0.1);
    CHECK_FALSE(result.has_value());
    CHECK(result.error() == SchedulerError::NegativeWeight);
  }

  SECTION("Invalid wait boost factor returns error") {
    auto result = scheduler.SetWaitBoostFactor(-0.5);
    CHECK_FALSE(result.has_value());
    CHECK(result.error() == SchedulerError::InvalidParameter);
  }

  SECTION("Invalid frequency penalty returns error") {
    auto result = scheduler.SetFrequencyPenalty(1.5);
    CHECK_FALSE(result.has_value());
    CHECK(result.error() == SchedulerError::InvalidParameter);
  }
}

TEST_CASE("Scheduler: Manual Weight Setting", "[scheduler]") {
  Scheduler<size_t> scheduler;
  REQUIRE(scheduler.AddProcess(1, 10.0).has_value());

  SECTION("Set process weight") {
    auto result = scheduler.SetBaseWeight(1, 20.0);
    CHECK(result.has_value());

    auto weight = scheduler.GetBaseWeight(1);
    REQUIRE(weight.has_value());
    CHECK(*weight == 20.0);
  }

  SECTION("Set weight for non-existent process returns error") {
    auto result = scheduler.SetBaseWeight(99, 10.0);
    CHECK_FALSE(result.has_value());
    CHECK(result.error() == SchedulerError::ProcessNotFound);
  }

  SECTION("Set negative weight returns error") {
    auto result = scheduler.SetBaseWeight(1, -5.0);
    CHECK_FALSE(result.has_value());
    CHECK(result.error() == SchedulerError::NegativeWeight);
  }

  SECTION("Set NaN weight returns error") {
    auto result =
        scheduler.SetBaseWeight(1, std::numeric_limits<double>::quiet_NaN());
    CHECK_FALSE(result.has_value());
    CHECK(result.error() == SchedulerError::InvalidWeight);
  }

  SECTION("Set infinity weight returns error") {
    auto result =
        scheduler.SetBaseWeight(1, std::numeric_limits<double>::infinity());
    CHECK_FALSE(result.has_value());
    CHECK(result.error() == SchedulerError::InvalidWeight);
  }

  SECTION("SetBaseWeight returns error when rebalancing is enabled") {
    scheduler.EnableRebalancing(true);
    auto result = scheduler.SetBaseWeight(1, 20.0);
    CHECK_FALSE(result.has_value());
    CHECK(result.error() == SchedulerError::WeightBlockedByRebalancing);
  }

  SECTION("SetBaseWeight succeeds after disabling rebalancing") {
    scheduler.EnableRebalancing(true);
    scheduler.EnableRebalancing(false);
    auto result = scheduler.SetBaseWeight(1, 20.0);
    CHECK(result.has_value());
    auto weight = scheduler.GetBaseWeight(1);
    REQUIRE(weight.has_value());
    CHECK(*weight == 20.0);
  }
}

TEST_CASE("Scheduler: Wait Cycle Tracking", "[scheduler]") {
  Scheduler<size_t> scheduler;
  REQUIRE(scheduler.AddProcess(1, 10.0).has_value());
  REQUIRE(scheduler.AddProcess(2, 1.0).has_value());
  scheduler.EnableRebalancing(true);

  SECTION("Wait cycles increment for non-executed processes") {
    for (int i = 0; i < 5; ++i) {
      REQUIRE(scheduler.GetNext().has_value());
    }

    auto wait1 = scheduler.GetWaitCycles(1);
    auto wait2 = scheduler.GetWaitCycles(2);

    REQUIRE(wait1.has_value());
    REQUIRE(wait2.has_value());

    CHECK(*wait2 > 0);
  }

  SECTION("GetWaitCycles for non-existent process returns error") {
    auto result = scheduler.GetWaitCycles(99);
    CHECK_FALSE(result.has_value());
    CHECK(result.error() == SchedulerError::ProcessNotFound);
  }
}

TEST_CASE("Scheduler: Rebalancing", "[scheduler]") {
  Scheduler<size_t> scheduler;
  REQUIRE(scheduler.AddProcess(1, 10.0).has_value());
  REQUIRE(scheduler.AddProcess(2, 5.0).has_value());
  scheduler.EnableRebalancing(true);
  REQUIRE(scheduler.SetFrequencyPenalty(0.1).has_value());

  SECTION("Adjusted weight decreases after execution") {
    auto initial = scheduler.GetAdjustedWeight(1);
    REQUIRE(initial.has_value());

    REQUIRE(scheduler.GetNext().has_value());

    auto after = scheduler.GetAdjustedWeight(1);
    REQUIRE(after.has_value());

    CHECK(*after < *initial);
  }

  SECTION("GetAdjustedWeight for non-existent process returns error") {
    auto result = scheduler.GetAdjustedWeight(99);
    CHECK_FALSE(result.has_value());
    CHECK(result.error() == SchedulerError::ProcessNotFound);
  }
}

TEST_CASE("Scheduler: Starvation Prevention", "[scheduler]") {
  Scheduler<size_t> scheduler;
  REQUIRE(scheduler.AddProcess(1, 100.0).has_value());
  REQUIRE(scheduler.AddProcess(2, 1.0).has_value());
  scheduler.EnableRebalancing(true);
  REQUIRE(scheduler.SetWaitBoostFactor(0.5).has_value());
  REQUIRE(scheduler.SetWeightFloor(0.1).has_value());

  SECTION("Low-weight process eventually gets scheduled") {
    bool process2_executed = false;

    for (int i = 0; i < 1000 && !process2_executed; ++i) {
      auto next = scheduler.GetNext();
      REQUIRE(next.has_value());

      if (*next == 2) {
        process2_executed = true;
      }
    }

    CHECK(process2_executed);
  }
}

TEST_CASE("Scheduler: Scheduling Cycle Counter", "[scheduler]") {
  Scheduler<size_t> scheduler;
  REQUIRE(scheduler.AddProcess(1, 5.0).has_value());

  SECTION("Scheduling cycle starts at 0") {
    CHECK(scheduler.GetSchedulingCycle() == 0);
  }

  SECTION("Scheduling cycle increments with GetNext") {
    for (int i = 0; i < 10; ++i) {
      REQUIRE(scheduler.GetNext().has_value());
    }

    CHECK(scheduler.GetSchedulingCycle() == 10);
  }
}

TEST_CASE("Scheduler: Reset Dynamic Weights", "[scheduler]") {
  Scheduler<size_t> scheduler;
  REQUIRE(scheduler.AddProcess(1, 10.0).has_value());
  REQUIRE(scheduler.AddProcess(2, 5.0).has_value());
  scheduler.EnableRebalancing(true);

  SECTION("Reset clears all adjustments") {
    for (int i = 0; i < 20; ++i) {
      REQUIRE(scheduler.GetNext().has_value());
    }

    scheduler.ResetAdjustedWeights();

    auto adj1 = scheduler.GetAdjustedWeight(1);
    auto adj2 = scheduler.GetAdjustedWeight(2);
    auto weight1 = scheduler.GetBaseWeight(1);
    auto weight2 = scheduler.GetBaseWeight(2);

    REQUIRE(adj1.has_value());
    REQUIRE(adj2.has_value());
    REQUIRE(weight1.has_value());
    REQUIRE(weight2.has_value());

    CHECK(*adj1 == *weight1);
    CHECK(*adj2 == *weight2);

    auto wait1 = scheduler.GetWaitCycles(1);
    auto wait2 = scheduler.GetWaitCycles(2);

    REQUIRE(wait1.has_value());
    REQUIRE(wait2.has_value());

    CHECK(*wait1 == 0);
    CHECK(*wait2 == 0);
  }
}

// Tests: Failure Handling & Recovery

TEST_CASE("Scheduler: Failure Handling Enable/Disable", "[scheduler]") {
  Scheduler<size_t> scheduler;

  SECTION("Failure handling disabled by default") {
    CHECK_FALSE(scheduler.IsFailureHandlingEnabled());
  }

  SECTION("Enable failure handling") {
    scheduler.EnableFailureHandling(true);
    CHECK(scheduler.IsFailureHandlingEnabled());
  }

  SECTION("Disable failure handling") {
    scheduler.EnableFailureHandling(true);
    scheduler.EnableFailureHandling(false);
    CHECK_FALSE(scheduler.IsFailureHandlingEnabled());
  }

  SECTION(
      "Disabling failure handling does not re-enable manually disabled "
      "processes") {
    REQUIRE(scheduler.AddProcess(1, 5.0).has_value());
    REQUIRE(scheduler.AddProcess(2, 5.0).has_value());
    REQUIRE(scheduler.DisableProcess(1).has_value());
    scheduler.EnableFailureHandling(true);
    scheduler.EnableFailureHandling(false);
    auto enabled = scheduler.IsProcessEnabled(1);
    REQUIRE(enabled.has_value());
    CHECK_FALSE(*enabled);
  }

  SECTION(
      "Disabling failure handling does not re-enable auto-disabled processes") {
    REQUIRE(scheduler.AddProcess(1, 5.0).has_value());
    REQUIRE(scheduler.AddProcess(2, 5.0).has_value());
    scheduler.EnableFailureHandling(true);
    REQUIRE(scheduler.SetMaxConsecutiveFailures(2).has_value());
    REQUIRE(scheduler.MarkProcessFailed(1).has_value());
    REQUIRE(scheduler.MarkProcessFailed(1).has_value());
    {
      auto eb = scheduler.IsProcessEnabled(1);
      REQUIRE(eb.has_value());
      CHECK_FALSE(*eb);
    }
    scheduler.EnableFailureHandling(false);
    auto enabled = scheduler.IsProcessEnabled(1);
    REQUIRE(enabled.has_value());
    CHECK_FALSE(*enabled);
  }
}

TEST_CASE("Scheduler: Process Enable/Disable", "[scheduler]") {
  Scheduler<size_t> scheduler;
  REQUIRE(scheduler.AddProcess(1, 10.0).has_value());

  SECTION("Process enabled by default") {
    auto enabled = scheduler.IsProcessEnabled(1);
    REQUIRE(enabled.has_value());
    CHECK(*enabled == true);
  }

  SECTION("Disable process") {
    CHECK(scheduler.DisableProcess(1).has_value());

    auto enabled = scheduler.IsProcessEnabled(1);
    REQUIRE(enabled.has_value());
    CHECK(*enabled == false);
  }

  SECTION("Enable disabled process") {
    REQUIRE(scheduler.DisableProcess(1).has_value());
    CHECK(scheduler.EnableProcess(1).has_value());

    auto enabled = scheduler.IsProcessEnabled(1);
    REQUIRE(enabled.has_value());
    CHECK(*enabled == true);
  }

  SECTION("Enable/Disable non-existent process returns error") {
    auto result1 = scheduler.EnableProcess(99);
    CHECK_FALSE(result1.has_value());
    CHECK(result1.error() == SchedulerError::ProcessNotFound);

    auto result2 = scheduler.DisableProcess(99);
    CHECK_FALSE(result2.has_value());
    CHECK(result2.error() == SchedulerError::ProcessNotFound);
  }
}

TEST_CASE("Scheduler: Failure Tracking", "[scheduler]") {
  Scheduler<size_t> scheduler;
  REQUIRE(scheduler.AddProcess(1, 10.0).has_value());
  scheduler.EnableFailureHandling(true);
  REQUIRE(scheduler.SetRecoverySuccessThreshold(3).has_value());

  SECTION("Initial failure and success counts are zero") {
    auto failures = scheduler.GetProcessFailureCount(1);
    auto successes = scheduler.GetProcessSuccessCount(1);
    auto total = scheduler.GetTotalFailures(1);

    REQUIRE(failures.has_value());
    REQUIRE(successes.has_value());
    REQUIRE(total.has_value());

    CHECK(*failures == 0);
    CHECK(*successes == 0);
    CHECK(*total == 0);
  }

  SECTION("Marking failure increments counts") {
    CHECK(scheduler.MarkProcessFailed(1).has_value());

    auto failures = scheduler.GetProcessFailureCount(1);
    auto total = scheduler.GetTotalFailures(1);

    REQUIRE(failures.has_value());
    REQUIRE(total.has_value());

    CHECK(*failures == 1);
    CHECK(*total == 1);
  }

  SECTION("Marking success increments success count") {
    CHECK(scheduler.MarkProcessSuccess(1).has_value());

    auto successes = scheduler.GetProcessSuccessCount(1);
    REQUIRE(successes.has_value());
    CHECK(*successes == 1);
  }

  SECTION("Failure resets success count") {
    REQUIRE(scheduler.MarkProcessSuccess(1).has_value());
    REQUIRE(scheduler.MarkProcessSuccess(1).has_value());
    REQUIRE(scheduler.MarkProcessFailed(1).has_value());

    auto successes = scheduler.GetProcessSuccessCount(1);
    REQUIRE(successes.has_value());
    CHECK(*successes == 0);
  }

  SECTION("Recovery after successes") {
    REQUIRE(scheduler.MarkProcessFailed(1).has_value());
    REQUIRE(scheduler.MarkProcessFailed(1).has_value());

    REQUIRE(scheduler.MarkProcessSuccess(1).has_value());
    REQUIRE(scheduler.MarkProcessSuccess(1).has_value());
    REQUIRE(scheduler.MarkProcessSuccess(1).has_value());

    auto failures = scheduler.GetProcessFailureCount(1);
    auto successes = scheduler.GetProcessSuccessCount(1);

    REQUIRE(failures.has_value());
    REQUIRE(successes.has_value());

    CHECK(*failures == 0);
    CHECK(*successes == 0);
  }

  SECTION("Mark non-existent process returns error") {
    auto result1 = scheduler.MarkProcessFailed(99);
    CHECK_FALSE(result1.has_value());
    CHECK(result1.error() == SchedulerError::ProcessNotFound);

    auto result2 = scheduler.MarkProcessSuccess(99);
    CHECK_FALSE(result2.has_value());
    CHECK(result2.error() == SchedulerError::ProcessNotFound);
  }
}

TEST_CASE("Scheduler: Exponential Backoff", "[scheduler]") {
  Scheduler<size_t> scheduler;
  REQUIRE(scheduler.AddProcess(1, 10.0).has_value());
  scheduler.EnableFailureHandling(true);
  scheduler.SetInitialBackoffCycles(2);
  REQUIRE(scheduler.SetBackoffMultiplier(2.0).has_value());

  SECTION("First failure sets initial backoff") {
    REQUIRE(scheduler.MarkProcessFailed(1).has_value());

    auto backoff = scheduler.GetBackoffCycles(1);
    REQUIRE(backoff.has_value());
    CHECK(*backoff == 2);
  }

  SECTION("Second failure doubles backoff") {
    REQUIRE(scheduler.MarkProcessFailed(1).has_value());
    REQUIRE(scheduler.MarkProcessFailed(1).has_value());

    auto backoff = scheduler.GetBackoffCycles(1);
    REQUIRE(backoff.has_value());
    CHECK(*backoff == 4);
  }

  SECTION("Third failure quadruples initial backoff") {
    REQUIRE(scheduler.MarkProcessFailed(1).has_value());
    REQUIRE(scheduler.MarkProcessFailed(1).has_value());
    REQUIRE(scheduler.MarkProcessFailed(1).has_value());

    auto backoff = scheduler.GetBackoffCycles(1);
    REQUIRE(backoff.has_value());
    CHECK(*backoff == 8);
  }
}

TEST_CASE("Scheduler: Max Backoff Cap", "[scheduler]") {
  Scheduler<size_t> scheduler;
  REQUIRE(scheduler.AddProcess(1, 10.0).has_value());
  scheduler.EnableFailureHandling(true);
  scheduler.SetInitialBackoffCycles(1);
  REQUIRE(scheduler.SetBackoffMultiplier(2.0).has_value());
  scheduler.SetMaxBackoffCycles(10);

  SECTION("Backoff capped at max") {
    for (int i = 0; i < 10; ++i) {
      REQUIRE(scheduler.MarkProcessFailed(1).has_value());
    }

    auto backoff = scheduler.GetBackoffCycles(1);
    REQUIRE(backoff.has_value());
    CHECK(*backoff <= 10);
  }
}

TEST_CASE("Scheduler: Auto-disable After Max Failures", "[scheduler]") {
  Scheduler<size_t> scheduler;
  REQUIRE(scheduler.AddProcess(1, 10.0).has_value());
  REQUIRE(scheduler.AddProcess(2, 10.0).has_value());
  scheduler.EnableFailureHandling(true);
  REQUIRE(scheduler.SetMaxConsecutiveFailures(3).has_value());

  SECTION("Process disabled after max failures") {
    for (int i = 0; i < 3; ++i) {
      REQUIRE(scheduler.MarkProcessFailed(1).has_value());
    }

    auto enabled = scheduler.IsProcessEnabled(1);
    REQUIRE(enabled.has_value());
    CHECK(*enabled == false);
  }

  SECTION("Disabled process not scheduled") {
    for (int i = 0; i < 3; ++i) {
      REQUIRE(scheduler.MarkProcessFailed(1).has_value());
    }

    auto schedulable = scheduler.IsProcessSchedulable(1);
    REQUIRE(schedulable.has_value());
    CHECK(*schedulable == false);

    for (int i = 0; i < 10; ++i) {
      auto next = scheduler.GetNext();
      REQUIRE(next.has_value());
      CHECK(*next == 2);
    }
  }
}

TEST_CASE("Scheduler: Recovery After Successes", "[scheduler]") {
  Scheduler<size_t> scheduler;
  REQUIRE(scheduler.AddProcess(1, 10.0).has_value());
  scheduler.EnableFailureHandling(true);
  REQUIRE(scheduler.SetRecoverySuccessThreshold(2).has_value());

  SECTION("Success streak clears failures") {
    REQUIRE(scheduler.MarkProcessFailed(1).has_value());
    REQUIRE(scheduler.MarkProcessFailed(1).has_value());

    auto before = scheduler.GetProcessFailureCount(1);
    REQUIRE(before.has_value());
    CHECK(*before == 2);

    REQUIRE(scheduler.MarkProcessSuccess(1).has_value());
    REQUIRE(scheduler.MarkProcessSuccess(1).has_value());

    auto after = scheduler.GetProcessFailureCount(1);
    auto backoff = scheduler.GetBackoffCycles(1);

    REQUIRE(after.has_value());
    REQUIRE(backoff.has_value());

    CHECK(*after == 0);
    CHECK(*backoff == 0);
  }
}

TEST_CASE("Scheduler: Failure Configuration", "[scheduler]") {
  Scheduler<size_t> scheduler;

  SECTION("Default configuration values") {
    Scheduler<size_t>::Config defaults;
    CHECK(scheduler.GetMaxConsecutiveFailures() ==
          defaults.max_consecutive_failures);
    CHECK(scheduler.GetInitialBackoffCycles() ==
          defaults.initial_backoff_cycles);
    CHECK(scheduler.GetBackoffMultiplier() == defaults.backoff_multiplier);
    CHECK(scheduler.GetMaxBackoffCycles() == defaults.max_backoff_cycles);
    CHECK(scheduler.GetRecoverySuccessThreshold() ==
          defaults.recovery_success_threshold);
  }

  SECTION("Set valid configuration") {
    CHECK(scheduler.SetMaxConsecutiveFailures(5).has_value());
    CHECK(scheduler.GetMaxConsecutiveFailures() == 5);

    scheduler.SetInitialBackoffCycles(4);
    CHECK(scheduler.GetInitialBackoffCycles() == 4);

    CHECK(scheduler.SetBackoffMultiplier(1.5).has_value());
    CHECK(scheduler.GetBackoffMultiplier() == 1.5);

    scheduler.SetMaxBackoffCycles(100);
    CHECK(scheduler.GetMaxBackoffCycles() == 100);

    CHECK(scheduler.SetRecoverySuccessThreshold(3).has_value());
    CHECK(scheduler.GetRecoverySuccessThreshold() == 3);
  }

  SECTION("Invalid configuration returns error") {
    auto result1 = scheduler.SetMaxConsecutiveFailures(0);
    CHECK_FALSE(result1.has_value());
    CHECK(result1.error() == SchedulerError::ZeroMaxFailures);

    auto result2 = scheduler.SetBackoffMultiplier(0.5);
    CHECK_FALSE(result2.has_value());
    CHECK(result2.error() == SchedulerError::InvalidBackoffMultiplier);

    auto result3 = scheduler.SetRecoverySuccessThreshold(0);
    CHECK_FALSE(result3.has_value());
    CHECK(result3.error() == SchedulerError::ZeroRecoveryThreshold);
  }
}

TEST_CASE("Scheduler: Process Count Queries", "[scheduler]") {
  Scheduler<size_t> scheduler;
  REQUIRE(scheduler.AddProcess(1, 10.0).has_value());
  REQUIRE(scheduler.AddProcess(2, 10.0).has_value());
  REQUIRE(scheduler.AddProcess(3, 10.0).has_value());

  SECTION("Total and enabled counts match when all enabled") {
    CHECK(scheduler.GetProcessCount() == 3);
    CHECK(scheduler.GetEnabledProcessCount() == 3);
    CHECK(scheduler.GetSchedulableProcessCount() == 3);
  }

  SECTION("Counts reflect disabled processes") {
    REQUIRE(scheduler.DisableProcess(1).has_value());

    CHECK(scheduler.GetProcessCount() == 3);
    CHECK(scheduler.GetEnabledProcessCount() == 2);
    CHECK(scheduler.GetSchedulableProcessCount() == 2);
  }

  SECTION("Counts reflect backoff processes") {
    scheduler.EnableFailureHandling(true);
    scheduler.SetInitialBackoffCycles(5);
    REQUIRE(scheduler.MarkProcessFailed(1).has_value());

    auto next = scheduler.GetNext();
    REQUIRE(next.has_value());

    CHECK(scheduler.GetProcessCount() == 3);
    CHECK(scheduler.GetEnabledProcessCount() == 3);
    CHECK(scheduler.GetSchedulableProcessCount() < 3);
  }
}

TEST_CASE("Scheduler: Failure Handling Integration", "[scheduler]") {
  Scheduler<size_t> scheduler;
  REQUIRE(scheduler.AddProcess(1, 10.0).has_value());
  REQUIRE(scheduler.AddProcess(2, 5.0).has_value());
  scheduler.EnableFailureHandling(true);
  REQUIRE(scheduler.SetMaxConsecutiveFailures(2).has_value());

  SECTION("Failed process skipped during scheduling") {
    REQUIRE(scheduler.MarkProcessFailed(1).has_value());
    REQUIRE(scheduler.MarkProcessFailed(1).has_value());

    for (int i = 0; i < 5; ++i) {
      auto next = scheduler.GetNext();
      REQUIRE(next.has_value());
      CHECK(*next == 2);
    }
  }

  SECTION("Re-enabled process can be scheduled again") {
    REQUIRE(scheduler.MarkProcessFailed(1).has_value());
    REQUIRE(scheduler.MarkProcessFailed(1).has_value());

    REQUIRE(scheduler.EnableProcess(1).has_value());

    bool process1_scheduled = false;
    for (int i = 0; i < 20; ++i) {
      auto next = scheduler.GetNext();
      REQUIRE(next.has_value());

      if (*next == 1) {
        process1_scheduled = true;
        break;
      }
    }

    CHECK(process1_scheduled);
  }
}

TEST_CASE("Scheduler: Zero-weight process", "[scheduler]") {
  Scheduler<size_t> scheduler;
  REQUIRE(scheduler.AddProcess(1, 0.0).has_value());
  REQUIRE(scheduler.AddProcess(2, 5.0).has_value());

  SECTION("Deterministic mode: zero-weight process is never selected") {
    size_t zero_count = 0;
    for (int i = 0; i < 100; ++i) {
      auto next = scheduler.GetNext();
      REQUIRE(next.has_value());
      if (*next == 1) ++zero_count;
    }
    CHECK(zero_count == 0);
  }

  SECTION("Probabilistic mode: zero-weight process is never selected") {
    scheduler.SetMode(Scheduler<size_t>::Mode::PROBABILISTIC);
    size_t zero_count = 0;
    for (int i = 0; i < 200; ++i) {
      auto next = scheduler.GetNext();
      REQUIRE(next.has_value());
      if (*next == 1) ++zero_count;
    }
    CHECK(zero_count == 0);
  }
}

TEST_CASE("Scheduler: All processes disabled", "[scheduler]") {
  Scheduler<size_t> scheduler;
  REQUIRE(scheduler.AddProcess(1, 3.0).has_value());
  REQUIRE(scheduler.AddProcess(2, 5.0).has_value());
  REQUIRE(scheduler.DisableProcess(1).has_value());
  REQUIRE(scheduler.DisableProcess(2).has_value());

  SECTION("GetNext returns NoSchedulableProcesses when all disabled") {
    auto next = scheduler.GetNext();
    REQUIRE_FALSE(next.has_value());
    CHECK(next.error() == SchedulerError::NoSchedulableProcesses);
  }
}

TEST_CASE("Scheduler: GetProcessCyclesUntilRetry countdown", "[scheduler]") {
  Scheduler<size_t> scheduler;
  scheduler.EnableFailureHandling(true);
  REQUIRE(scheduler.AddProcess(1, 5.0).has_value());
  REQUIRE(scheduler.AddProcess(2, 5.0).has_value());

  SECTION("Cycles until retry counts down after failure") {
    REQUIRE(scheduler.MarkProcessFailed(1).has_value());

    auto initial = scheduler.GetProcessCyclesUntilRetry(1);
    REQUIRE(initial.has_value());
    size_t start_cycles = *initial;
    CHECK(start_cycles > 0);

    for (size_t i = 0; i < start_cycles; ++i) {
      auto next = scheduler.GetNext();
      REQUIRE(next.has_value());
    }

    auto after = scheduler.GetProcessCyclesUntilRetry(1);
    REQUIRE(after.has_value());
    CHECK(*after == 0);
  }
}

TEST_CASE("Scheduler: SetMode mid-scheduling switches algorithm",
          "[scheduler]") {
  Scheduler<size_t> scheduler(Scheduler<size_t>::Mode::DETERMINISTIC, 42);
  REQUIRE(scheduler.AddProcess(1, 1.0).has_value());
  REQUIRE(scheduler.AddProcess(2, 3.0).has_value());

  for (int i = 0; i < 5; ++i) {
    auto next = scheduler.GetNext();
    REQUIRE(next.has_value());
  }

  SECTION("Switching to probabilistic mid-run does not error") {
    scheduler.SetMode(Scheduler<size_t>::Mode::PROBABILISTIC);
    CHECK(scheduler.GetMode() == Scheduler<size_t>::Mode::PROBABILISTIC);

    for (int i = 0; i < 10; ++i) {
      auto next = scheduler.GetNext();
      REQUIRE(next.has_value());
    }
  }

  SECTION("Switching back to deterministic mid-run resets round-robin state") {
    scheduler.SetMode(Scheduler<size_t>::Mode::PROBABILISTIC);
    for (int i = 0; i < 5; ++i) {
      auto next = scheduler.GetNext();
      REQUIRE(next.has_value());
    }
    scheduler.SetMode(Scheduler<size_t>::Mode::DETERMINISTIC);
    CHECK(scheduler.GetMode() == Scheduler<size_t>::Mode::DETERMINISTIC);

    auto next = scheduler.GetNext();
    REQUIRE(next.has_value());
  }
}
