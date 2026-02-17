/**
 * @file example_scheduler_usage.cpp
 * @brief Example usage of the Scheduler class 
 * @author Joshua Twumasi
 * 
 * This file demonstrates usage patterns for the Scheduler class including
 * dynamic weight adjustment, starvation prevention, and failure handling.
 * 
 * Compile with: g++ -std=c++23 example_scheduler_usage.cpp -o example_scheduler
 */

#include "Scheduler.h"
#include <iostream>
#include <iomanip>

using namespace cse498;

// Helper function to unwrap expected or print error
template<typename T>
T unwrap_or_exit(std::expected<T, SchedulerError> result, const char* context) 
{
  if (!result) 
  {
    std::cerr << "Error in " << context << ": " << to_string(result.error()) << "\n";
    std::exit(1);
  }
  return *result;
}

void DemonstrateDeterministicScheduling() {
  std::cout << " Deterministic Scheduling Demo \n\n";
  
  Scheduler<size_t> scheduler(Scheduler<size_t>::Mode::DETERMINISTIC);
  
  // Add processes with different priorities
  scheduler.AddProcess(1, 10.0);  // High priority
  scheduler.AddProcess(2, 5.0);   // Medium priority
  scheduler.AddProcess(3, 1.0);   // Low priority
  
  std::cout << "Added 3 processes:\n";
  std::cout << "  Process 1: weight = 10.0\n";
  std::cout << "  Process 2: weight = 5.0\n";
  std::cout << "  Process 3: weight = 1.0\n";
  std::cout << "  Total weight: " << scheduler.GetTotalWeight() << "\n\n";
  
  std::cout << "Scheduling order (16 iterations):\n";
  for (int i = 0; i < 16; ++i) 
  {
    auto next_result = scheduler.GetNext();
    if (!next_result) 
    {
      std::cerr << "Error: " << to_string(next_result.error()) << "\n";
      return;
    }
    size_t next = *next_result;
    auto count = unwrap_or_exit(scheduler.GetExecutionCount(next), "GetExecutionCount");
    std::cout << "  Step " << std::setw(2) << (i+1) << ": Process " << next 
              << " (executed " << count << " times)\n";
  }
  
  std::cout << "\nFinal execution counts:\n";
  std::cout << "  Process 1: " << *scheduler.GetExecutionCount(1) << " times (62.5%)\n";
  std::cout << "  Process 2: " << *scheduler.GetExecutionCount(2) << " times (31.25%)\n";
  std::cout << "  Process 3: " << *scheduler.GetExecutionCount(3) << " times (6.25%)\n\n";
}

void DemonstrateProbabilisticScheduling() {
  std::cout << " Probabilistic Scheduling Demo \n\n";
  
  Scheduler<size_t> scheduler(Scheduler<size_t>::Mode::PROBABILISTIC);
  
  // Add same processes
  scheduler.AddProcess(1, 10.0);
  scheduler.AddProcess(2, 5.0);
  scheduler.AddProcess(3, 1.0);
  
  std::cout << "Running 1000 iterations with probabilistic selection...\n\n";
  
  for (int i = 0; i < 1000; ++i) 
  {
    auto result = scheduler.GetNext();
    if (!result) 
    {
      std::cerr << "Scheduling error: " << to_string(result.error()) << "\n";
      return;
    }
  }
  
  std::cout << "Execution distribution:\n";
  std::cout << "  Process 1: " << *scheduler.GetExecutionCount(1) 
            << " times (~62.5% expected)\n";
  std::cout << "  Process 2: " << *scheduler.GetExecutionCount(2) 
            << " times (~31.25% expected)\n";
  std::cout << "  Process 3: " << *scheduler.GetExecutionCount(3) 
            << " times (~6.25% expected)\n\n";
}

void DemonstrateStringIDs() {
  std::cout << " String ID Demo \n\n";
  
  Scheduler<std::string> scheduler;
  
  // Schedule tasks by name
  scheduler.AddProcess("render_graphics", 60.0);
  scheduler.AddProcess("update_physics", 30.0);
  scheduler.AddProcess("process_ai", 10.0);
  
  std::cout << "Scheduling game engine tasks (10 frames):\n";
  for (int frame = 0; frame < 10; ++frame) 
  {
    auto task_result = scheduler.GetNext();
    if (!task_result) 
    {
      std::cerr << "Error: " << to_string(task_result.error()) << "\n";
      return;
    }
    std::cout << "  Frame " << frame << ": " << *task_result << "\n";
  }
  std::cout << "\n";
}

void DemonstrateDynamicProcesses() {
  std::cout << " Dynamic Process Management Demo \n\n";
  
  Scheduler<size_t> scheduler;
  
  // Start with two processes
  scheduler.AddProcess(1, 10.0);
  scheduler.AddProcess(2, 10.0);
  
  std::cout << "Initial: 2 processes\n";
  for (int i = 0; i < 4; ++i) 
  {
    auto next = unwrap_or_exit(scheduler.GetNext(), "GetNext");
    std::cout << "  Scheduled: Process " << next << "\n";
  }
  
  // Add a high-priority process
  std::cout << "\nAdding high-priority Process 3 (weight=20)...\n";
  if (auto result = scheduler.AddProcess(3, 20.0); !result) 
  {
    std::cerr << "Failed to add process: " << to_string(result.error()) << "\n";
    return;
  }
  
  for (int i = 0; i < 6; ++i) 
  {
    auto next = unwrap_or_exit(scheduler.GetNext(), "GetNext");
    std::cout << "  Scheduled: Process " << next << "\n";
  }
  
  // Remove a process
  std::cout << "\nRemoving Process 2...\n";
  if (auto result = scheduler.RemoveProcess(2); !result) 
  {
    std::cerr << "Failed to remove process: " << to_string(result.error()) << "\n";
    return;
  }
  
  for (int i = 0; i < 4; ++i) 
  {
    auto next = unwrap_or_exit(scheduler.GetNext(), "GetNext");
    std::cout << "  Scheduled: Process " << next << "\n";
  }
  std::cout << "\n";
}

void DemonstrateDynamicWeightAdjustment() 
{
  std::cout << "Dynamic Weight Adjustment Demo \n\n";
  
  Scheduler<size_t> scheduler;
  scheduler.AddProcess(1, 10.0);
  scheduler.AddProcess(2, 10.0);
  scheduler.AddProcess(3, 10.0);
  
  // Configure auto-adjustment
  scheduler.EnableAutoAdjustment(true);
  scheduler.SetFrequencyPenalty(0.15);  // 15% reduction per execution
  scheduler.SetWaitBoostFactor(0.5);     // +0.5 weight per wait cycle
  scheduler.SetMinWeight(2.0);           // Prevent starvation
  
  std::cout << "Configuration:\n";
  std::cout << "  Frequency Penalty: 15% per execution\n";
  std::cout << "  Wait Boost: +0.5 per cycle waiting\n";
  std::cout << "  Minimum Weight: 2.0\n\n";
  
  std::cout << "Running 20 scheduling cycles...\n\n";
  std::cout << std::fixed << std::setprecision(2);
  
  for (int cycle = 1; cycle <= 20; ++cycle) 
  {
    auto next_result = scheduler.GetNext();
    if (!next_result) continue;
    size_t next = *next_result;
    
    if (cycle % 5 == 0)   // Show detailed stats every 5 cycles
    {
      std::cout << "After cycle " << cycle << ":\n";
      std::cout << "  Process 1: weight=" << *scheduler.GetDynamicWeight(1) 
                << ", wait=" << *scheduler.GetWaitCycles(1)
                << ", exec=" << *scheduler.GetExecutionCount(1) << "\n";
      std::cout << "  Process 2: weight=" << *scheduler.GetDynamicWeight(2)
                << ", wait=" << *scheduler.GetWaitCycles(2)
                << ", exec=" << *scheduler.GetExecutionCount(2) << "\n";
      std::cout << "  Process 3: weight=" << *scheduler.GetDynamicWeight(3)
                << ", wait=" << *scheduler.GetWaitCycles(3)
                << ", exec=" << *scheduler.GetExecutionCount(3) << "\n";
      std::cout << "  → Selected: Process " << next << "\n\n";
    }
  }
  
  std::cout << "Notice how:\n";
  std::cout << "  • Frequently executed processes have reduced weights\n";
  std::cout << "  • Waiting processes accumulate priority boosts\n";
  std::cout << "  • All weights stay above minimum (2.0)\n";
  std::cout << "  • Execution becomes more balanced over time\n\n";
}

void DemonstrateStarvationPrevention() 
{
  std::cout << " Starvation Prevention Demo \n\n";
  
  Scheduler<size_t> scheduler;
  scheduler.AddProcess(1, 100.0);  // Very high priority
  scheduler.AddProcess(2, 1.0);    // Very low priority
  
  std::cout << "Setup: Process 1 (weight=100), Process 2 (weight=1)\n\n";
  
  // Without auto-adjustment
  std::cout << "WITHOUT auto-adjustment (first 20 cycles):\n";
  scheduler.EnableAutoAdjustment(false);
  
  int count1_static = 0, count2_static = 0;
  for (int i = 0; i < 20; ++i) 
  {
    auto next = scheduler.GetNext();
    if (!next) continue;
    
    if (*next == 1) count1_static++;
    else count2_static++;
  }
  
  std::cout << "  Process 1: " << count1_static << " times\n";
  std::cout << "  Process 2: " << count2_static << " times\n";
  std::cout << "  → Process 2 is starved!\n\n";
  
  // With auto-adjustment
  std::cout << "WITH auto-adjustment (next 20 cycles):\n";
  scheduler.ResetDynamicWeights();
  scheduler.EnableAutoAdjustment(true);
  scheduler.SetMinWeight(5.0);
  scheduler.SetFrequencyPenalty(0.2);   // Aggressive
  scheduler.SetWaitBoostFactor(2.0);    // Strong boost
  
  int count1_dynamic = 0, count2_dynamic = 0;
  for (int i = 0; i < 20; ++i) 
  {
    auto next = scheduler.GetNext();
    if (!next) continue;
    
    if (*next == 1) count1_dynamic++;
    else count2_dynamic++;
  }
  
  std::cout << "  Process 1: " << count1_dynamic << " times\n";
  std::cout << "  Process 2: " << count2_dynamic << " times\n";
  std::cout << "  → Process 2 gets fair share!\n\n";
  
  std::cout << "Starvation prevented through:\n";
  std::cout << "  • Wait boost increases priority of waiting processes\n";
  std::cout << "  • Frequency penalty reduces priority of busy processes\n";
  std::cout << "  • Minimum weight ensures all processes remain viable\n\n";
}

void DemonstrateFailureHandling() 
{
  std::cout << "Failure Handling & Recovery Demo\n\n";
  
  Scheduler<size_t> scheduler;
  scheduler.AddProcess(1, 10.0);  // Reliable process
  scheduler.AddProcess(2, 10.0);  // Unreliable process
  scheduler.AddProcess(3, 10.0);  // Another process
  
  // Configure failure handling
  scheduler.EnableFailureHandling(true);
  scheduler.SetMaxConsecutiveFailures(3);
  scheduler.SetInitialBackoffCycles(2);
  scheduler.SetBackoffMultiplier(2.0);
  scheduler.SetRecoverySuccessThreshold(2);
  
  std::cout << "Configuration:\n";
  std::cout << "  Max Consecutive Failures: 3\n";
  std::cout << "  Initial Backoff: 2 cycles\n";
  std::cout << "  Backoff Multiplier: 2.0x\n";
  std::cout << "  Recovery Threshold: 2 successes\n\n";
  
  std::cout << "Simulating 15 scheduling cycles with failures...\n\n";
  
  // Simulate: Process 2 fails sometimes
  for (int cycle = 1; cycle <= 15; ++cycle) 
  {
    auto next_result = scheduler.GetNext();
    if (!next_result) 
    {
      std::cout << "Cycle " << std::setw(2) << cycle << ": No schedulable processes!\n";
      continue;
    }
    
    size_t next = *next_result;
    std::cout << "Cycle " << std::setw(2) << cycle << ": Process " << next << " scheduled";
    
    // Simulate failures for process 2 (cycles 2, 4, 10)
    if (next == 2 && (cycle == 2 || cycle == 4 || cycle == 10)) 
    {
      scheduler.MarkProcessFailed(2);
      std::cout << " → FAILED (failures=" << *scheduler.GetFailureCount(2) 
                << ", backoff=" << *scheduler.GetBackoffCycles(2) << ")";
    } 
    else 
    {
      scheduler.MarkProcessSuccess(next);
      std::cout << " → success";
      if (next == 2 && *scheduler.GetSuccessCount(2) > 0) 
      {
        std::cout << " (recovery=" << *scheduler.GetSuccessCount(2) << ")";
      }
    }
    
    std::cout << "\n";
    
    // Show state periodically
    if (cycle % 5 == 0) 
    {
      bool s1 = *scheduler.IsProcessSchedulable(1);
      bool s2 = *scheduler.IsProcessSchedulable(2);
      bool e2 = *scheduler.IsProcessEnabled(2);
      bool s3 = *scheduler.IsProcessSchedulable(3);
      
      std::cout << "  Status: P1=" << (s1 ? "ready" : "wait")
                << ", P2=" << (s2 ? "ready" : 
                             e2 ? 
                             ("backoff[" + std::to_string(*scheduler.GetCyclesUntilRetry(2)) + "]") : 
                             "disabled")
                << ", P3=" << (s3 ? "ready" : "wait") << "\n\n";
    }
  }
  
  std::cout << "\nKey observations:\n";
  std::cout << "  • Failed processes enter exponential backoff\n";
  std::cout << "  • Backoff increases: 2 → 4 → 8 cycles\n";
  std::cout << "  • Other processes continue normally during backoff\n";
  std::cout << "  • Process recovers after " << scheduler.GetRecoverySuccessThreshold() << " consecutive successes\n";
  std::cout << "  • Total failures tracked: P2=" << *scheduler.GetTotalFailures(2) << "\n\n";
}

void DemonstrateAutoDisable() 
{
  std::cout << " Auto-Disable After Max Failures Demo \n\n";
  
  Scheduler<size_t> scheduler;
  scheduler.AddProcess(1, 10.0);  // Good process
  scheduler.AddProcess(2, 10.0);  // Bad process
  
  scheduler.EnableFailureHandling(true);
  scheduler.SetMaxConsecutiveFailures(3);
  scheduler.SetInitialBackoffCycles(1);
  
  std::cout << "Setup: Max failures = 3\n";
  std::cout << "Process 2 will fail repeatedly...\n\n";
  
  for (int cycle = 1; cycle <= 10; ++cycle) 
  {
    if (scheduler.GetSchedulableProcessCount() == 0) 
    {
      std::cout << "Cycle " << cycle << ": No schedulable processes!\n";
      break;
    }
    
    auto next_result = scheduler.GetNext();
    if (!next_result) 
    {
      std::cout << "Cycle " << cycle << ": Scheduling error!\n";
      break;
    }
    
    size_t next = *next_result;
    std::cout << "Cycle " << std::setw(2) << cycle << ": Process " << next;
    
    // Process 2 always fails
    if (next == 2) 
    {
      scheduler.MarkProcessFailed(2);
      std::cout << " FAILED (count=" << *scheduler.GetFailureCount(2) << ")";
      
      if (!*scheduler.IsProcessEnabled(2)) 
      {
        std::cout << " → AUTO-DISABLED!";
      }
    } 
    else 
    {
      scheduler.MarkProcessSuccess(next);
      std::cout << " success";
    }
    std::cout << "\n";
  }
  
  std::cout << "\nResult:\n";
  std::cout << "  Process 1: " << (*scheduler.IsProcessEnabled(1) ? "enabled" : "disabled") << "\n";
  std::cout << "  Process 2: " << (*scheduler.IsProcessEnabled(2) ? "enabled" : "disabled") 
            << " (failures=" << *scheduler.GetFailureCount(2) << ")\n";
  std::cout << "  Schedulable: " << scheduler.GetSchedulableProcessCount() << " processes\n\n";
  
  std::cout << "Manual re-enable:\n";
  scheduler.EnableProcess(2);
  std::cout << "  Process 2 re-enabled, failures cleared\n";
  std::cout << "  Process 2: " << (*scheduler.IsProcessEnabled(2) ? "enabled" : "disabled")
            << " (failures=" << *scheduler.GetFailureCount(2) << ")\n\n";
}

int main() {
  std::cout << "\n";
 
  std::cout << "       Scheduler Class - Usage Examples            \n";
  
  std::cout << "\n";
  
  DemonstrateDeterministicScheduling();
  DemonstrateProbabilisticScheduling();
  DemonstrateStringIDs();
  DemonstrateDynamicProcesses();
  DemonstrateDynamicWeightAdjustment();
  DemonstrateStarvationPrevention();
  DemonstrateFailureHandling();
  DemonstrateAutoDisable();
  
  std::cout << "All examples completed successfully!\n\n";
  

  
  return 0;
}