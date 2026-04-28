#include <catch2/catch_test_macros.hpp>

#include "../source/Agents/ScriptedAgent.hpp"
#include "../source/Worlds/InfectiousWorld.hpp"

using namespace cse498;

static DiseaseData at(size_t x, size_t y) {
  return DiseaseData{WorldPosition{x, y}};
}

// ============================================================================
// Basic counts
// ============================================================================

TEST_CASE("InfectiousWorld: empty world counts", "[InfectiousWorld]") {
  InfectiousWorld world(8, 6);
  CHECK(world.GetNumAgents() == 0);
  CHECK(world.GetSusceptibleCount() == 0);
  CHECK(world.GetInfectedCount() == 0);
  CHECK(world.GetRecoveredCount() == 0);
}

TEST_CASE("InfectiousWorld: new agent is susceptible", "[InfectiousWorld]") {
  InfectiousWorld world(8, 6);
  world.AddAgent<ScriptedAgent<DiseaseData>>(at(2, 2));

  CHECK(world.GetNumAgents() == 1);
  CHECK(world.GetAgentHealth(0) == HealthState::SUSCEPTIBLE);
  CHECK(world.GetSusceptibleCount() == 1);
  CHECK(world.GetInfectedCount() == 0);
  CHECK(world.GetRecoveredCount() == 0);
}

// ============================================================================
// Manual infection
// ============================================================================

TEST_CASE("InfectiousWorld: InfectAgent updates state", "[InfectiousWorld]") {
  InfectiousWorld world(8, 6);
  world.AddAgent<ScriptedAgent<DiseaseData>>(at(2, 2));

  world.InfectAgent(0);
  CHECK(world.GetAgentHealth(0) == HealthState::INFECTED);
  CHECK(world.GetInfectedCount() == 1);
  CHECK(world.GetSusceptibleCount() == 0);
}



// ============================================================================
// Spread
// ============================================================================

TEST_CASE("InfectiousWorld: spread applies after timers for this tick",
          "[InfectiousWorld]") {
  // Agents never auto-recover here (no fallback, no quarantine zone), so
  // patient zero stays INFECTED long enough for spread to work.
  InfectiousWorld world(6, 4);
  world.SetTransmissionRate(1.0);
  world.SetInfectionRadius(5.0);

  world.AddAgent<ScriptedAgent<DiseaseData>>(at(2, 2));
  world.AddAgent<ScriptedAgent<DiseaseData>>(at(3, 2));
  world.InfectAgent(0);

  world.UpdateWorld();

  CHECK(world.GetAgentHealth(1) == HealthState::INFECTED);
  CHECK(world.GetInfectedCount() == 2);
}

TEST_CASE("InfectiousWorld: newly spread infection clock starts next tick",
          "[InfectiousWorld]") {
  InfectiousWorld world(6, 4);
  world.SetTransmissionRate(1.0);
  world.SetInfectionRadius(5.0);
  world.SetFallbackRecoveryTicks(20);  // keep patient zero infected long enough

  world.AddAgent<ScriptedAgent<DiseaseData>>(at(2, 2));
  world.AddAgent<ScriptedAgent<DiseaseData>>(at(3, 2));
  world.InfectAgent(0);

  world.UpdateWorld();
  REQUIRE(world.GetAgentHealth(1) == HealthState::INFECTED);

  // Lower fallback to 1: agent 1's ticks_in_state is 0 right now, so it
  // needs one more UpdateWorld tick before it reaches the threshold.
  world.SetFallbackRecoveryTicks(1);
  world.UpdateWorld();
  CHECK(world.GetAgentHealth(1) == HealthState::RECOVERED);
}

// Quarantine-zone (Olin) recovery

TEST_CASE("InfectiousWorld: quarantine zone heals infected agents",
          "[InfectiousWorld]") {
  InfectiousWorld world(10, 8);
  world.SetTreatmentDuration(3);

  // Register a quarantine zone and place an infected agent inside it.
  world.AddQuarantineZone(Box::FromCorners(Point(3, 3), Point(7, 7)));
  world.AddAgent<ScriptedAgent<DiseaseData>>(at(4, 4));
  world.InfectAgent(0);
  REQUIRE(world.GetAgentHealth(0) == HealthState::INFECTED);

  // One tick before threshold — still infected.
  for (int i = 0; i < 2; ++i) world.UpdateWorld();
  CHECK(world.GetAgentHealth(0) == HealthState::INFECTED);

  // Exactly at threshold — now recovered.
  world.UpdateWorld();
  CHECK(world.GetAgentHealth(0) == HealthState::RECOVERED);
}

TEST_CASE(
    "InfectiousWorld: agent outside quarantine does not recover via treatment",
    "[InfectiousWorld]") {
  InfectiousWorld world(10, 8);
  world.SetTreatmentDuration(2);

  // Quarantine zone does not contain the agent's starting position.
  world.AddQuarantineZone(Box::FromCorners(Point(6, 6), Point(9, 9)));
  world.AddAgent<ScriptedAgent<DiseaseData>>(at(1, 1));
  world.InfectAgent(0);

  // Run longer than treatment_duration — still infected (never entered zone).
  for (int i = 0; i < 5; ++i) world.UpdateWorld();
  CHECK(world.GetAgentHealth(0) == HealthState::INFECTED);
}

// Fallback recovery (swarming agents that miss Olin)

TEST_CASE("InfectiousWorld: fallback recovery after N infection ticks",
          "[InfectiousWorld]") {
  InfectiousWorld world(8, 6);
  world.SetFallbackRecoveryTicks(5);

  world.AddAgent<ScriptedAgent<DiseaseData>>(at(2, 2));
  world.InfectAgent(0);

  // One tick before threshold — still infected.
  for (int i = 0; i < 4; ++i) world.UpdateWorld();
  CHECK(world.GetAgentHealth(0) == HealthState::INFECTED);

  // Exactly at threshold — now recovered.
  world.UpdateWorld();
  CHECK(world.GetAgentHealth(0) == HealthState::RECOVERED);
}

TEST_CASE("InfectiousWorld: fallback disabled by default (ticks = 0)",
          "[InfectiousWorld]") {
  InfectiousWorld world(8, 6);
  // No fallback set, no quarantine zone — agent should stay infected forever.

  world.AddAgent<ScriptedAgent<DiseaseData>>(at(2, 2));
  world.InfectAgent(0);

  for (int i = 0; i < 50; ++i) world.UpdateWorld();
  CHECK(world.GetAgentHealth(0) == HealthState::INFECTED);
}

// Immunity after recovery

TEST_CASE("InfectiousWorld: immunity period after fallback recovery",
          "[InfectiousWorld]") {
  InfectiousWorld world(8, 6);
  world.SetFallbackRecoveryTicks(1);
  world.SetImmunityDuration(3);

  world.AddAgent<ScriptedAgent<DiseaseData>>(at(2, 2));
  world.InfectAgent(0);

  // Recovers after 1 tick.
  world.UpdateWorld();
  REQUIRE(world.GetAgentHealth(0) == HealthState::RECOVERED);

  // Still immune for 2 more ticks (3 total).
  for (int i = 0; i < 2; ++i) world.UpdateWorld();
  CHECK(world.GetAgentHealth(0) == HealthState::RECOVERED);

  // Immunity expires on tick 3.
  world.UpdateWorld();
  CHECK(world.GetAgentHealth(0) == HealthState::SUSCEPTIBLE);
}

TEST_CASE("InfectiousWorld: recovered agent can be re-infected after immunity",
          "[InfectiousWorld]") {
  // Tick ordering: UpdateHealthTimers runs before SpreadInfection.
  // fallback=2 means patient zero stays infected on tick 1 (ticks_in_state=1)
  // while spread reaches agent 1, then recovers on tick 2 (ticks_in_state=2).
  // immunity_duration=2: patient zero needs 2 more ticks before becoming
  // susceptible; on that same tick SpreadInfection immediately re-infects them
  // (timers and spread both happen within one UpdateWorld call), so
  // SUSCEPTIBLE is a transient state not observable between ticks.
  InfectiousWorld world(8, 6);
  world.SetTransmissionRate(1.0);
  world.SetInfectionRadius(5.0);
  world.SetFallbackRecoveryTicks(2);
  world.SetImmunityDuration(2);

  world.AddAgent<ScriptedAgent<DiseaseData>>(at(2, 2));  // patient zero
  world.AddAgent<ScriptedAgent<DiseaseData>>(at(3, 2));  // target
  world.InfectAgent(0);

  // Tick 1: patient zero ticks_in_state=1 (< 2, stays infected); spread infects
  // agent 1.
  world.UpdateWorld();
  REQUIRE(world.GetAgentHealth(1) == HealthState::INFECTED);

  // Tick 2: patient zero ticks_in_state=2 >= 2 — recovers.
  world.UpdateWorld();
  REQUIRE(world.GetAgentHealth(0) == HealthState::RECOVERED);

  // Lock agent 1 as infected for the rest of the test.
  world.SetFallbackRecoveryTicks(999);

  // Ticks 3-4: immunity window. On tick 3 ticks_in_state=1 (still immune).
  // On tick 4 ticks_in_state=2 >= immunity_duration — patient zero becomes
  // susceptible and SpreadInfection re-infects them in the same tick.
  for (int i = 0; i < 2; ++i) world.UpdateWorld();
  CHECK(world.GetAgentHealth(0) == HealthState::INFECTED);
}
