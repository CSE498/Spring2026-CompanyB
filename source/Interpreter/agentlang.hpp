#include "WorldPosition.hpp"
#include <string>
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

}; // namespace agentlang::Types

namespace agentlang::Operators {
struct Operator {};
}; // namespace agentlang::Operators

}; // namespace cse498
