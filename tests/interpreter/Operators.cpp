#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>

#include "Interpreter/Evaluation/OpVisits.hpp"
#include "Interpreter/Parser.hpp"
#include "Interpreter/agentlang.hpp"
#include "Interpreter/errors.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"
#include "test-tools/matchers.hpp"
#include <expected>
#include <functional>
#include <unordered_map>
#include <variant>

struct EmptyDataclass {};

using namespace cse498;
using namespace agentlang::Types;
using DirectionType = agentlang::Types::Direction;
using TypeTuple = std::tuple<bool, int, double, str, Point, DirectionType>;

using EvalRet = std::expected<Type, InterpErr>;
using namespace matchers;

TEST_CASE("Unary operators", "[Interpreter]") {
  using AgentLexer::IDs;
  SECTION("minus, success") {
    EvalRet res = evaluate_unary(IDs::ID_OP_MINUS, Type{5});

    REQUIRE_THAT(res, (ExpNotErr() && VariantHas<int, CheckExp::SUCCESS>(-5)));
  }

  SECTION("minus, failure") {
    EvalRet res = evaluate_unary(IDs::ID_OP_MINUS, Type{"foo"});

    REQUIRE_FALSE(res.has_value());
    REQUIRE(std::holds_alternative<RuntimeErr>(res.error()));
    REQUIRE(std::get<RuntimeErr>(res.error()).m_Kind ==
            RuntimeErr::UNSUPPORTED_OP);

    // Just this once we'll make sure the msg is correct
    REQUIRE(std::get<RuntimeErr>(res.error()).m_Why ==
            "RuntimeErr: No operation defined for 'OP_MINUS' w/ operand type "
            "'str'");

    // REQUIRE_THAT(res, (ExpIsErr() &&
    //                    VariantHas<RuntimeErr, CheckExp::ERROR>(
    //                        RuntimeErr(RuntimeErr::UNSUPPORTED_OP,
    //                                   "RuntimeErr: No operation defined for "
    //                                   "'OP_MINUS' w/ operand type 'str'"))));
  }
  SECTION("logical not, success") {
    EvalRet res = evaluate_unary(IDs::ID_OP_LNOT, Type{false});

    REQUIRE(res.has_value());
    REQUIRE(std::holds_alternative<bool>(res.value()));
    REQUIRE(std::get<bool>(res.value()));
  }
  SECTION("logic not, failure") {
    EvalRet res = evaluate_unary(IDs::ID_OP_LNOT, Type{DirectionType::DOWN});

    REQUIRE_FALSE(res.has_value());
    REQUIRE(std::holds_alternative<RuntimeErr>(res.error()));
    REQUIRE(std::get<RuntimeErr>(res.error()).m_Kind ==
            RuntimeErr::UNSUPPORTED_OP);
  }
  SECTION("operator with no unary definitions") {
    EvalRet res = evaluate_unary(IDs::ID_CMP_EQ, Type{DirectionType::DOWN});
    REQUIRE_FALSE(res.has_value());
    REQUIRE(std::holds_alternative<RuntimeErr>(res.error()));
    REQUIRE(std::get<RuntimeErr>(res.error()).m_Kind ==
            RuntimeErr::UNSUPPORTED_OP);
  }
}

TEST_CASE("Binary operators", "[Interpreter]") {
  using AgentLexer::IDs;

  SECTION("add, int, int : success") {
    EvalRet res = evaluate_binary(IDs::ID_OP_ADD, Type{5}, Type{-5});

    REQUIRE(res.has_value());
    REQUIRE(std::holds_alternative<int>(res.value()));
    REQUIRE(std::get<int>(res.value()) == 0);
  }

  SECTION("add, double, double : success") {
    EvalRet res = evaluate_binary(IDs::ID_OP_ADD, Type{5.0}, Type{-5.0});

    REQUIRE(res.has_value());
    REQUIRE(std::holds_alternative<double>(res.value()));
    REQUIRE_THAT(std::get<double>(res.value()),
                 Catch::Matchers::WithinRel(0.0));
  }

  SECTION("add, int, double : success") {
    EvalRet res = evaluate_binary(IDs::ID_OP_ADD, Type{5}, Type{-5.0});

    REQUIRE(res.has_value());
    REQUIRE(std::holds_alternative<double>(res.value()));
    REQUIRE_THAT(std::get<double>(res.value()),
                 Catch::Matchers::WithinRel(0.0));
  }

  SECTION("add, str, str, : success") {
    EvalRet res = evaluate_binary(IDs::ID_OP_ADD, Type{"foo"}, Type{"bar"});

    REQUIRE(res.has_value());
    REQUIRE(std::holds_alternative<str>(res.value()));
    REQUIRE(std::get<str>(res.value()) == "foobar");
  }
}
