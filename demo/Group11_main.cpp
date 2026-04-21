/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A simplistic main file to demonstrate a system.
 * @note Status: PROPOSAL
 **/

// Include the modules that we will be using.
#include "Agents/ScriptedAgent.hpp"
#include "Worlds/ScriptingTestWorld.hpp"

using namespace cse498;

int main() {
  ScriptingTestWorld world;


  world.Run();
}