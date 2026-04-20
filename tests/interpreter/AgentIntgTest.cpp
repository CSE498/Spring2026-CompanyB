/**
 * @file AgentIntgTest.cpp
 * @brief Integration tests for Scripted Agents
 * @author Lalit Chitibomma
 */
#include <catch2/catch_test_macros.hpp>

#include "Worlds/ScriptingTestWorld.hpp"
#include "Agents/ScriptedAgent.hpp"

#include "catch2/matchers/catch_matchers.hpp"
#include "interpreter-tests.hpp"

#include <format>
#include <sstream>
#include <string>

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

TEST_CASE("Basic Agent Test",
          "[traffic][intg]") {
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