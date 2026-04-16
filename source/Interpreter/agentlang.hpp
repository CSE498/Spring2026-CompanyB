#pragma once

#include "Interpreter/errors.hpp"
#include "RobinHoodMap.hpp"
#include "core.hpp"
#include "core/WorldPosition.hpp"
#include "lexer.hpp"

#include <cstddef>
#include <functional>
#include <limits>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <variant>

namespace cse498 {

namespace agentlang::Types {

// Alias so that the internal "point" and the type "point" match
using Point = WorldPosition;
using str = std::string;

enum class Direction {
  LEFT,
  RIGHT,
  UP,
  DOWN,
};

// TODO : How are agents represented internally?
struct Car {};
struct Student {};
using Type =
    std::variant<bool, int, double, str, Point, Direction, Car, Student>;
using TypeTuple =
    std::tuple<bool, int, double, str, Point, Direction, Car, Student>;

// Concept satisfied if T is a valid agentlang type
template <typename T>
concept TypeKind = Concepts::IsOneOf<T, bool, int, double, str, Point,
                                     Direction, Car, Student>;

inline std::optional<Type> NameToType(const emplex::Token &type_tok) {
  static const std::unordered_map<std::string, Type> type_map = {
      {"bool", bool{}}, {"int", int{}},         {"double", double{}},
      {"str", str{}},   {"Point", Point{}},     {"Direction", Direction{}},
      {"Car", Car{}},   {"Student", Student{}},
  };

  auto it = type_map.find(type_tok.lexeme);
  if (it != type_map.end()) {
    return it->second;
  }
  return std::nullopt;
}
}; // namespace agentlang::Types

namespace agentlang::Operators {
struct OpInfo {
  enum class Assoc {
    LEFT,
    RIGHT,
    NEITHER,
  };

  int m_Prec;
  Assoc m_Assoc;

  /*
  Precedences:
   - 1 : member access (self.X)
   - 2 : Unary ops (lnot, minus)
   - 3 : Exp
   - 4 : Mult, div, rem
   - 5 : Add, sub
   - 6 : Comparisons (all)
   - 7 : Assignment
  */

  static std::expected<OpInfo, InterpErr>
  FromBinary(AgentLexer::Token const &token) {
    using AgentLexer::IDs;
    // Use fallthrough to evaluate as logical-or
    switch (token.id) {
    case IDs::ID_OP_MULT:
    case IDs::ID_OP_DIVIDE:
    case IDs::ID_OP_REM:
      return OpInfo{4, Assoc::LEFT};
    case IDs::ID_OP_MINUS:
    case IDs::ID_OP_ADD:
      return OpInfo{5, Assoc::LEFT};
    case IDs::ID_CMP_EQ:
    case IDs::ID_CMP_NEQ:
    case IDs::ID_CMP_LT:
    case IDs::ID_CMP_LEQ:
    case IDs::ID_CMP_GT:
    case IDs::ID_CMP_GEQ:
      return OpInfo{6, Assoc::NEITHER};
    case IDs::ID_OP_ASSIGN:
      return OpInfo{7, Assoc::RIGHT};
    default:
      return std::unexpected(ParseErr(
          ParseErr::INVALID_OP_TOKEN,
          std::format("Given token '{}' is not a valid binary operator",
                      AgentLexer::TokenName(token.id))));
    };
  }
  static std::expected<OpInfo, InterpErr>
  FromUnary(AgentLexer::Token const &token) {
    using AgentLexer::IDs;
    // Use fallthrough to evaluate as logical-or
    switch (token.id) {
    case IDs::ID_OP_MINUS:
    case IDs::ID_OP_LNOT:
      return OpInfo{2, Assoc::RIGHT};
    default:
      return std::unexpected(
          ParseErr(ParseErr::INVALID_OP_TOKEN,
                   std::format("Given token '{}' is not a valid unary operator",
                               AgentLexer::TokenName(token.id))));
    };
  }

  // The tokens are ints and the operators are tightly grouped
  // so we can just check a range
  static constexpr int MIN_OP = AgentLexer::IDs::ID_OP_ADD;
  static constexpr int MAX_OP = AgentLexer::IDs::ID_OP_MINUS;

  static bool IsOpToken(AgentLexer::Token const &token) {
    return ((token.id < MIN_OP) && (token.id > MAX_OP));
  }
};
}; // namespace agentlang::Operators

namespace agentlang::Symbols {

using agentlang::Types::Type;

struct SymInfo {
  std::string name;
  size_t line_def;
  Type type;
};

} // namespace agentlang::Symbols

}; // namespace cse498
