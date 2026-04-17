/**
 * @file ExprParserTest.cpp
 * @brief Tests for expression parsing (parse_expr).
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

TEST_CASE("Expr (Basic): 1+-1", "[expr][parser]") {
  auto [p, result] = parse("1+-1;");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 1);

  auto *expr_binary = dynamic_cast<ExprBinary *>(p.m_Nodes[0].get());
  REQUIRE(expr_binary != nullptr);
  CHECK(expr_binary->m_Sym->name == "+");
  CHECK(std::holds_alternative<int>(expr_binary->m_Sym->type));
  CHECK(dynamic_cast<ValLiteral *>(expr_binary->m_Value.get()) != nullptr);
}

TEST_CASE("Expr (Ambiguity): 2-1+1", "[expr][parser]") {
  auto [p, result] = parse("2-1+1;");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 1);

  auto *expr_binary = dynamic_cast<ExprBinary *>(p.m_Nodes[0].get());
  REQUIRE(expr_binary != nullptr);
  CHECK(expr_binary->m_Sym->name == "+");
  CHECK(std::holds_alternative<int>(expr_binary->m_Sym->type));
  CHECK(dynamic_cast<ValLiteral *>(expr_binary->m_Value.get()) != nullptr);
}

TEST_CASE("Expr (Precedence): 2-3*4", "[expr][parser]") {
  auto [p, result] = parse("2-3*4;");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 1);

  auto *expr_binary = dynamic_cast<ExprBinary *>(p.m_Nodes[0].get());
  REQUIRE(expr_binary != nullptr);
  CHECK(expr_binary->m_Sym->name == "+");
  CHECK(std::holds_alternative<int>(expr_binary->m_Sym->type));
  CHECK(dynamic_cast<ValLiteral *>(expr_binary->m_Value.get()) != nullptr);
}

TEST_CASE("Expr (PMDAS): 1+2*(3+4)+5", "[expr][parser]") {
  auto [p, result] = parse("1+2*(3+4)+5;");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 1);

  auto *expr_binary = dynamic_cast<ExprBinary *>(p.m_Nodes[0].get());
  REQUIRE(expr_binary != nullptr);
  CHECK(expr_binary->m_Sym->name == "+");
  CHECK(std::holds_alternative<int>(expr_binary->m_Sym->type));
  CHECK(dynamic_cast<ValLiteral *>(expr_binary->m_Value.get()) != nullptr);
}