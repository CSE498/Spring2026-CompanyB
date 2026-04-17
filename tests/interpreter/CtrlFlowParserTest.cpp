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

TEST_CASE("Ctrl-Flow: Basic Empty Stmt-Block (Int)", "[ctrl-flow][int][parser]") {
  auto [p, result] = parse("let i : int = 5;\nif (i == 5) {}");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 1);

  auto *expr_binary = dynamic_cast<ExprBinary *>(p.m_Nodes[0].get());
  REQUIRE(expr_binary != nullptr);
  CHECK(expr_binary->m_Sym->name == "+");
  CHECK(std::holds_alternative<int>(expr_binary->m_Sym->type));
  CHECK(dynamic_cast<ValLiteral *>(expr_binary->m_Value.get()) != nullptr);
}

TEST_CASE("Ctrl-Flow: Basic Empty Stmt (Int)", "[ctrl-flow][int][parser]") {
  auto [p, result] = parse("let i : int = 5;\nif (i == 5)");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 1);

  auto *expr_binary = dynamic_cast<ExprBinary *>(p.m_Nodes[0].get());
  REQUIRE(expr_binary != nullptr);
  CHECK(expr_binary->m_Sym->name == "+");
  CHECK(std::holds_alternative<int>(expr_binary->m_Sym->type));
  CHECK(dynamic_cast<ValLiteral *>(expr_binary->m_Value.get()) != nullptr);
}

TEST_CASE("Ctrl-Flow: Basic Stmt-Block (Int)", "[ctrl-flow][int][parser]") {
  auto [p, result] = parse("let i : int = 5;\nif (i == 5) {i = 10;}");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 1);

  auto *expr_binary = dynamic_cast<ExprBinary *>(p.m_Nodes[0].get());
  REQUIRE(expr_binary != nullptr);
  CHECK(expr_binary->m_Sym->name == "+");
  CHECK(std::holds_alternative<int>(expr_binary->m_Sym->type));
  CHECK(dynamic_cast<ValLiteral *>(expr_binary->m_Value.get()) != nullptr);
}

TEST_CASE("Ctrl-Flow: Basic Stmt (Int)", "[ctrl-flow][int][parser]") {
  auto [p, result] = parse("let i : int = 5;\nif (i == 5) i = 10;");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 1);

  auto *expr_binary = dynamic_cast<ExprBinary *>(p.m_Nodes[0].get());
  REQUIRE(expr_binary != nullptr);
  CHECK(expr_binary->m_Sym->name == "+");
  CHECK(std::holds_alternative<int>(expr_binary->m_Sym->type));
  CHECK(dynamic_cast<ValLiteral *>(expr_binary->m_Value.get()) != nullptr);
}

TEST_CASE("Ctrl-Flow: Else-if WITHOUT Else (Direction)", "[ctrl-flow][direction][parser]") {
  auto [p, result] = parse("let next_direction : direction = RIGHT;\nif (next_direction == LEFT) move(UP);\nelse if (next_direction == RIGHT) move(DOWN);");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 1);

  auto *expr_binary = dynamic_cast<ExprBinary *>(p.m_Nodes[0].get());
  REQUIRE(expr_binary != nullptr);
  CHECK(expr_binary->m_Sym->name == "+");
  CHECK(std::holds_alternative<int>(expr_binary->m_Sym->type));
  CHECK(dynamic_cast<ValLiteral *>(expr_binary->m_Value.get()) != nullptr);
}

TEST_CASE("Ctrl-Flow: Else-if WITH Else (Direction)", "[ctrl-flow][direction][parser]") {
  auto [p, result] = parse("let next_direction : direction = UP;\nif (next_direction == LEFT) move(UP);\nelse if (next_direction == RIGHT) move(DOWN);\nelse move(next_direction)");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 1);

  auto *expr_binary = dynamic_cast<ExprBinary *>(p.m_Nodes[0].get());
  REQUIRE(expr_binary != nullptr);
  CHECK(expr_binary->m_Sym->name == "+");
  CHECK(std::holds_alternative<int>(expr_binary->m_Sym->type));
  CHECK(dynamic_cast<ValLiteral *>(expr_binary->m_Value.get()) != nullptr);
}

TEST_CASE("Ctrl-Flow: Condition without Paren Error (double)", "[ctrl-flow][double][parser]") {
  auto [p, result] = parse("let x : double = 2.0;\nif x == 2.0 x = 1.0;");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 1);

  auto *expr_binary = dynamic_cast<ExprBinary *>(p.m_Nodes[0].get());
  REQUIRE(expr_binary != nullptr);
  CHECK(expr_binary->m_Sym->name == "+");
  CHECK(std::holds_alternative<int>(expr_binary->m_Sym->type));
  CHECK(dynamic_cast<ValLiteral *>(expr_binary->m_Value.get()) != nullptr);
}