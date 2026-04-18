/**
 * @file CtrlFlowParserTest.cpp
 * @brief Tests for control-flow parsing (parse_if).
 * @author Lalit Chitibomma
 */
#include <catch2/catch_test_macros.hpp>

#include "Interpreter/Parser.hpp"
#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"

#include <sstream>
#include <string>

using namespace cse498;
using namespace cse498::AST;

// Parses the given statements under a "world traffic;" header.
// Returns the parser (which holds m_Nodes and m_Syms) and the parse result.
static std::pair<Parser, std::expected<void, InterpErr>>
parse(std::string const &stmts) {
  Parser p;
  std::istringstream ss("world traffic;\n" + stmts);
  auto result = p.parse(ss);
  return {std::move(p), std::move(result)};
}

// If, Else If, and Else Statements

TEST_CASE("Ctrl-Flow: Basic Empty Stmt-Block (Int)", "[ctrl-flow][int][parser]") {
  auto [p, result] = parse("let i : int = 5;\nif (i == 5) {}");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 2);

  auto *stmt_if = dynamic_cast<StmtIf *>(p.m_Nodes[1].get());
  REQUIRE(stmt_if != nullptr);
  CHECK(dynamic_cast<ExprBinary *>(stmt_if->m_Condition.get()) != nullptr);
  CHECK(dynamic_cast<StmtBlock *>(stmt_if->m_TBody.get()) != nullptr);
}

TEST_CASE("Ctrl-Flow: Basic Empty Stmt (Int)", "[ctrl-flow][int][parser]") {
  auto [p, result] = parse("let i : int = 5;\nif (i == 5)");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 2);

  auto *stmt_if = dynamic_cast<StmtIf *>(p.m_Nodes[1].get());
  REQUIRE(stmt_if != nullptr);
  CHECK(dynamic_cast<ExprBinary *>(stmt_if->m_Condition.get()) != nullptr);
  CHECK(dynamic_cast<StmtBlock *>(stmt_if->m_TBody.get()) != nullptr);
}

TEST_CASE("Ctrl-Flow: Basic Stmt-Block (Int)", "[ctrl-flow][int][parser]") {
  auto [p, result] = parse("let i : int = 5;\nif (i == 5) {i = 10;}");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 2);

  auto *stmt_if = dynamic_cast<StmtIf *>(p.m_Nodes[1].get());
  REQUIRE(stmt_if != nullptr);
  CHECK(dynamic_cast<ExprBinary *>(stmt_if->m_Condition.get()) != nullptr);
  CHECK(dynamic_cast<StmtBlock *>(stmt_if->m_TBody.get()) != nullptr);
}

TEST_CASE("Ctrl-Flow: Basic Stmt (Int)", "[ctrl-flow][int][parser]") {
  auto [p, result] = parse("let i : int = 5;\nif (i == 5) i = 10;");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 2);

  auto *stmt_if = dynamic_cast<StmtIf *>(p.m_Nodes[1].get());
  REQUIRE(stmt_if != nullptr);
  CHECK(dynamic_cast<ExprBinary *>(stmt_if->m_Condition.get()) != nullptr);
  CHECK(dynamic_cast<StmtBlock *>(stmt_if->m_TBody.get()) != nullptr);
}

TEST_CASE("Ctrl-Flow: Else-if WITHOUT Else (Direction)", "[ctrl-flow][direction][parser]") {
  auto [p, result] = parse("let next_direction : direction = RIGHT;\nif (next_direction == LEFT) move(UP);\nelse if (next_direction == RIGHT) move(DOWN);");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 3);

  auto *stmt_if = dynamic_cast<StmtIf *>(p.m_Nodes[1].get());
  REQUIRE(stmt_if != nullptr);
  CHECK(dynamic_cast<ExprBinary *>(stmt_if->m_Condition.get()) != nullptr);
  CHECK(dynamic_cast<StmtBlock *>(stmt_if->m_TBody.get()) != nullptr);

  auto *stmt_else_if = dynamic_cast<StmtIf *>(p.m_Nodes[2].get());
  REQUIRE(stmt_else_if != nullptr);
  CHECK(dynamic_cast<ExprBinary *>(stmt_else_if->m_Condition.get()) != nullptr);
  CHECK(dynamic_cast<StmtBlock *>(stmt_else_if->m_TBody.get()) != nullptr);
}

TEST_CASE("Ctrl-Flow: Else-if WITH Else (Direction)", "[ctrl-flow][direction][parser]") {
  auto [p, result] = parse("let next_direction : direction = UP;\nif (next_direction == LEFT) move(UP);\nelse if (next_direction == RIGHT) move(DOWN);\nelse move(next_direction)");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 4);

  auto *stmt_if = dynamic_cast<StmtIf *>(p.m_Nodes[1].get());
  REQUIRE(stmt_if != nullptr);
  CHECK(dynamic_cast<ExprBinary *>(stmt_if->m_Condition.get()) != nullptr);
  CHECK(dynamic_cast<StmtBlock *>(stmt_if->m_TBody.get()) != nullptr);

  auto *stmt_else_if = dynamic_cast<StmtIf *>(p.m_Nodes[2].get());
  REQUIRE(stmt_else_if != nullptr);
  CHECK(dynamic_cast<ExprBinary *>(stmt_else_if->m_Condition.get()) != nullptr);
  CHECK(dynamic_cast<StmtBlock *>(stmt_else_if->m_TBody.get()) != nullptr);

  auto *stmt_else = dynamic_cast<StmtIf *>(p.m_Nodes[3].get());
  REQUIRE(stmt_else_if != nullptr);
  CHECK(dynamic_cast<ExprBinary *>(stmt_else_if->m_Condition.get()) != nullptr);
  CHECK(dynamic_cast<StmtBlock *>(stmt_else_if->m_TBody.get()) != nullptr);
}

TEST_CASE("Ctrl-Flow: Condition without Paren Error (double)", "[ctrl-flow][error][double][parser]") {
  auto [p, result] = parse("let x : double = 2.0;\nif x == 2.0 x = 1.0;");

  REQUIRE(result.has_value() == false);
  REQUIRE(p.m_Nodes.size() == 1);
}

// While Statements

TEST_CASE("Ctrl-Flow (While): Basic Empty Stmt-Block Error (Double)", "[ctrl-flow][error][double][parser]") {
  auto [p, result] = parse("let x : double = 0.0;\nwhile (x < 2.0) {}");

  REQUIRE(result.has_value() == false);
  REQUIRE(p.m_Nodes.size() == 1);
}

TEST_CASE("Ctrl-Flow (While): Basic Empty Stmt (Double)", "[ctrl-flow][double][parser]") {
  auto [p, result] = parse("let x : double = 0.0;\nwhile (x < 2.0)");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 2);

  auto *stmt_while = dynamic_cast<StmtWhile *>(p.m_Nodes[1].get());
  REQUIRE(stmt_while != nullptr);
  CHECK(dynamic_cast<ExprBinary *>(stmt_while->m_Condition.get()) != nullptr);
  CHECK(dynamic_cast<StmtBlock *>(stmt_while->m_Body.get()) != nullptr);
}

TEST_CASE("Ctrl-Flow (While): Basic Stmt-Block (Double)", "[ctrl-flow][double][parser]") {
  auto [p, result] = parse("let x : double = 0.0;\nwhile (x < 2.0) { x = x + 0.1 }");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 2);

  auto *stmt_while = dynamic_cast<StmtWhile *>(p.m_Nodes[1].get());
  REQUIRE(stmt_while != nullptr);
  CHECK(dynamic_cast<ExprBinary *>(stmt_while->m_Condition.get()) != nullptr);
  CHECK(dynamic_cast<StmtBlock *>(stmt_while->m_Body.get()) != nullptr);
}

TEST_CASE("Ctrl-Flow (While): Basic Stmt (Double)", "[ctrl-flow][double][parser]") {
  auto [p, result] = parse("let x : double = 0.0;\nwhile (x < 2.0) x = x + 0.1");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 2);

  auto *stmt_while = dynamic_cast<StmtWhile *>(p.m_Nodes[1].get());
  REQUIRE(stmt_while != nullptr);
  CHECK(dynamic_cast<ExprBinary *>(stmt_while->m_Condition.get()) != nullptr);
  CHECK(dynamic_cast<StmtBlock *>(stmt_while->m_Body.get()) != nullptr);
}

TEST_CASE("Ctrl-Flow (Loop-Ctrl): Basic Break Statement (Double)", "[ctrl-flow][double][parser]") {
  auto [p, result] = parse("let x : double = 0.0;\nwhile (x < 2.0) { x = x + 0.1; break; }");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 2);

  auto *stmt_while = dynamic_cast<StmtWhile *>(p.m_Nodes[1].get());
  REQUIRE(stmt_while != nullptr);
  CHECK(dynamic_cast<ExprBinary *>(stmt_while->m_Condition.get()) != nullptr);

  auto *stmt_block = dynamic_cast<StmtBlock *>(stmt_while->m_Body.get();
  CHECK(stmt_block != nullptr);

  CHECK(dynamic_cast<StmtLoopCtl *>(stmt_block->m_Body[1].get()));
}

TEST_CASE("Ctrl-Flow (Loop-Ctrl): Basic Continue Statement (Double)", "[ctrl-flow][double][parser]") {
  auto [p, result] = parse("let x : double = 0.0;\nwhile (x < 2.0) { x = x + 0.1; continue; }");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 2);

  auto *stmt_while = dynamic_cast<StmtWhile *>(p.m_Nodes[1].get());
  REQUIRE(stmt_while != nullptr);
  CHECK(dynamic_cast<ExprBinary *>(stmt_while->m_Condition.get()) != nullptr);

  auto *stmt_block = dynamic_cast<StmtBlock *>(stmt_while->m_Body.get();
  CHECK(stmt_block != nullptr);

  CHECK(dynamic_cast<StmtLoopCtl *>(stmt_block->m_Body[1].get()));
}