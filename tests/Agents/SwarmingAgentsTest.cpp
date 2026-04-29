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
      .position = pos,
      .health = state,
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

// Traffic agent with destination, ordered movement returns

TEST_CASE("Traffic agent heading right emits Right as first movement option",
          "[SwarmingAgent]") {
  WorldPosition pos{5, 5};
  WorldPosition dest{8, 5};
  auto data = MakeTrafficData(pos, dest);
  SwarmingAgent<TrafficData> agent(data, 0);

  auto turn = agent.GetTurn();
  MovementStep move = NextMovement(turn);

  REQUIRE(move.loc == pos.Right());
}

TEST_CASE("Traffic agent heading left emits Left as first movement option",
          "[SwarmingAgent]") {
  WorldPosition pos{5, 5};
  WorldPosition dest{2, 5};
  auto data = MakeTrafficData(pos, dest);
  SwarmingAgent<TrafficData> agent(data, 0);

  auto turn = agent.GetTurn();
  MovementStep move = NextMovement(turn);

  REQUIRE(move.loc == pos.Left());
}

TEST_CASE("Traffic agent heading up emits Up as first movement option",
          "[SwarmingAgent]") {
  WorldPosition pos{5, 5};
  WorldPosition dest{5, 2};
  auto data = MakeTrafficData(pos, dest);
  SwarmingAgent<TrafficData> agent(data, 0);

  auto turn = agent.GetTurn();
  MovementStep move = NextMovement(turn);

  REQUIRE(move.loc == pos.Up());
}

TEST_CASE("Traffic agent heading down emits Down as first movement option",
          "[SwarmingAgent]") {
  WorldPosition pos{5, 5};
  WorldPosition dest{5, 8};
  auto data = MakeTrafficData(pos, dest);
  SwarmingAgent<TrafficData> agent(data, 0);

  auto turn = agent.GetTurn();
  MovementStep move = NextMovement(turn);

  REQUIRE(move.loc == pos.Down());
}

// Traffic agent with destination, ordered movement returns

TEST_CASE("Traffic agent with destination emits four movement options",
          "[SwarmingAgent]") {
  WorldPosition pos{5, 5};
  WorldPosition dest{8, 5};
  auto data = MakeTrafficData(pos, dest);
  SwarmingAgent<TrafficData> agent(data, 0);

  auto turn = agent.GetTurn();

  MovementStep first = NextMovement(turn);
  MovementStep second = NextMovement(turn);
  MovementStep third = NextMovement(turn);
  MovementStep fourth = NextMovement(turn);

  REQUIRE(IsCardinalNeighbor(pos, first.loc));
  REQUIRE(IsCardinalNeighbor(pos, second.loc));
  REQUIRE(IsCardinalNeighbor(pos, third.loc));
  REQUIRE(IsCardinalNeighbor(pos, fourth.loc));

  std::set<std::pair<double, double>> unique_moves;
  unique_moves.insert({first.loc.X(), first.loc.Y()});
  unique_moves.insert({second.loc.X(), second.loc.Y()});
  unique_moves.insert({third.loc.X(), third.loc.Y()});
  unique_moves.insert({fourth.loc.X(), fourth.loc.Y()});

  REQUIRE(unique_moves.size() == 4);

  auto extra = turn.get_next();
  REQUIRE_FALSE(extra.has_value());
  REQUIRE(extra.error().kind == StepErr::Kind::STEPS_EXHAUSTED);
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
