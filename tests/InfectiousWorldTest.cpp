#include <catch2/catch_test_macros.hpp>

#include "../source/Agents/PacingAgent.hpp"
#include "../source/Worlds/InfectiousWorld.hpp"

using namespace cse498;

TEST_CASE("InfectiousWorld: empty world counts", "[InfectiousWorld]") {
  InfectiousWorld world(8, 6);
  CHECK(world.GetNumAgents() == 0);
  CHECK(world.GetSusceptibleCount() == 0);
  CHECK(world.GetInfectedCount() == 0);
  CHECK(world.GetRecoveredCount() == 0);
}

TEST_CASE("InfectiousWorld: new agent is susceptible", "[InfectiousWorld]") {
  InfectiousWorld world(8, 6);
  world.AddAgent<PacingAgent>("a0").SetLocation(WorldPosition{2, 2});

  CHECK(world.GetNumAgents() == 1);
  CHECK(world.GetAgentHealth(0) == InfectiousWorld::HealthState::SUSCEPTIBLE);
  CHECK(world.GetSusceptibleCount() == 1);
  CHECK(world.GetInfectedCount() == 0);
  CHECK(world.GetRecoveredCount() == 0);
}

TEST_CASE("InfectiousWorld: InfectAgent updates state", "[InfectiousWorld]") {
  InfectiousWorld world(8, 6);
  world.AddAgent<PacingAgent>("a0").SetLocation(WorldPosition{2, 2});

  world.InfectAgent(0);
  CHECK(world.GetAgentHealth(0) == InfectiousWorld::HealthState::INFECTED);
  CHECK(world.GetInfectedCount() == 1);
  CHECK(world.GetSusceptibleCount() == 0);
}

TEST_CASE("InfectiousWorld: InfectAgent invalid id throws", "[InfectiousWorld]") {
  InfectiousWorld world(8, 6);
  CHECK_THROWS_AS(world.InfectAgent(0), std::out_of_range);
}

TEST_CASE("InfectiousWorld: spread applies after timers for this tick",
          "[InfectiousWorld]") {
  InfectiousWorld world(6, 4);
  world.SetTransmissionRate(1.0);
  world.SetInfectionRadius(5.0);
  // Carrier must stay INFECTED through UpdateHealthTimers so SpreadInfection
  // still runs; with duration 1, patient zero would recover before spread.
  world.SetInfectionDuration(20);

  world.AddAgent<PacingAgent>("a0").SetLocation(WorldPosition{2, 2});
  world.AddAgent<PacingAgent>("a1").SetLocation(WorldPosition{3, 2});
  world.InfectAgent(0);

  world.UpdateWorld();

  CHECK(world.GetAgentHealth(1) == InfectiousWorld::HealthState::INFECTED);
  CHECK(world.GetInfectedCount() == 2);
}

TEST_CASE("InfectiousWorld: new infection clock starts next UpdateWorld",
          "[InfectiousWorld]") {
  InfectiousWorld world(6, 4);
  world.SetTransmissionRate(1.0);
  world.SetInfectionRadius(5.0);
  world.SetInfectionDuration(20);

  world.AddAgent<PacingAgent>("a0").SetLocation(WorldPosition{2, 2});
  world.AddAgent<PacingAgent>("a1").SetLocation(WorldPosition{3, 2});
  world.InfectAgent(0);

  world.UpdateWorld();
  REQUIRE(world.GetAgentHealth(1) == InfectiousWorld::HealthState::INFECTED);

  world.SetInfectionDuration(1);
  world.UpdateWorld();
  CHECK(world.GetAgentHealth(1) == InfectiousWorld::HealthState::RECOVERED);
}
