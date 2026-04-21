#include "Interpreter/Evaluation/BoolConv.hpp"

#include <expected>

#include "Interpreter/agentlang.hpp"
#include "Interpreter/errors.hpp"

namespace cse498 {

std::expected<bool, InterpErr> evaluate_bool(Type t) {
  return std::visit(BoolConvVisitor{}, t);
}

};  // namespace cse498
