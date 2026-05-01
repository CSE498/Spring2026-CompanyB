/**
 * @file ExprParserTest.cpp
 * @brief Tests for expression parsing (parse_expr).
 * @author Lalit Chitibomma
 */
#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <string>

#include "Interpreter/Evaluation/OpVisits.hpp"
#include "Interpreter/Parser.hpp"
#include "Interpreter/agentlang.hpp"
#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"
#include "Interpreter/lexer.hpp"
#include "interpreter-tests.hpp"
#include "test-tools/matchers.hpp"

using namespace cse498;
using namespace cse498::AST;
using namespace cse498::matchers;
using AgentLexer::IDs;

// Parses the given statements under a "world traffic;" header.
// Returns the parser (which holds m_Nodes and m_Syms) and the parse result.
static std::pair<
    Parser,
    std::expected<std::vector<std::unique_ptr<StmtAgentDef>>, InterpErr>>
parse(std::string const& stmts) {
  Parser p;
  std::istringstream ss("world traffic;\n" + stmts);
  auto result = p.parse(ss);
  return {std::move(p), std::move(result)};
}

const char* LITERALS_TEST = R"V0G0N(
// Check all literals are correctly parsed
up;
down;
left;
right;
0.0;
0.;
0;
true;
false;
"foo";

)V0G0N";
TEST_CASE(
    "Literals"
    "[expr][literal][parser]") {
  auto [p, result] = parse(std::string{LITERALS_TEST});

  CAPTURE(result);
  REQUIRE(result.has_value());

  using Types::Dir;
  using Types::Type;

  std::vector<Type> check_values{
      Type{Dir::UP}, Type{Dir::DOWN}, Type{Dir::LEFT}, Type{Dir::RIGHT},
      Type{0.0},     Type{0.0},       Type{0},         Type{true},
      Type{false},   Type{"foo"}};

  size_t i = 0;
  for (Type cur_type : check_values) {
    auto* lit = dynamic_cast<ValLiteral*>(p.m_Nodes[i].get());
    REQUIRE(lit);
    CHECK(lit->m_Val.index() == cur_type.index());
    // Make use of opvisits to compare equality
    CHECK(evaluate_binary(AgentLexer::IDs::ID_CMP_EQ, lit->m_Val, cur_type));
    ++i;
  }

  CHECK(dynamic_cast<ValLiteral*>(p.m_Nodes[0].get()));
}

TEST_CASE("Expr (Basic): 1+-1", "[expr][parser]") {
  auto [p, result] = parse("1+-1;");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 1);

  auto* root = dynamic_cast<ExprBinary*>(p.m_Nodes[0].get());
  REQUIRE(root);
  auto* left = dynamic_cast<ValLiteral*>(root->m_Left.get());
  REQUIRE(left);
  auto* right = dynamic_cast<ExprUnary*>(root->m_Right.get());
  REQUIRE(right);
  auto* right_left = dynamic_cast<ValLiteral*>(right->m_Left.get());
  REQUIRE(right_left);

  CHECK_THAT(left->m_Val, VariantHas<int>(1));
  CHECK_THAT(right_left->m_Val, VariantHas<int>(1));
}

TEST_CASE("Expr (Ambiguity): 2-1+1", "[expr][parser]") {
  auto [p, result] = parse("2-1+1;");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 1);

  // ExprBinary

  auto* root = dynamic_cast<ExprBinary*>(p.m_Nodes[0].get());
  REQUIRE(root);
  auto* left = dynamic_cast<ExprBinary*>(root->m_Left.get());
  REQUIRE(left);
  auto* right = dynamic_cast<ValLiteral*>(root->m_Right.get());
  REQUIRE(right);
  auto* left_left = dynamic_cast<ValLiteral*>(left->m_Left.get());
  REQUIRE(left_left);
  auto* left_right = dynamic_cast<ValLiteral*>(left->m_Right.get());
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

  auto* root = dynamic_cast<ExprBinary*>(p.m_Nodes[0].get());
  REQUIRE(root);
  auto* left = dynamic_cast<ValLiteral*>(root->m_Left.get());
  REQUIRE(left);
  auto* right = dynamic_cast<ExprBinary*>(root->m_Right.get());
  REQUIRE(right);
  auto* right_left = dynamic_cast<ValLiteral*>(right->m_Left.get());
  REQUIRE(right_left);
  auto* right_right = dynamic_cast<ValLiteral*>(right->m_Right.get());
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

  auto* root = dynamic_cast<ExprBinary*>(p.m_Nodes[0].get());
  REQUIRE(root);
  auto* left = dynamic_cast<ExprBinary*>(root->m_Left.get());
  REQUIRE(left);
  auto* left_left = dynamic_cast<ValLiteral*>(left->m_Left.get());
  REQUIRE(left_left);
  auto* left_right = dynamic_cast<ExprBinary*>(left->m_Right.get());
  REQUIRE(left_right);
  auto* left_right_left = dynamic_cast<ValLiteral*>(left_right->m_Left.get());
  REQUIRE(left_right_left);
  auto* left_right_right = dynamic_cast<ExprBinary*>(left_right->m_Right.get());
  REQUIRE(left_right_right);
  auto* left_right_right_left =
      dynamic_cast<ValLiteral*>(left_right_right->m_Left.get());
  REQUIRE(left_right_right_left);
  auto* left_right_right_right =
      dynamic_cast<ValLiteral*>(left_right_right->m_Right.get());
  REQUIRE(left_right_right_right);
  auto* right = dynamic_cast<ValLiteral*>(root->m_Right.get());
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
