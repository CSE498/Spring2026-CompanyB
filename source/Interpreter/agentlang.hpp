#pragma once

#include "core/WorldPosition.hpp"
#include "lexer.hpp"

#include <optional>
#include <string>
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
struct Operator {};
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
