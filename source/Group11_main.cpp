/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A simplistic main file to demonstrate a system.
 * @note Status: PROPOSAL
 **/

// Include the modules that we will be using.
#include "Agents/ScriptedAgent.hpp"
// #include "Interfaces/TrashInterface.hpp"
#include "Interpreter/SymbolTable.hpp"
#include "Worlds/MazeWorld.hpp"
#include "core/WorldBase.hpp"

using namespace cse498;

int main() {
  SymbolTable s;
  MazeWorld world;
  world.AddAgent<ScriptedAgent>(DummyAgentData(WorldPosition{3, 7}));

  /**
   * Our company has two worlds, neither interact with the user
   * Therefore, i dont use this interface (since it derives from agent and isnt
   * suited to what we need) So i just modified the world.run method to instead
   * handle the display of the board and stuff
   */
  // world.AddAgent<TrashInterface>("Interface")
  //     .SetSymbol('@')
  //     .SetLocation(WorldPosition{1, 1});

  world.Run();
}
