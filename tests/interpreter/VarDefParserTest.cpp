/**
 * @file VarDefParserTest.cpp
 * @brief Tests for variable definition parsing (parse_var_def).
 * @author Chirag Bhansali
 */
#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <string>
#include <variant>

#include "Interpreter/Parser.hpp"
#include "Interpreter/SymbolTable.hpp"
#include "Interpreter/agentlang.hpp"
#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"
#include "test-tools/matchers.hpp"

using namespace cse498;
using namespace cse498::AST;

using matchers::VariantState;

// Parses the given statements under a "world traffic;" header.
// Returns the parser (which holds m_Nodes and m_Syms) and the parse result.
static std::pair<
    Parser,
    std::expected<std::vector<std::unique_ptr<StmtAgentDef>>, InterpErr>>
parse(std::string const &stmts) {
  Parser p;
  std::istringstream ss("world traffic;\n" + stmts);
  auto result = p.parse(ss);
  return {std::move(p), std::move(result)};
}

using Sym_T = std::shared_ptr<SymInfo>;

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

  REQUIRE(assign->m_Sym->sym.IsA<VarSym>());
  REQUIRE(
      std::holds_alternative<int>(std::get<VarSym>(assign->m_Sym->sym).m_Type));

  CHECK(dynamic_cast<ValLiteral *>(assign->m_Value.get()) != nullptr);
}

TEST_CASE("VarDef: double literal", "[vardef][parser]") {
  auto [p, result] = parse("let x : double = 3.14;");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 1);

  auto *assign = dynamic_cast<Assign *>(p.m_Nodes[0].get());
  REQUIRE(assign != nullptr);

  CHECK(assign->m_Sym->name == "x");

  REQUIRE(assign->m_Sym->sym.IsA<VarSym>());
  REQUIRE_THAT(assign->m_Sym->sym.As<VarSym>().m_Type, VariantState<double>());

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

  REQUIRE(assign->m_Sym->sym.IsA<VarSym>());
  REQUIRE_THAT(assign->m_Sym->sym.As<VarSym>().m_Type, VariantState<bool>());

  CHECK(dynamic_cast<ExprBinary *>(assign->m_Value.get()) != nullptr);
}

TEST_CASE("VarDef: string literal", "[vardef][parser]") {
  auto [p, result] = parse("let label : str = \"main\";");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 1);

  auto *assign = dynamic_cast<Assign *>(p.m_Nodes[0].get());
  REQUIRE(assign != nullptr);

  CHECK(assign->m_Sym->name == "label");

  REQUIRE(assign->m_Sym->sym.IsA<VarSym>());
  REQUIRE_THAT(assign->m_Sym->sym.As<VarSym>().m_Type,
               VariantState<std::string>());

  CHECK(dynamic_cast<ValLiteral *>(assign->m_Value.get()) != nullptr);
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

  REQUIRE(assign->m_Sym->sym.IsA<VarSym>());
  REQUIRE_THAT(assign->m_Sym->sym.As<VarSym>().m_Type, VariantState<int>());

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

  auto *assign1 = dynamic_cast<Assign *>(p.m_Nodes[0].get());
  REQUIRE(assign1 != nullptr);

  CHECK(assign1->m_Sym->name == "a");

  REQUIRE(assign1->m_Sym->sym.IsA<VarSym>());
  REQUIRE_THAT(assign1->m_Sym->sym.As<VarSym>().m_Type, VariantState<int>());

  auto *assign2 = dynamic_cast<Assign *>(p.m_Nodes[1].get());
  REQUIRE(assign2 != nullptr);

  CHECK(assign2->m_Sym->name == "b");

  REQUIRE(assign2->m_Sym->sym.IsA<VarSym>());
  REQUIRE_THAT(assign2->m_Sym->sym.As<VarSym>().m_Type, VariantState<double>());

  CHECK(dynamic_cast<ValLiteral *>(assign2->m_Value.get()) != nullptr);

  CHECK(dynamic_cast<ValLiteral *>(assign1->m_Value.get()) != nullptr);
}

// ------------------------------------------------------------
// Symbol-related behavior
// ------------------------------------------------------------

TEST_CASE("VarDef: use previously defined variable in new definition",
          "[vardef][parser][symbols]") {
  auto [p, result] = parse("let i : int = 5;\nlet j : int = i;");

  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 2);

  auto *assign = dynamic_cast<Assign *>(p.m_Nodes[1].get());
  REQUIRE(assign != nullptr);

  CHECK(assign->m_Sym->name == "j");

  REQUIRE(assign->m_Sym->sym.IsA<VarSym>());
  REQUIRE_THAT(assign->m_Sym->sym.As<VarSym>().m_Type, VariantState<int>());

  CHECK(dynamic_cast<ValVariable *>(assign->m_Value.get()) != nullptr);
}

TEST_CASE("VarDef: redefining a symbol is an error",
          "[vardef][parser][symbols]") {
  auto [p, result] = parse("let i : int = 5;\nlet i : int = 6;");

  REQUIRE_FALSE(result.has_value());
  REQUIRE(std::holds_alternative<SymbolErr>(result.error()));
  CHECK(std::get<SymbolErr>(result.error()).m_Kind == SymbolErr::REDEFINITION);
}
