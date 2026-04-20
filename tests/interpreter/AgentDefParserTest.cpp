/**
 * @file AgentDefParserTest.cpp
 * @brief Tests for agent definition parsing (parse_agent_def).
 * @author Devansh Tayal
 */
#include <array>
#include <catch2/catch_test_macros.hpp>
#include <expected>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "Interpreter/Parser.hpp"
#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"
#include "interpreter-tests.hpp"

using cse498::InterpErr;
using cse498::ParseErr;
using cse498::Parser;
using cse498::AST::StmtAgentDef;
using cse498::AST::StmtBlock;

using ParseResult =
    std::expected<std::vector<std::unique_ptr<StmtAgentDef>>, InterpErr>;

struct ParseOutput {
  Parser parser;
  ParseResult result;
};

static ParseOutput parse(std::string_view script) {
  Parser p;
  std::istringstream ss{std::string(script)};
  auto result = p.parse(ss);
  return {std::move(p), std::move(result)};
}

static StmtAgentDef& require_one_agent_def(ParseResult& result) {
  REQUIRE(result.has_value());
  auto& defs = result.value();
  REQUIRE(defs.size() == 1);
  return *defs.front();
}

static void require_init_and_turn_blocks(StmtAgentDef& def) {
  REQUIRE(dynamic_cast<StmtBlock*>(def.m_Init.get()) != nullptr);
  REQUIRE(dynamic_cast<StmtBlock*>(def.m_Turn.get()) != nullptr);
}

constexpr std::string_view kStudentAgentInTrafficWorld = R"V0G0N(
world traffic;
let agent : student {
    init : { };
    turn : { };
};
)V0G0N";

constexpr std::string_view kCarAgentInInfectionWorld = R"V0G0N(
world infection;
let agent : car {
    init : { };
    turn : { };
};
)V0G0N";

constexpr std::string_view kEmptyInitAndTurn = R"V0G0N(
world infection;
let walker : student {
  init : { };
  turn : { };
};
)V0G0N";

constexpr std::string_view kGoalAndDestination = R"V0G0N(
world traffic;
let driver : car {
  init : {
    let goal : int = 1;
  };
  turn : {
    let destination : int = 2;
  };
};
)V0G0N";

constexpr std::string_view kMoveInInit = R"V0G0N(
world infection;
let walker : student {
  init : move(up);
  turn : { };
};
)V0G0N";

constexpr std::string_view kMoveInTurn = R"V0G0N(
world infection;
let walker : student {
  init : { };
  turn : {
    move(up);
  };
};
)V0G0N";

constexpr std::string_view kInitSymbolsVisibleInTurn = R"V0G0N(
world infection;
let square_walker : student {
  init : {
    let step_idx : int = 0;
  };
  turn : {
    step_idx;
  };
};
)V0G0N";

struct ErrorCase {
  std::string_view name;
  std::string_view script;
  ParseErr::Kind expected;
};

constexpr std::array<ErrorCase, 2> kErrorCases{{
    ErrorCase{"car cannot be instantiated in infection",
              kCarAgentInInfectionWorld, ParseErr::WORLD_MISMATCH},
    ErrorCase{"student cannot be instantiated in traffic",
              kStudentAgentInTrafficWorld, ParseErr::WORLD_MISMATCH},
}};

struct SuccessCase {
  std::string_view name;
  std::string_view script;
};

constexpr std::array<SuccessCase, 4> kSuccessCases{{
    SuccessCase{"empty init and turn are allowed", kEmptyInitAndTurn},
    SuccessCase{"goal/destination identifiers are allowed",
                kGoalAndDestination},
    SuccessCase{"move in turn parses", kMoveInTurn},
    SuccessCase{"symbols from init are visible in turn",
                kInitSymbolsVisibleInTurn},
}};

// https://github.com/catchorg/Catch2/blob/devel/docs/other-macros.md
TEST_CASE("AgentDef parser", "[parser][agent-def]") {
  SECTION("rejects invalid world/agent combinations") {
    for (auto const& tc : kErrorCases) {
      DYNAMIC_SECTION(tc.name) {
        auto out = parse(tc.script);

        REQUIRE_FALSE(out.result.has_value());
        CHECK(out.result.error().Is<ParseErr>(tc.expected));
      }
    }
  }

  SECTION("accepts valid agent definitions") {
    for (auto const& tc : kSuccessCases) {
      DYNAMIC_SECTION(tc.name) {
        auto out = parse(tc.script);

        auto& def = require_one_agent_def(out.result);
        require_init_and_turn_blocks(def);
      }
    }
  }

  SECTION("move in init is an error") {
    auto out = parse(kMoveInInit);

    REQUIRE_FALSE(out.result.has_value());
    REQUIRE(std::holds_alternative<ParseErr>(out.result.error()));
    CHECK(std::get<ParseErr>(out.result.error()).m_Kind ==
          ParseErr::OUT_OF_TURN);

    REQUIRE(out.parser.m_AgentDefs.empty());
  }
}
