/**
 * @file ErrorTest.cpp
 * @brief Smoke tests verifying ToStr() does not crash for any error kind.
 */
#include <catch2/catch_test_macros.hpp>

#include "Interpreter/errors.hpp"

using namespace cse498;

TEST_CASE("RuntimeErr::ToStr does not crash for any kind", "[errors]") {
  for (int k = 0; k <= static_cast<int>(RuntimeErr::SPAWN_OUTSIDE_INIT); ++k) {
    InterpErr err = RuntimeErr(static_cast<RuntimeErr::Kind>(k));
    CHECK_FALSE(err.ToStr().empty());
  }
}

TEST_CASE("ParseErr::ToStr does not crash for any kind", "[errors]") {
  for (int k = 0; k <= static_cast<int>(ParseErr::ILLEGAL_NESTING); ++k) {
    InterpErr err = ParseErr(static_cast<ParseErr::Kind>(k));
    CHECK_FALSE(err.ToStr().empty());
  }
}

TEST_CASE("SymbolErr::ToStr does not crash for any kind", "[errors]") {
  for (int k = 0; k <= static_cast<int>(SymbolErr::NOT_FUNCTION); ++k) {
    InterpErr err = SymbolErr(static_cast<SymbolErr::Kind>(k));
    CHECK_FALSE(err.ToStr().empty());
  }
}

TEST_CASE("LexerErr::ToStr does not crash for any kind", "[errors]") {
  for (int k = 0; k <= static_cast<int>(LexerErr::UNEXP_TOKEN); ++k) {
    InterpErr err = LexerErr(static_cast<LexerErr::Kind>(k));
    CHECK_FALSE(err.ToStr().empty());
  }
}

TEST_CASE("ASTErr::ToStr does not crash for any kind", "[errors]") {
  for (int k = 0; k <= static_cast<int>(ASTErr::UNTYPED_NODE); ++k) {
    InterpErr err = ASTErr(static_cast<ASTErr::Kind>(k));
    CHECK_FALSE(err.ToStr().empty());
  }
}

TEST_CASE("LoopControlErr::ToStr does not crash for any kind", "[errors]") {
  for (int k = 0; k <= static_cast<int>(LoopControlErr::RETURN); ++k) {
    InterpErr err = LoopControlErr(static_cast<LoopControlErr::Kind>(k));
    CHECK_FALSE(err.ToStr().empty());
  }
}
