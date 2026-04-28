/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A demo of group 19's TrafficWorld class.
 * @note Status: PROPOSAL
 **/

#include "../source/Interfaces/AutoInterface.hpp"
#include "../source/Worlds/TrafficWorld.hpp"

using namespace cse498;

int main() {
  // At this point, there isn't much you can do with TrafficWorld besides make
  // it. You can add agents in the same way as any world, but it spawns agents
  // on its own, so you don't need to. Also note that TrafficWorld is only fully
  // compatible with DrivingAgent right now. This is not something we want to
  // have long-term, of course, but until the agent-world API is fully worked
  // out, we figure we'll just use a single simple agent to show things off.
  TrafficWorld world("demo/DemoWorld.grid");
  // AutoInterface is a little class we made to replace TrashInterface for the
  // specific purpose of
  world.AddAgent<AutoInterface>("Viewer");

  world.Run();
}
