#include <catch2/catch_test_macros.hpp>

#include "Interpreter/Parser.hpp"
#include "Interpreter/ast.hpp"
#include "Interpreter/lexer.hpp"
#include "interpreter-tests.hpp"

using namespace cse498;
using AgentLexer::IDs;
using namespace agentlang;

static std::pair<
    Parser,
    std::expected<std::vector<std::unique_ptr<AST::StmtAgentDef>>, InterpErr>>
parse(std::string const &stmts) {
  Parser p;
  std::istringstream ss(stmts);
  auto result = p.parse(ss);
  return {std::move(p), std::move(result)};
}

const char *BASIC_FUNC_TESTS = R"V0G0N(
world traffic;
// Empty function, always returns 10;
fn ten() : int {
  return 10;
}

// Identity function for integer
fn ident(val : int) : int {
  return val;
}

// Invoke identity function and do nothing with it
ident(5);

// Define a variable as the invocation of identity function 
let a : int = ident(5);

// Use function return as value in an expression
a = a + ten();
a = ten() + ident(a);

// Multiple parameters
fn mid(a : int, b : int) : double {
  return (a + b) / 2;
}

let middle : double = mid(5, 10);

// Permit recursion
fn fact(val : int, step : int) : int {
  // Error case
  if (step < 0) return 0;

  // Base case
  else if (step == 1) return step;

  // Recurive case
  else return fact(val*(step - 1), step - 1);
}

fn factorial(val : int) : int {
  return fact(val, val);
}

)V0G0N";

TEST_CASE("Basic function tests", "[func][parser]") {
  auto [p, result] = parse(std::string{BASIC_FUNC_TESTS});

  CAPTURE(result);
  REQUIRE(result.has_value());
  REQUIRE(p.m_Nodes.size() == 10);
}
