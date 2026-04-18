/**
 * @file ExprParserTest.cpp
 * @brief Tests for expression parsing (parse_expr).
 * @author Lalit Chitibomma
 */
#include <catch2/catch_test_macros.hpp>

#include "Interpreter/Parser.hpp"
#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"
#include "Interpreter/lexer.hpp"
#include "test-tools/matchers.hpp"

#include "interpreter-tests.hpp"

#include <sstream>
#include <string>

using namespace cse498;
using namespace cse498::AST;
using namespace cse498::matchers;
using AgentLexer::IDs;

// Parses the given statements under a "world traffic;" header.
// Returns the parser (which holds m_Nodes and m_Syms) and the parse result.
static std::pair<Parser, std::expected<std::vector<std::unique_ptr<StmtAgentDef>>, InterpErr>>
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

  auto *root = dynamic_cast<ExprBinary *>(p.m_Nodes[0].get());
  REQUIRE(root);
  auto *left = dynamic_cast<ValLiteral *>(root->m_Left.get());
  REQUIRE(left);
  auto *right = dynamic_cast<ExprUnary *>(root->m_Right.get());
  REQUIRE(right);
  auto *right_left = dynamic_cast<ValLiteral *>(right->m_Left.get());
  REQUIRE(right_left);

  CHECK_THAT(left->m_Val, VariantHas<int>(1));
  CHECK_THAT(right_left->m_Val, VariantHas<int>(1));
}

TEST_CASE("Expr (Ambiguity): 2-1+1", "[expr][parser]") {
  auto [p, result] = parse("2-1+1;");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 1);

  // ExprBinary

  auto *root = dynamic_cast<ExprBinary *>(p.m_Nodes[0].get());
  REQUIRE(root);
  auto *left = dynamic_cast<ExprBinary *>(root->m_Left.get());
  REQUIRE(left);
  auto *right = dynamic_cast<ValLiteral *>(root->m_Right.get());
  REQUIRE(right);
  auto *left_left = dynamic_cast<ValLiteral *>(left->m_Left.get());
  REQUIRE(left_left);
  auto *left_right = dynamic_cast<ValLiteral *>(left->m_Right.get());
  REQUIRE(left_right);

  CHECK(root->m_Token.id == AgentLexer::IDs::ID_OP_ADD);
  CHECK(left->m_Token.id == AgentLexer::IDs::ID_OP_MINUS);
  CHECK_THAT(right->m_Val, VariantHas<int>(1));
  CHECK_THAT(left_left->m_Val, VariantHas<int>(2));
  CHECK_THAT(left_right->m_Val, VariantHas<int>(1));
}

TEST_CASE("Expr (Precedence): 2-3*4", "[expr][parser]") {
  auto [p, result] = parse("2-3*4;");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 1);

  // ExprBinary

  auto *root = dynamic_cast<ExprBinary *>(p.m_Nodes[0].get());
  REQUIRE(root);
  auto *left = dynamic_cast<ValLiteral *>(root->m_Left.get());
  REQUIRE(left);
  auto *right = dynamic_cast<ExprBinary *>(root->m_Right.get());
  REQUIRE(right);
  auto *right_left = dynamic_cast<ValLiteral *>(right->m_Left.get());
  REQUIRE(right_left);
  auto *right_right = dynamic_cast<ValLiteral *>(right->m_Right.get());
  REQUIRE(right_right);

  CHECK(root->m_Token.id == AgentLexer::IDs::ID_OP_MINUS);
  CHECK(right->m_Token.id == AgentLexer::IDs::ID_OP_MULT);
  CHECK_THAT(left->m_Val, VariantHas<int>(2));
  CHECK_THAT(right_left->m_Val, VariantHas<int>(3));
  CHECK_THAT(right_right->m_Val, VariantHas<int>(4));
}

TEST_CASE("Expr (PMDAS): 1+2*(3+4)+5", "[expr][parser]") {
  auto [p, result] = parse("1+2*(3+4)+5;");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 1);

  // ExprBinary

  auto *root = dynamic_cast<ExprBinary *>(p.m_Nodes[0].get());
  REQUIRE(root);
  auto *left = dynamic_cast<ExprBinary *>(root->m_Left.get());
  REQUIRE(left);
  auto *left_left = dynamic_cast<ValLiteral *>(left->m_Left.get());
  REQUIRE(left_left);
  auto *left_right = dynamic_cast<ExprBinary *>(left->m_Right.get());
  REQUIRE(left_right);
  auto *left_right_left = dynamic_cast<ValLiteral *>(left_right->m_Left.get());
  REQUIRE(left_right_left);
  auto *left_right_right =
      dynamic_cast<ExprBinary *>(left_right->m_Right.get());
  REQUIRE(left_right_right);
  auto *left_right_right_left =
      dynamic_cast<ValLiteral *>(left_right_right->m_Left.get());
  REQUIRE(left_right_right_left);
  auto *left_right_right_right =
      dynamic_cast<ValLiteral *>(left_right_right->m_Right.get());
  REQUIRE(left_right_right_right);
  auto *right = dynamic_cast<ValLiteral *>(root->m_Right.get());
  REQUIRE(right);

  CHECK(root->m_Token.id == IDs::ID_OP_ADD);
  CHECK(left->m_Token.id == IDs::ID_OP_ADD);
  CHECK(left_right->m_Token.id == IDs::ID_OP_MULT);
  CHECK(left_right_right->m_Token.id == IDs::ID_OP_ADD);

  CHECK_THAT(left_left->m_Val, VariantHas<int>(1));
  CHECK_THAT(left_right_left->m_Val, VariantHas<int>(2));
  CHECK_THAT(left_right_right_left->m_Val, VariantHas<int>(3));
  CHECK_THAT(left_right_right_right->m_Val, VariantHas<int>(4));
  CHECK_THAT(right->m_Val, VariantHas<int>(5));
}
