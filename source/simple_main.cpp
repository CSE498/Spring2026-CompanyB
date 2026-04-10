/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A simplistic main file to demonstrate a system.
 * @note Status: PROPOSAL
 **/

// Include the modules that we will be using.
#include "Agents/SwarmingAgent.hpp"
#include "Worlds/MazeWorld.hpp"
#include "core/AgentData.hpp"

using namespace cse498;


// // Still needs more work, gotta figure out how we want SwarmingAgents to act in
// // the infectiious world, but this is a start
// struct InfectionData {
//   double infection_probability;
//   cse498::HealthState infection_state;
//   std::optional<cse498::WorldPosition> destination;
//   cse498::WorldPosition position;
//   cse498::Direction direction;
// };


int main() {
    TrafficData test_data{
        cse498::WorldPosition{16, 5},  // destination (open floor, reachable)
        cse498::WorldPosition{3, 1},   // position    (open floor)
        false,
        true
    };
    
  MazeWorld world;
 
//   world.AddAgent<PacingAgent>("Pacer 1").SetLocation(WorldPosition{3, 1});
//   world.AddAgent<PacingAgent>("Pacer 2").SetLocation(WorldPosition{6, 1});
//   world.AddAgent<PacingAgent>("Guard 1").SetHorizontal().SetLocation(
//       WorldPosition{7, 7});
//   world.AddAgent<PacingAgent>("Guard 2")
//       .SetHorizontal()
//       .ToggleDirection()
//       .SetLocation(WorldPosition{8, 8});
//   world.AddAgent<TrashInterface>("Interface")
//       .SetSymbol('@')
//       .SetLocation(WorldPosition{1, 1});



world.AddAgent<SwarmingAgent<TrafficData>>(test_data);
world.Run();
// clang-format -i -style=Google simple_main.cpp

// winget install -e --id LLVM.LLVM
}
