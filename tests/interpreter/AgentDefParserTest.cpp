/**
 * @file AgentDefParserTest.cpp
 * @brief Tests for agent definition parsing (parse_agent_def).
 * @author Devansh Tayal
 */
#include <catch2/catch_test_macros.hpp>

#include "Interpreter/Parser.hpp"
#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"

#include "interpreter-tests.hpp"

#include <sstream>
#include <string>

using namespace cse498;
using namespace cse498::AST;

static std::pair<
    Parser,
    std::expected<std::vector<std::unique_ptr<StmtAgentDef>>, InterpErr>>
parse(std::string const &script) {
  Parser p;
  std::istringstream ss(script);
  auto result = p.parse(ss);
  return {std::move(p), std::move(result)};
}

TEST_CASE("AgentDef: empty init and turn are allowed", "[parser][agent-def]") {
  auto [p, result] = parse("world infection;\n"
                           "let walker : student {\n"
                           "  init : { };\n"
                           "  turn : { };\n"
                           "};");

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
}

TEST_CASE("AgentDef: goal/destination identifiers allowed in both contexts",
          "[parser][agent-def]") {
  auto [p, result] = parse("world traffic;\n"
                           "let driver : car {\n"
                           "  init : {\n"
                           "    let goal : int = 1;\n"
                           "  };\n"
                           "  turn : {\n"
                           "    let destination : int = 2;\n"
                           "  };\n"
                           "};");

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
}

TEST_CASE("AgentDef: move in init is an error", "[parser][agent-def]") {
  auto [p, result] = parse("world infection;\n"
                           "let walker : student {\n"
                           "  init : move(up);\n"
                           "  turn : { };\n"
                           "};");

  REQUIRE_FALSE(result.has_value());
  REQUIRE(std::holds_alternative<ParseErr>(result.error()));
  CHECK(std::get<ParseErr>(result.error()).m_Kind == ParseErr::OUT_OF_TURN);

  REQUIRE(p.m_AgentDefs.empty());
}

TEST_CASE("AgentDef: move in turn parses", "[parser][agent-def]") {
  auto [p, result] = parse("world infection;\n"
                           "let walker : student {\n"
                           "  init : { };\n"
                           "  turn : {\n"
                           "    move(up);\n"
                           "  };\n"
                           "};");

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
}

TEST_CASE("AgentDef: symbols from init are visible in turn",
          "[parser][agent-def]") {
  auto [p, result] = parse("world infection;\n"
                           "let square_walker : student {\n"
                           "  init : {\n"
                           "    let step_idx : int = 0;\n"
                           "  };\n"
                           "  turn : {\n"
                           "    step_idx;\n"
                           "  };\n"
                           "};");

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
}
