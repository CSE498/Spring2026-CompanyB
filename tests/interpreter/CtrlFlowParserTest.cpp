/**
 * @file CtrlFlowParserTest.cpp
 * @brief Tests for control-flow parsing (parse_if).
 * @author Lalit Chitibomma
 */
#include <catch2/catch_test_macros.hpp>
#include <format>
#include <sstream>
#include <string>

#include "Interpreter/Parser.hpp"
#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"
#include "catch2/matchers/catch_matchers.hpp"
#include "interpreter-tests.hpp"

using namespace cse498;
using namespace cse498::AST;

// Parses the given statements under a "world traffic;" header.
// Returns the parser (which holds m_Nodes and m_Syms) and the parse result.
static std::pair<
    Parser,
    std::expected<std::vector<std::unique_ptr<StmtAgentDef>>, InterpErr>>
parse(std::string const& stmts) {
  Parser p;
  std::istringstream ss(stmts);
  auto result = p.parse(ss);
  return {std::move(p), std::move(result)};
}

// If, Else If, and Else Statements

const char* TEST_1 = R"V0G0N(

// Ctrl-Flow: Basic Empty Stmt-Block (Int)
world traffic;
let i : int = 5;
if (i == 5) {}

)V0G0N";

TEST_CASE("Ctrl-Flow: Basic Empty Stmt-Block (Int)",
          "[ctrl-flow][int][parser]") {
  auto [p, result] = parse(std::string{TEST_1});

  CAPTURE(result);
  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 2);

  auto* stmt_if = dynamic_cast<StmtIf*>(p.m_Nodes[1].get());
  REQUIRE(stmt_if != nullptr);
  CHECK(dynamic_cast<ExprBinary*>(stmt_if->m_Condition.get()) != nullptr);
  CHECK(dynamic_cast<StmtBlock*>(stmt_if->m_TBody.get()) != nullptr);
}

const char* TEST_2 = R"V0G0N(

// Ctrl-Flow: Basic Empty Stmt (Int)
world traffic;

let i : int = 5;
if (i == 5) // ERROR

)V0G0N";

TEST_CASE("Ctrl-Flow: Basic Empty Stmt (Int)", "[ctrl-flow][int][parser]") {
  auto [p, result] = parse(std::string{TEST_2});

  CAPTURE(result);
  REQUIRE_FALSE(result.has_value());
  REQUIRE(result.error().Is<ParseErr>(ParseErr::EXPECTED_STMT));
}

const char* TEST_3 = R"V0G0N(

// Ctrl-Flow: Basic Stmt-Block (Int)
world traffic;

let i : int = 5;
if (i == 5) {
  i = 10;
}

)V0G0N";
TEST_CASE("Ctrl-Flow: Basic Stmt-Block (Int)", "[ctrl-flow][int][parser]") {
  auto [p, result] = parse(std::string{TEST_3});

  CAPTURE(result);
  REQUIRE(result.has_value());
  CAPTURE(AST::IDNodeForTest(p.m_Nodes[0].get()));
  REQUIRE(p.m_Nodes.size() == 2);

  auto* stmt_if = dynamic_cast<StmtIf*>(p.m_Nodes[1].get());
  REQUIRE(stmt_if != nullptr);
  CHECK(dynamic_cast<ExprBinary*>(stmt_if->m_Condition.get()) != nullptr);
  CHECK(dynamic_cast<StmtBlock*>(stmt_if->m_TBody.get()) != nullptr);
}

const char* TEST_4 = R"V0G0N(

// Ctrl-Flow: Basic Stmt (Int)
world traffic;

let i : int = 5;
if (i == 5)
  i = 10;

)V0G0N";
TEST_CASE("Ctrl-Flow: Basic Stmt (Int)", "[ctrl-flow][int][parser]") {
  auto [p, result] = parse(std::string{TEST_4});

  CAPTURE(result);
  REQUIRE(result.has_value());

  REQUIRE(p.m_Nodes.size() == 2);

  auto* stmt_if = dynamic_cast<StmtIf*>(p.m_Nodes[1].get());
  REQUIRE(stmt_if != nullptr);
  CHECK(dynamic_cast<ExprBinary*>(stmt_if->m_Condition.get()) != nullptr);
  CHECK(dynamic_cast<Assign*>(stmt_if->m_TBody.get()) != nullptr);
}

const char* TEST_5 = R"V0G0N(

// Ctrl-Flow: Else-if WITHOUT Else (Direction)
world traffic;

let next_direction : direction = right;
let new_direction : direction = up;

if (next_direction == left)
  new_direction = left;
else if (next_direction == right)
  new_direction = right;

)V0G0N";
TEST_CASE("Ctrl-Flow: Else-if WITHOUT Else (Direction)",
          "[ctrl-flow][direction][parser]") {
  // Correction note - moves outside of a turn def are an error
  auto [p, result] = parse(std::string{TEST_5});

  CAPTURE(result);
  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 3);

  auto* first = dynamic_cast<Assign*>(p.m_Nodes[0].get());
  REQUIRE(first);
  auto* second = dynamic_cast<Assign*>(p.m_Nodes[1].get());
  REQUIRE(second);

  auto* stmt_if = dynamic_cast<StmtIf*>(p.m_Nodes[2].get());
  REQUIRE(stmt_if);
  REQUIRE(stmt_if->m_FBody.has_value());
  auto* stmt_else_if = dynamic_cast<StmtIf*>(stmt_if->m_FBody.value().get());
  REQUIRE(stmt_else_if);
  CHECK_FALSE(stmt_else_if->m_FBody.has_value());
}

const char* TEST_6 = R"V0G0N(

// Ctrl-Flow: Else-if WITH Else (Direction)
world traffic;

let next_direction : direction = right;
let new_direction : direction = up;

if (next_direction == left)
  new_direction = left;
else if (next_direction == right)
  new_direction = right;
else
  new_direction = down;

)V0G0N";
TEST_CASE("Ctrl-Flow: Else-if WITH Else (Direction)",
          "[ctrl-flow][direction][parser]") {
  auto [p, result] = parse(std::string{TEST_6});

  CAPTURE(result);
  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 3);

  auto* stmt_if = dynamic_cast<StmtIf*>(p.m_Nodes[2].get());
  REQUIRE(stmt_if);
  REQUIRE(stmt_if->m_FBody.has_value());
  auto* stmt_if_t = dynamic_cast<Assign*>(stmt_if->m_TBody.get());
  REQUIRE(stmt_if_t);
  auto* stmt_if_f = dynamic_cast<StmtIf*>(stmt_if->m_FBody.value().get());
  REQUIRE(stmt_if_f);
  REQUIRE(stmt_if_f->m_FBody.has_value());
  auto* stmt_if_f_t = dynamic_cast<Assign*>(stmt_if_f->m_TBody.get());
  REQUIRE(stmt_if_f_t);
  auto* stmt_if_f_f = dynamic_cast<Assign*>(stmt_if_f->m_FBody.value().get());
  REQUIRE(stmt_if_f_f);
}

const char* TEST_7 = R"V0G0N(

// Ctrl-Flow: Condition without Paren Error (double)
world traffic;

let x : double = 2.0;
if x == 2.0
  x = 1.0;

)V0G0N";
TEST_CASE("Ctrl-Flow: Condition without Paren Error (double)",
          "[ctrl-flow][error][double][parser]") {
  auto [p, result] = parse(std::string{TEST_7});

  REQUIRE_FALSE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 1);
}

// While Statements

const char* TEST_8 = R"V0G0N(

// Ctrl-Flow (While): Basic Empty Stmt-Block (Double)
world traffic;

let x : double = 0.0;
while (x < 2.0) {} // Not an error -- empty stmt block is fine

)V0G0N";
TEST_CASE("Ctrl-Flow (While): Basic Empty Stmt-Block (Double)",
          "[ctrl-flow][error][double][parser]") {
  auto [p, result] = parse(std::string{TEST_8});

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 2);

  auto* first = dynamic_cast<AST::Assign*>(p.m_Nodes[0].get());
  REQUIRE(first);
  auto* second = dynamic_cast<AST::StmtWhile*>(p.m_Nodes[1].get());
  REQUIRE(second);

  // TODO - further test nodes
}

const char* TEST_9 = R"V0G0N(

// Ctrl-Flow (While): Basic Empty Stmt (Error) (Double)
world traffic;

let x: double = 0.0;

while (x < 2.0) // ERROR

)V0G0N";
TEST_CASE("Ctrl-Flow (While): Basic Empty Stmt (Error) (Double)",
          "[ctrl-flow][double][parser]") {
  auto [p, result] = parse(std::string{TEST_9});

  CAPTURE(result);
  REQUIRE_FALSE(result.has_value());
  CHECK(result.error().Is<ParseErr>(ParseErr::EXPECTED_STMT));
}

const char* TEST_10 = R"V0G0N(

// Ctrl-Flow (While): Basic Stmt-Block (Double)
world traffic;
let x : double = 0.0;
while (x < 2.0) {
  x = x + 0.1;
}

)V0G0N";
TEST_CASE("Ctrl-Flow (While): Basic Stmt-Block (Double)",
          "[ctrl-flow][double][parser]") {
  auto [p, result] = parse(std::string{TEST_10});

  CAPTURE(result);
  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 2);

  auto* stmt_while = dynamic_cast<StmtWhile*>(p.m_Nodes[1].get());
  REQUIRE(stmt_while != nullptr);
  CHECK(dynamic_cast<ExprBinary*>(stmt_while->m_Condition.get()) != nullptr);
  CHECK(dynamic_cast<StmtBlock*>(stmt_while->m_Body.get()) != nullptr);
}

const char* TEST_11 = R"V0G0N(

// Ctrl-Flow (While): Basic Stmt (Double)
world traffic;
let x : double = 0.0;
while (x < 2.0)
  x = x + 0.1;

)V0G0N";
TEST_CASE("Ctrl-Flow (While): Basic Stmt (Double)",
          "[ctrl-flow][double][parser]") {
  auto [p, result] = parse(std::string{TEST_11});

  CAPTURE(result);
  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 2);

  auto* stmt_while = dynamic_cast<StmtWhile*>(p.m_Nodes[1].get());
  REQUIRE(stmt_while != nullptr);
  CHECK(dynamic_cast<ExprBinary*>(stmt_while->m_Condition.get()) != nullptr);
  CHECK(dynamic_cast<Assign*>(stmt_while->m_Body.get()) != nullptr);
}

const char* TEST_12 = R"V0G0N(

// Ctrl-Flow (Loop-Ctrl): Basic Break Statement (Double)
world traffic;

let x : double = 0.0;
while (x < 2.0) {
  x = x + 0.1;
  break;
}

)V0G0N";
TEST_CASE("Ctrl-Flow (Loop-Ctrl): Basic Break Statement (Double)",
          "[ctrl-flow][double][parser]") {
  auto [p, result] = parse(std::string{TEST_12});

  CAPTURE(result);
  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 2);

  auto* stmt_while = dynamic_cast<StmtWhile*>(p.m_Nodes[1].get());
  REQUIRE(stmt_while != nullptr);
  CHECK(dynamic_cast<ExprBinary*>(stmt_while->m_Condition.get()) != nullptr);

  auto* stmt_block = dynamic_cast<StmtBlock*>(stmt_while->m_Body.get());
  CHECK(stmt_block != nullptr);

  CHECK(dynamic_cast<StmtLoopCtl*>(stmt_block->m_Body[1].get()));
}

const char* TEST_13 = R"V0G0N(

// Ctrl-Flow (Loop-Ctrl): Basic Continue Statement (Double)
world traffic;

let x : double = 0.0;
while (x < 2.0) {
  x = x + 0.1;
  continue;
}

)V0G0N";
TEST_CASE("Ctrl-Flow (Loop-Ctrl): Basic Continue Statement (Double)",
          "[ctrl-flow][double][parser]") {
  auto [p, result] = parse(std::string{TEST_13});

  CAPTURE(result);
  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 2);

  auto* stmt_while = dynamic_cast<StmtWhile*>(p.m_Nodes[1].get());
  REQUIRE(stmt_while != nullptr);
  CHECK(dynamic_cast<ExprBinary*>(stmt_while->m_Condition.get()) != nullptr);

  auto* stmt_block = dynamic_cast<StmtBlock*>(stmt_while->m_Body.get());
  CHECK(stmt_block != nullptr);

  CHECK(dynamic_cast<StmtLoopCtl*>(stmt_block->m_Body[1].get()));
}
