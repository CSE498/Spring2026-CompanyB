#include "../source/Worlds/StepTrafficWorld.hpp"
#include "../source/Agents/SwarmingAgent.hpp"
using namespace cse498;
int main() {
  StepTrafficWorld<SwarmingAgent<TrafficData>> world("DemoWorld.grid");
  world.RunWithDisplay();
}