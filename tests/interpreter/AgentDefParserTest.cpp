/**
 * @file AgentDefParserTest.cpp
 * @brief Tests for agent definition parsing (parse_agent_def).
 * @author Devansh Tayal
 */
#include <catch2/catch_test_macros.hpp>

#include "Interpreter/Parser.hpp"
#include "Interpreter/errors.hpp"

#include <sstream>
#include <string>

using namespace cse498;

static std::pair<Parser, std::expected<void, InterpErr>>
parse(std::string const &script) {
  Parser p;
  std::istringstream ss(script);
  auto result = p.parse(ss);
  return {std::move(p), std::move(result)};
}

TEST_CASE("AgentDef: empty init and turn are allowed", "[parser][agent-def]") {
  REQUIRE(parse("world infection;\n"
                "let walker : student {\n"
                "  init : { };\n"
                "  turn : { };\n"
                "};")
              .second
              .has_value());
}

TEST_CASE("AgentDef: goal/destination identifiers allowed in both contexts",
          "[parser][agent-def]") {
  REQUIRE(parse("world traffic;\n"
                "let driver : car {\n"
                "  init : {\n"
                "    let goal : int = 1;\n"
                "  };\n"
                "  turn : {\n"
                "    let destination : int = 2;\n"
                "  };\n"
                "};")
              .second
              .has_value());
}

TEST_CASE("AgentDef: move in init is an error", "[parser][agent-def]") {
  auto const result =
      parse("world infection;\n"
            "let walker : student {\n"
            "  init : move(up);\n"
            "  turn : { };\n"
            "};")
          .second;

  REQUIRE_FALSE(result.has_value());
  REQUIRE(std::holds_alternative<ParseErr>(result.error()));
  CHECK(std::get<ParseErr>(result.error()).m_Kind == ParseErr::OUT_OF_TURN);
}

TEST_CASE("AgentDef: move in turn parses", "[parser][agent-def]") {
  REQUIRE(parse("world infection;\n"
                "let walker : student {\n"
                "  init : { };\n"
                "  turn : {\n"
                "    move(up);\n"
                "  };\n"
                "};")
              .second
              .has_value());
}

TEST_CASE("AgentDef: symbols from init are visible in turn",
          "[parser][agent-def]") {
  REQUIRE(parse("world infection;\n"
                "let square_walker : student {\n"
                "  init : {\n"
                "    let step_idx : int = 0;\n"
                "  };\n"
                "  turn : {\n"
                "    step_idx;\n"
                "  };\n"
                "};")
              .second
              .has_value());
}
