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
parse(std::string const& stmts) {
  Parser p;
  std::istringstream ss(stmts);
  auto result = p.parse(ss);
  return {std::move(p), std::move(result)};
}

const char* BASIC_FUNC_TESTS = R"V0G0N(
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

TEST_CASE("User function called with too many args fails at parse",
          "[func][parser]") {
  auto [p, result] = parse(R"V0G0N(
world traffic;
fn add(a : int, b : int) : int {
    return a + b;
}
add(1, 2, 3);
)V0G0N");

  CAPTURE(result);
  REQUIRE_FALSE(result.has_value());
  CHECK(result.error().Is<RuntimeErr>(RuntimeErr::TOO_MANY_ARGS));
}

TEST_CASE("Parser reuse: second parse sees only new nodes", "[parser][reuse]") {
  Parser p;

  std::istringstream ss1("world traffic;\nlet a : int = 1;\nlet b : int = 2;");
  auto result1 = p.parse(ss1);
  REQUIRE(result1.has_value());
  REQUIRE(p.m_Nodes.size() == 2);

  std::istringstream ss2("world traffic;\nlet x : int = 10;");
  auto result2 = p.parse(ss2);
  CAPTURE(result2);
  REQUIRE(result2.has_value());
  CHECK(p.m_Nodes.size() == 1);
}

TEST_CASE("Parser reuse: success after failure produces clean state",
          "[parser][reuse]") {
  Parser p;

  std::istringstream ss_bad(
      "world traffic;\nlet a : int = 1;\nlet a : int = 2;");
  auto bad = p.parse(ss_bad);
  REQUIRE_FALSE(bad.has_value());

  std::istringstream ss_good("world traffic;\nlet b : int = 99;");
  auto good = p.parse(ss_good);
  CAPTURE(good);
  REQUIRE(good.has_value());
  CHECK(p.m_Nodes.size() == 1);
}
