#include "../source/Agents/StepDrivingAgent.hpp"
#include "../source/Worlds/StepTrafficWorld.hpp"
using namespace cse498;
int main() {
  StepTrafficWorld world("DemoWorld.grid");
  TrafficData test_data{
      WorldPosition{0, 0}, WorldPosition{1, 4}, Direction::East, true, '>',
      "\033[91m"};
  world.AddAgent<StepDrivingAgent>(test_data);
  world.RunWithDisplay();
}