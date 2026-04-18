/**
 * @file VarDefParserTest.cpp
 * @brief Tests for variable definition parsing (parse_var_def).
 * @author Chirag Bhansali
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
static std::pair<Parser, std::expected<std::vector<std::unique_ptr<StmtAgentDef>>, InterpErr>>
parse(std::string const &stmts) {
  Parser p;
  std::istringstream ss("world traffic;\n" + stmts);
  auto result = p.parse(ss);
  return {std::move(p), std::move(result)};
}

// ------------------------------------------------------------
// Literal definitions
// ------------------------------------------------------------

TEST_CASE("VarDef: int literal", "[vardef][parser]") {
  auto [p, result] = parse("let i : int = 5;");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 1);

  auto *assign = dynamic_cast<Assign *>(p.m_Nodes[0].get());
  REQUIRE(assign != nullptr);
  CHECK(assign->m_Sym->name == "i");
  CHECK(std::holds_alternative<int>(assign->m_Sym->type));
  CHECK(dynamic_cast<ValLiteral *>(assign->m_Value.get()) != nullptr);
}

TEST_CASE("VarDef: double literal", "[vardef][parser]") {
  auto [p, result] = parse("let x : double = 3.14;");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 1);

  auto *assign = dynamic_cast<Assign *>(p.m_Nodes[0].get());
  REQUIRE(assign != nullptr);
  CHECK(assign->m_Sym->name == "x");
  CHECK(std::holds_alternative<double>(assign->m_Sym->type));
  CHECK(dynamic_cast<ValLiteral *>(assign->m_Value.get()) != nullptr);
}

TEST_CASE("VarDef: bool from comparison expression", "[vardef][parser]") {
  // agentlang has no bool literals; booleans come from comparisons
  auto [p, result] = parse("let flag : bool = 1 == 1;");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 1);

  auto *assign = dynamic_cast<Assign *>(p.m_Nodes[0].get());
  REQUIRE(assign != nullptr);
  CHECK(assign->m_Sym->name == "flag");
  CHECK(std::holds_alternative<bool>(assign->m_Sym->type));
  CHECK(dynamic_cast<ExprBinary *>(assign->m_Value.get()) != nullptr);
}

// ------------------------------------------------------------
// Expression definitions
// ------------------------------------------------------------

TEST_CASE("VarDef: binary expression RHS", "[vardef][parser]") {
  auto [p, result] = parse("let j : int = 1 + 1;");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 1);

  auto *assign = dynamic_cast<Assign *>(p.m_Nodes[0].get());
  REQUIRE(assign != nullptr);
  CHECK(assign->m_Sym->name == "j");
  CHECK(dynamic_cast<ExprBinary *>(assign->m_Value.get()) != nullptr);
}

TEST_CASE("VarDef: compound arithmetic expression RHS", "[vardef][parser]") {
  auto [p, result] = parse("let k : int = 2 * 3 + 1;");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 1);

  auto *assign = dynamic_cast<Assign *>(p.m_Nodes[0].get());
  REQUIRE(assign != nullptr);
  CHECK(dynamic_cast<ExprBinary *>(assign->m_Value.get()) != nullptr);
}

// ------------------------------------------------------------
// Multiple definitions in sequence
// ------------------------------------------------------------

TEST_CASE("VarDef: multiple sequential definitions", "[vardef][parser]") {
  auto [p, result] = parse("let a : int = 1;\nlet b : double = 2.0;");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 2);

  auto *first = dynamic_cast<Assign *>(p.m_Nodes[0].get());
  auto *second = dynamic_cast<Assign *>(p.m_Nodes[1].get());
  REQUIRE(first != nullptr);
  REQUIRE(second != nullptr);
  CHECK(first->m_Sym->name == "a");
  CHECK(second->m_Sym->name == "b");
}

// ------------------------------------------------------------
// Symbol-related behavior
// ------------------------------------------------------------

TEST_CASE("VarDef: use previously defined variable in new definition",
          "[vardef][parser][symbols]") {
  auto [p, result] = parse("let i : int = 5;\nlet j : int = i;");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 2);

  auto *second = dynamic_cast<Assign *>(p.m_Nodes[1].get());
  REQUIRE(second != nullptr);
  CHECK(second->m_Sym->name == "j");
  CHECK(dynamic_cast<ValVariable *>(second->m_Value.get()) != nullptr);
}

TEST_CASE("VarDef: redefining a symbol is an error", "[vardef][parser][symbols]") {
  auto [p, result] = parse("let i : int = 5;\nlet i : int = 6;");

  REQUIRE_FALSE(result.has_value());
  REQUIRE(std::holds_alternative<SymbolErr>(result.error()));
  CHECK(std::get<SymbolErr>(result.error()).m_Kind == SymbolErr::REDEFINITION);
}
