#pragma once

#include "Interpreter/agentlang.hpp"
#include "Interpreter/errors.hpp"
#include "Interpreter/macros.hpp"

using namespace cse498::agentlang::Types;

namespace cse498 {
std::expected<bool, InterpErr> evaluate_bool(Type);

struct BoolConvVisitor {
  template <TypeKind T>
    requires std::is_convertible_v<T, bool>
  std::expected<bool, InterpErr> operator()(T t) {
    return static_cast<bool>(t);
  }

  template <TypeKind T> std::expected<bool, InterpErr> operator()(T) {
    return RuntimeErr(RuntimeErr::NOT_BOOL_CONV);
  }
};

}; // namespace cse498
