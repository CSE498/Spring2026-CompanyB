/**
 * @file probabilistic_usage.cpp
 * @brief Example usage of the Probabilistic property of the Scheduler class
 * @author Joshua Twumasi
 * 
 * This file will shouw how to use the Scheduler class for probabilistic scheduling
 * 
 * Compile with g++ -std=c++23 -Wall -Wextra -Wcast-align -Wnon-virtual-dtor -Woverloaded-virtual -pedantic probabilistic_usage.cpp -o probabilistic
 */

#include <iostream>
#include <iomanip>

#include "Scheduler.hpp"

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

int main()
{
    DemonstrateProbabilisticScheduling();
}