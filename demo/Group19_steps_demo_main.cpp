#include "../source/Agents/SwarmingAgent.hpp"
#include "../source/Worlds/StepTrafficWorld.hpp"
using namespace cse498;
int main() {
  StepTrafficWorld<SwarmingAgent<TrafficData>> world("assets/DemoWorld.grid");
  //StepTrafficWorld<SwarmingAgent<TrafficData>> world("TinyWorld.grid");
  world.RunWithDisplay();
}