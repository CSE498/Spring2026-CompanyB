#include "../source/Agents/SwarmingAgent.hpp"
#include "../source/Worlds/StepTrafficWorld.hpp"
using namespace cse498;
int main() {
  // NOTE: this file path assumes that you're compiling with the build system
  // and running the result from the top-level directory, i.e. "make build"
  // (with the target set to this file), then "./build/native/app".
  StepTrafficWorld<SwarmingAgent<TrafficData>> world(
      "assets/grids/DemoWorld.grid");
  // StepTrafficWorld<SwarmingAgent<TrafficData>> world("TinyWorld.grid");
  world.RunWithDisplay();
}