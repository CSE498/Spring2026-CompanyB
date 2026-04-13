#include "../source/Agents/StepDrivingAgent.hpp"
#include "../source/Worlds/StepTrafficWorld.hpp"
using namespace cse498;
int main() {
  StepTrafficWorld world("DemoWorld.grid");
  world.RunWithDisplay();
}