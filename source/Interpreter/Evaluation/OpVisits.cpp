#include "Interpreter/Evaluation/OpVisits.hpp"

#include <expected>

#include "Interpreter/agentlang.hpp"
#include "Interpreter/errors.hpp"
#include "core.hpp"

namespace cse498 {

std::expected<Type, InterpErr> evaluate_unary(int id, Type left) {
  return std::visit(OpVisitor{id}, left);
}

std::expected<Type, InterpErr> evaluate_binary(int id, Type left, Type right) {
  return std::visit(OpVisitor{id}, left, right);
}
std::expected<Type, InterpErr> evaluate_unary(AgentLexer::Token const& token,
                                              Type left) {
  return std::visit(OpVisitor{token.id}, left);
}
std::expected<Type, InterpErr> evaluate_binary(AgentLexer::Token const& token,
                                               Type left, Type right) {
  return std::visit(OpVisitor{token.id}, left, right);
}

std::expected<bool, InterpErr> evaluate_bool(Type t) {
  return std::visit(BoolConvVisitor{}, t);
}

};  // namespace cse498
