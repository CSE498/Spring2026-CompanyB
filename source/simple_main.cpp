/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A simplistic main file to demonstrate a system.
 * @note Status: PROPOSAL
 **/

// Include the modules that we will be using.
#include "Agents/PacingAgent.hpp"
#include "Interfaces/TrashInterface.hpp"
#include "Worlds/MazeWorld.hpp"

using namespace cse498;

int main()
{
  MazeWorld world;
  world.AddAgent<PacingAgent>("Pacer 1").SetLocation(WorldPosition{3,1});
  world.AddAgent<PacingAgent>("Pacer 2").SetLocation(WorldPosition{6,1});
  world.AddAgent<PacingAgent>("Guard 1").SetHorizontal().SetLocation(WorldPosition{7,7});
  world.AddAgent<PacingAgent>("Guard 2").SetHorizontal().ToggleDirection().SetLocation(WorldPosition{8,8});
  world.AddAgent<TrashInterface>("Interface").SetSymbol('@').SetLocation(WorldPosition{1,1});

  world.Run();
}
