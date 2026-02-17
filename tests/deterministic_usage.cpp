/**
 * @file deterministic_usage.cpp
 * @brief Example usage of the Determisitic property of the Scheduler class
 * @author Joshua Twumasi
 * 
 * This file will shouw how to use the Scheduler class for determinsitic prediction
 * 
 * Compile with g++ -std=c++23 -Wall -Wextra -Wcast-align -Wnon-virtual-dtor -Woverloaded-virtual -pedantic deterministic_usage.cpp -o deterministic
 */


#include "Scheduler.hpp"
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
    // if (!next_result) 
    // {
    //   std::cerr << "Error: " << to_string(next_result.error()) << "\n";
    //   return;
    // }
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

int main()
{
    DemonstrateDeterministicScheduling();
}