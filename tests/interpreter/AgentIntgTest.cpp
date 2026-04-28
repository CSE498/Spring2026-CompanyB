/**
 * @file AgentIntgTest.cpp
 * @brief Integration tests for Scripted Agents
 * @author Lalit Chitibomma
 */
#include <array>
#include <catch2/catch_test_macros.hpp>
#include <format>
#include <sstream>
#include <string>

#include "Agents/ScriptedAgent.hpp"
#include "Worlds/ScriptingTestWorld.hpp"
#include "catch2/matchers/catch_matchers.hpp"
#include "interpreter-tests.hpp"

using namespace cse498;
using namespace cse498::AST;

// Parses the given statements under a "world traffic;" header.
// Returns the parser (which holds m_Nodes and m_Syms) and the parse result.
static std::pair<
    Parser,
    std::expected<std::vector<std::unique_ptr<StmtAgentDef>>, InterpErr>>
parse(std::string const &stmts) {
  Parser p;
  std::istringstream ss(stmts);
  auto result = p.parse(ss);
  return {std::move(p), std::move(result)};
}

const char *AGENT_CAR = R"V0G0N(

world traffic;

let square_walker : car {
    init : {
        let have_moved : bool = false;
    };
    turn : {
        if (!have_moved) {
      move(right);
      have_moved = true;
    } else {
      if (__facing__ == up) move(right);
      else if (__facing__ == right) move(down);
      else if (__facing__ == down) move(left);
      else if (__facing__ == left) move(up);
    }
    };
};

)V0G0N";

const char *TARGET_FOLLOWER = R"V0G0N(

world traffic;

let target_follower : car {
    init : {
        __spawn__ = make_point(1, 1);
        __destination__ = make_point(3, 1);
    };
    turn : {
        let x : int = get_x(__position__);
        let y : int = get_y(__position__);
        let d_x : int = get_x(__destination__);
        let d_y : int = get_y(__destination__);

        if (x < d_x) move(right);
        else if (x > d_x) move(left);
        else if (y < d_y) move(down);
        else if (y > d_y) move(up);
    };
};

)V0G0N";

const char *INFECTION_MAGIC_VALUES = R"V0G0N(

world infection;

let state_reader : student {
    init : { };
    turn : {
        if (__infected__) move(right);
        else if (__susceptible__) move(down);
        else if (__recovered__) move(left);
    };
};

)V0G0N";

TEST_CASE("Basic Agent Test", "[traffic][intg]") {
  auto [p, result] = parse(std::string{AGENT_CAR});

  // Init and Turn Exist (like AgentDefParserTest)
  CAPTURE(result);
  REQUIRE(result.has_value());
  auto &defs = result.value();
  REQUIRE(defs.size() == 1);

  auto *root = dynamic_cast<StmtAgentDef *>(defs[0].get());
  REQUIRE(root);
  auto *init = dynamic_cast<StmtBlock *>(root->m_Init.get());
  REQUIRE(init);
  auto *turn = dynamic_cast<StmtBlock *>(root->m_Turn.get());
  REQUIRE(turn);

  TrafficData data = {};

  ScriptedAgent agent(data, 0);
  ScriptingTestWorld();

  root->m_Init.release();
  agent.SetInit(std::unique_ptr<StmtBlock>(init));

  root->m_Turn.release();
  agent.SetTurn(std::unique_ptr<StmtBlock>(turn));

  // Turn #1
  auto step_container = agent.GetTurn();
  REQUIRE_FALSE(step_container.empty());

  auto cur_step = step_container.get_next();
  REQUIRE(cur_step.has_value());
  REQUIRE(std::holds_alternative<MovementStep>(cur_step.value()));
  CHECK(std::get<MovementStep>(cur_step.value()).loc == WorldPosition{1, 0});
}

TEST_CASE("GetTurn before SetInit is a safe no-op", "[traffic][intg]") {
  TrafficData data = {};
  ScriptedAgent agent(data, 0);

  auto step_container = agent.GetTurn();

  CHECK(step_container.empty());
}

TEST_CASE("Traffic built-ins drive a target-following move",
          "[traffic][intg]") {
  auto [p, result] = parse(std::string{TARGET_FOLLOWER});

  CAPTURE(result);
  REQUIRE(result.has_value());
  auto &defs = result.value();
  REQUIRE(defs.size() == 1);

  auto *root = dynamic_cast<StmtAgentDef *>(defs[0].get());
  REQUIRE(root);
  auto *init = dynamic_cast<StmtBlock *>(root->m_Init.get());
  REQUIRE(init);
  auto *turn = dynamic_cast<StmtBlock *>(root->m_Turn.get());
  REQUIRE(turn);

  TrafficData data = {};
  ScriptedAgent agent(data, 0);

  root->m_Init.release();
  agent.SetInit(std::unique_ptr<StmtBlock>(init));

  root->m_Turn.release();
  agent.SetTurn(std::unique_ptr<StmtBlock>(turn));

  auto step_container = agent.GetTurn();
  REQUIRE_FALSE(step_container.empty());

  auto cur_step = step_container.get_next();
  REQUIRE(cur_step.has_value());
  REQUIRE(std::holds_alternative<MovementStep>(cur_step.value()));
  CHECK(std::get<MovementStep>(cur_step.value()).loc == WorldPosition{2, 1});
}

TEST_CASE("Infection built-ins reflect the current health state",
          "[infection][intg]") {
  struct Case {
    HealthState health;
    WorldPosition start;
    WorldPosition expected;
    char const *name;
  };

  std::array<Case, 3> cases{{
      {HealthState::INFECTED, {5, 5}, {6, 5}, "infected moves right"},
      {HealthState::SUSCEPTIBLE, {5, 5}, {5, 6}, "susceptible moves down"},
      {HealthState::RECOVERED, {5, 5}, {4, 5}, "recovered moves left"},
  }};

  for (auto const &tc : cases) {
    DYNAMIC_SECTION(tc.name) {
      auto [p, result] = parse(std::string{INFECTION_MAGIC_VALUES});

      CAPTURE(result);
      REQUIRE(result.has_value());
      auto &defs = result.value();
      REQUIRE(defs.size() == 1);

      auto *root = dynamic_cast<StmtAgentDef *>(defs[0].get());
      REQUIRE(root);
      auto *init = dynamic_cast<StmtBlock *>(root->m_Init.get());
      REQUIRE(init);
      auto *turn = dynamic_cast<StmtBlock *>(root->m_Turn.get());
      REQUIRE(turn);

      DiseaseData data = {};
      data.health = tc.health;
      data.position = tc.start;

      ScriptedAgent agent(data, 0);

      root->m_Init.release();
      agent.SetInit(std::unique_ptr<StmtBlock>(init));

      root->m_Turn.release();
      agent.SetTurn(std::unique_ptr<StmtBlock>(turn));

      auto step_container = agent.GetTurn();
      REQUIRE_FALSE(step_container.empty());

      auto cur_step = step_container.get_next();
      REQUIRE(cur_step.has_value());
      REQUIRE(std::holds_alternative<MovementStep>(cur_step.value()));
      CHECK(std::get<MovementStep>(cur_step.value()).loc == tc.expected);
    }
  }
}
