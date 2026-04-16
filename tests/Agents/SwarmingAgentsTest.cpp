/**
 * @file SwarmingAgentsTest.cpp
 * @brief Test suite for SwarmingAgent<TrafficData> and
 * SwarmingAgent<DiseaseData>
 */

// Used ClaudeCode and Codex to assist in editing our test suite to be up to
// date.

#include <catch2/catch_test_macros.hpp>
#include <optional>
#include <set>
#include <variant>

#include "../../source/Agents/SwarmingAgent.hpp"
#include "../../source/core/AgentData.hpp"
#include "../../source/core/Step.hpp"

using namespace cse498;
using namespace cse498::steps;

// Helpers

static TrafficData MakeTrafficData(WorldPosition pos,
                                   std::optional<WorldPosition> dest,
                                   bool active = true) {
  return TrafficData{
      .destination = dest,
      .position = pos,
      .direction = Direction::North,
      .is_active = active,
  };
}

static DiseaseData MakeDiseaseData(WorldPosition pos, HealthState state) {
  return DiseaseData{
      .infection_probability = 0.0,
      .infection_state = state,
      .destination = std::nullopt,
      .position = pos,
  };
}

static bool IsCardinalNeighbor(WorldPosition from, WorldPosition to) {
  return to == from.Up() || to == from.Down() || to == from.Left() ||
         to == from.Right();
}

/// Extract the next step and return it, requiring success.
static Step NextStep(StepContainer& c) {
  auto result = c.get_next();
  REQUIRE(result.has_value());
  return result.value();
}

/// Extract a MovementStep from the container.
static MovementStep NextMovement(StepContainer& c) {
  Step s = NextStep(c);
  REQUIRE(std::holds_alternative<MovementStep>(s));
  return std::get<MovementStep>(s);
}

/// Extract an InfoStep from the container.
static InfoStep NextInfo(StepContainer& c) {
  Step s = NextStep(c);
  REQUIRE(std::holds_alternative<InfoStep>(s));
  return std::get<InfoStep>(s);
}

// Construction and state

TEST_CASE("TrafficAgent construction and GetId", "[SwarmingAgent]") {
  auto data = MakeTrafficData({3, 4}, std::nullopt);
  SwarmingAgent<TrafficData> agent(data, 42);

  REQUIRE(agent.GetId() == 42);
}

TEST_CASE("TrafficAgent GetState returns initial state", "[SwarmingAgent]") {
  WorldPosition pos{7, 8};
  auto data = MakeTrafficData(pos, std::nullopt, false);
  SwarmingAgent<TrafficData> agent(data, 0);

  auto state = agent.GetState();
  REQUIRE(state.position == pos);
  REQUIRE(state.is_active == false);
  REQUIRE_FALSE(state.destination.has_value());
}

TEST_CASE("SetGoal updates destination", "[SwarmingAgent]") {
  auto data = MakeTrafficData({1, 1}, std::nullopt);
  SwarmingAgent<TrafficData> agent(data, 0);

  WorldPosition goal{9, 9};
  agent.SetGoal(goal);

  auto state = agent.GetState();
  REQUIRE(state.destination.has_value());
  REQUIRE(state.destination.value() == goal);
}

TEST_CASE("SetActive updates is_active", "[SwarmingAgent]") {
  auto data = MakeTrafficData({1, 1}, std::nullopt, true);
  SwarmingAgent<TrafficData> agent(data, 0);

  agent.SetActive(false);
  REQUIRE(agent.GetState().is_active == false);

  agent.SetActive(true);
  REQUIRE(agent.GetState().is_active == true);
}

// Traffic agent inactive behavior

TEST_CASE("Inactive traffic agent returns empty StepContainer",
          "[SwarmingAgent]") {
  auto data = MakeTrafficData({5, 5}, WorldPosition{8, 8}, false);
  SwarmingAgent<TrafficData> agent(data, 0);

  auto turn = agent.GetTurn();
  REQUIRE(turn.empty());
}

// Traffic agent with no destination — random movement

TEST_CASE(
    "Active traffic agent with no destination moves to a cardinal neighbor",
    "[SwarmingAgent]") {
  WorldPosition pos{5, 5};
  auto data = MakeTrafficData(pos, std::nullopt);
  SwarmingAgent<TrafficData> agent(data, 0);

  for (int i = 0; i < 20; ++i) {
    auto turn = agent.GetTurn();
    REQUIRE_FALSE(turn.empty());

    MovementStep ms = NextMovement(turn);
    REQUIRE(IsCardinalNeighbor(pos, ms.loc));

    // No additional steps
    auto extra = turn.get_next();
    REQUIRE_FALSE(extra.has_value());
    REQUIRE(extra.error().kind == StepErr::Kind::STEPS_EXHAUSTED);
  }
}

// Traffic agent already at destination

TEST_CASE("Traffic agent at destination returns empty container",
          "[SwarmingAgent]") {
  WorldPosition pos{5, 5};
  auto data = MakeTrafficData(pos, pos);
  SwarmingAgent<TrafficData> agent(data, 0);

  auto turn = agent.GetTurn();
  REQUIRE(turn.empty());
}

// Traffic agent with destination — InfoStep emission

TEST_CASE("Traffic agent heading right emits InfoStep for Right neighbor",
          "[SwarmingAgent]") {
  WorldPosition pos{5, 5};
  WorldPosition dest{8, 5};
  auto data = MakeTrafficData(pos, dest);
  SwarmingAgent<TrafficData> agent(data, 0);

  auto turn = agent.GetTurn();
  InfoStep info = NextInfo(turn);

  REQUIRE(info.aspect == InfoStep::Aspect::LOC_AVAIL);
  REQUIRE(info.target == pos.Right());
}

TEST_CASE("Traffic agent heading left emits InfoStep for Left neighbor",
          "[SwarmingAgent]") {
  WorldPosition pos{5, 5};
  WorldPosition dest{2, 5};
  auto data = MakeTrafficData(pos, dest);
  SwarmingAgent<TrafficData> agent(data, 0);

  auto turn = agent.GetTurn();
  InfoStep info = NextInfo(turn);

  REQUIRE(info.aspect == InfoStep::Aspect::LOC_AVAIL);
  REQUIRE(info.target == pos.Left());
}

TEST_CASE("Traffic agent heading up emits InfoStep for Up neighbor",
          "[SwarmingAgent]") {
  WorldPosition pos{5, 5};
  WorldPosition dest{5, 2};  // Up means decreasing Y
  auto data = MakeTrafficData(pos, dest);
  SwarmingAgent<TrafficData> agent(data, 0);

  auto turn = agent.GetTurn();
  InfoStep info = NextInfo(turn);

  REQUIRE(info.aspect == InfoStep::Aspect::LOC_AVAIL);
  REQUIRE(info.target == pos.Up());
}

TEST_CASE("Traffic agent heading down emits InfoStep for Down neighbor",
          "[SwarmingAgent]") {
  WorldPosition pos{5, 5};
  WorldPosition dest{5, 8};  // Down means increasing Y
  auto data = MakeTrafficData(pos, dest);
  SwarmingAgent<TrafficData> agent(data, 0);

  auto turn = agent.GetTurn();
  InfoStep info = NextInfo(turn);

  REQUIRE(info.aspect == InfoStep::Aspect::LOC_AVAIL);
  REQUIRE(info.target == pos.Down());
}

// Conditional movement after InfoStep

TEST_CASE("inform(true) yields MovementStep to primary neighbor",
          "[SwarmingAgent]") {
  WorldPosition pos{5, 5};
  WorldPosition dest{8, 5};
  auto data = MakeTrafficData(pos, dest);
  SwarmingAgent<TrafficData> agent(data, 0);

  auto turn = agent.GetTurn();

  // Consume InfoStep
  NextInfo(turn);

  // Inform the container that the location is available
  turn.inform(true);

  // Next should be a ConditionalStep handled internally, resolving to primary
  MovementStep ms = NextMovement(turn);
  REQUIRE(ms.loc == pos.Right());
}

TEST_CASE("inform(false) yields MovementStep to backup neighbor",
          "[SwarmingAgent]") {
  WorldPosition pos{5, 5};
  WorldPosition dest{8, 5};
  auto data = MakeTrafficData(pos, dest);
  SwarmingAgent<TrafficData> agent(data, 0);

  auto turn = agent.GetTurn();

  // Consume InfoStep
  NextInfo(turn);

  // Inform the container that the location is NOT available
  turn.inform(false);

  // Next should be the backup neighbor — best_neighbor with primary excluded.
  // dest is (8,5), pos is (5,5). Primary is Right (6,5).
  // Excluding Right, neighbors considered in array order: Up(5,4), Down(5,6),
  // Left(4,5). Manhattan distances to (8,5): Up=|8-5|+|5-4|=4,
  // Down=|8-5|+|5-6|=4, Left=|8-4|+|5-5|=4 All tied at 4 — first in array order
  // wins: Up(5,4). But recent_positions may affect this; best_neighbor prefers
  // "fresh" cells. On a fresh agent with no history other than pos and primary
  // recorded, pos=(5,5) and primary=(6,5) are in recent. Up/Down/Left are all
  // fresh and tied. First fresh in array order (Up) wins.
  MovementStep ms = NextMovement(turn);
  REQUIRE(ms.loc == pos.Up());
}

TEST_CASE("ConditionalStep without inform returns NOT_INFORMED error",
          "[SwarmingAgent]") {
  WorldPosition pos{5, 5};
  WorldPosition dest{8, 5};
  auto data = MakeTrafficData(pos, dest);
  SwarmingAgent<TrafficData> agent(data, 0);

  auto turn = agent.GetTurn();

  // Consume InfoStep
  NextInfo(turn);

  // Do NOT call inform — the next get_next should error
  auto result = turn.get_next();
  // The ConditionalStep is next; without inform it should fail
  // Actually get_next returns the ConditionalStep node which checks world_info
  REQUIRE_FALSE(result.has_value());
  REQUIRE(result.error().kind == StepErr::Kind::NOT_INFORMED);
}

// Disease agent behavior

TEST_CASE("DiseaseAgent SUSCEPTIBLE moves to cardinal neighbor",
          "[SwarmingAgent]") {
  WorldPosition pos{5, 5};
  auto data = MakeDiseaseData(pos, HealthState::SUSCEPTIBLE);
  SwarmingAgent<DiseaseData> agent(data, 0);

  for (int i = 0; i < 10; ++i) {
    auto turn = agent.GetTurn();
    REQUIRE_FALSE(turn.empty());
    MovementStep ms = NextMovement(turn);
    REQUIRE(IsCardinalNeighbor(pos, ms.loc));
  }
}

TEST_CASE("DiseaseAgent INFECTED moves to cardinal neighbor",
          "[SwarmingAgent]") {
  WorldPosition pos{5, 5};
  auto data = MakeDiseaseData(pos, HealthState::INFECTED);
  SwarmingAgent<DiseaseData> agent(data, 0);

  for (int i = 0; i < 10; ++i) {
    auto turn = agent.GetTurn();
    REQUIRE_FALSE(turn.empty());
    MovementStep ms = NextMovement(turn);
    REQUIRE(IsCardinalNeighbor(pos, ms.loc));
  }
}

TEST_CASE("DiseaseAgent RECOVERED moves to cardinal neighbor",
          "[SwarmingAgent]") {
  WorldPosition pos{5, 5};
  auto data = MakeDiseaseData(pos, HealthState::RECOVERED);
  SwarmingAgent<DiseaseData> agent(data, 0);

  for (int i = 0; i < 10; ++i) {
    auto turn = agent.GetTurn();
    REQUIRE_FALSE(turn.empty());
    MovementStep ms = NextMovement(turn);
    REQUIRE(IsCardinalNeighbor(pos, ms.loc));
  }
}

// Anti-looping / recent-history behavior

TEST_CASE(
    "Traffic agent avoids recently visited positions when fresh neighbors "
    "exist",
    "[SwarmingAgent]") {
  // Agent with no destination wanders randomly but avoids recent positions.
  // Over many turns from a fixed position, it should visit more than one
  // distinct neighbor (demonstrating it doesn't just repeat one cell).
  WorldPosition pos{5, 5};
  auto data = MakeTrafficData(pos, std::nullopt);
  SwarmingAgent<TrafficData> agent(data, 0);

  std::set<std::pair<double, double>> visited;
  for (int i = 0; i < 30; ++i) {
    auto turn = agent.GetTurn();
    MovementStep ms = NextMovement(turn);
    visited.insert({ms.loc.X(), ms.loc.Y()});
  }

  // With anti-looping, the agent should visit more than 1 distinct neighbor
  REQUIRE(visited.size() > 1);
}
