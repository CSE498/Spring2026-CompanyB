/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A demo of group 19's TrafficWorld class.
 * @note Status: PROPOSAL
 **/

#include "../source/Interfaces/AutoInterface.hpp"
#include "../source/Worlds/TrafficWorld.hpp"

using namespace cse498;

int main() {
  TrafficWorld world("demo/DemoWorld.grid");
  world.AddAgent<AutoInterface>("Viewer");

  // Step the world a few times and print summary DataLog stats.
  for (int tick = 0; tick < 5; ++tick) {
    world.RunAgents();
    world.UpdateWorld();

    const auto& log = world.GetTrafficDataLog().GetAggregationData();
    if (log.contains("active_count") && !log.at("active_count").empty()) {
      const auto& stats = log.at("active_count").back();
      std::cout << "Tick " << tick << " active_count=" << stats.sum
                << " mean=" << stats.mean << " count=" << stats.count
                << "\n";
    }
  }

  return 0;
}
