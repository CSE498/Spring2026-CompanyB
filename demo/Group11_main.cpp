/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A simplistic main file to demonstrate a system.
 * @note Status: PROPOSAL
 **/

// Include the modules that we will be using.
#include "Agents/ScriptedAgent.hpp"
#include "Worlds/Group11DummyData.hpp"
#include "Worlds/Group11TestWorld.hpp"

using namespace cse498;

int main() {
  Group11TestWorld world;
  world.AddAgent<ScriptedAgent<Group11DummyData>>(
      Group11DummyData(WorldPosition{3, 7}));

  world.Run();
}
