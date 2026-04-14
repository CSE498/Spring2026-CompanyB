#pragma once

#include "Interpreter/errors.hpp"
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
  int m_Prec;
  bool m_RecurseLeft;
  bool m_RecurseRight;

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

  static std::expected<OpInfo, InterpErr> FromBinary(AgentLexer::Token token) {
    using AgentLexer::IDs;
    // Use fallthrough to evaluate as logical-or
    switch (token.id) {
    case IDs::ID_OP_DOT:
      return OpInfo{1, true, false};
    case IDs::ID_OP_EXP:
      return OpInfo{3, false, true};
    case IDs::ID_OP_MULT:
    case IDs::ID_OP_DIVIDE:
    case IDs::ID_OP_REM:
      return OpInfo{4, true, false};
    case IDs::ID_OP_MINUS:
    case IDs::ID_OP_ADD:
      return OpInfo{5, true, false};
    case IDs::ID_CMP_EQ:
    case IDs::ID_CMP_NEQ:
    case IDs::ID_CMP_LT:
    case IDs::ID_CMP_LEQ:
    case IDs::ID_CMP_GT:
    case IDs::ID_CMP_GEQ:
      return OpInfo{6, false, false};
    case IDs::ID_OP_ASSIGN:
      return OpInfo{7, false, true};
    default:
      return std::unexpected(ParseErr(
          ParseErr::INVALID_OP_TOKEN,
          std::format("Given token '{}' is not a valid binary operator",
                      AgentLexer::TokenName(token.id))));
    };
  }
  static std::expected<OpInfo, InterpErr> FromUnary(AgentLexer::Token token) {
    using AgentLexer::IDs;
    // Use fallthrough to evaluate as logical-or
    switch (token.id) {
    case IDs::ID_OP_MINUS:
    case IDs::ID_OP_LNOT:
      return OpInfo{2, false, true};
    default:
      return std::unexpected(
          ParseErr(ParseErr::INVALID_OP_TOKEN,
                   std::format("Given token '{}' is not a valid unary operator",
                               AgentLexer::TokenName(token.id))));
    };
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
