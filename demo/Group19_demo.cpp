/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A simplistic main file to demonstrate a system.
 * @note Status: PROPOSAL
 **/

// Include the modules that we will be using.
#include "../source/Agents/DrivingAgent.hpp"
#include "../source/Interfaces/AutoInterface.hpp"
#include "../source/Worlds/TrafficWorld.hpp"

using namespace cse498;

int main() {
  TrafficWorld world;
  // world.AddAgent<DrivingAgent>("Car 1").SetLocation(WorldPosition{5, 6});
  // world.AddAgent<DrivingAgent>("Car
  // 2").SetDirection(Direction::South).SetLocation(WorldPosition{8, 4});
  world.AddAgent<AutoInterface>("Viewer");

  world.Run();
}
