#pragma once
#include <concepts>
#include <functional>
#include <type_traits>

#include "Interpreter/agentlang.hpp"
#include "Interpreter/errors.hpp"
#include "Interpreter/macros.hpp"
#include "RobinHoodMap.hpp"
#include "core.hpp"

using namespace cse498::agentlang::Types;

namespace cse498 {

std::expected<Type, InterpErr> evaluate_unary(AgentLexer::Token const &, Type);
std::expected<Type, InterpErr> evaluate_binary(AgentLexer::Token const &, Type,
                                               Type);
std::expected<Type, InterpErr> evaluate_unary(int id, Type);
std::expected<Type, InterpErr> evaluate_binary(int id, Type, Type);

struct OpVisitor {
  int token_id;

  template <TypeKind Left> std::expected<Type, InterpErr> operator()(Left l) {
    using AgentLexer::IDs;
    switch (token_id) {
    case IDs::ID_OP_LNOT: {
      STATIC_CHECKED_OP(!l);
    }
    case IDs::ID_OP_MINUS: {
      STATIC_CHECKED_OP(-l);
    }
    };
    // If we leave the switch, there is no valid operation in this context
    return RuntimeErr(
        RuntimeErr::UNSUPPORTED_OP,
        std::format("No operation defined for '{}' w/ operand type '{}'",
                    AgentLexer::TokenName(token_id), TypeToName<Left>()));
  }

  template <TypeKind Left, TypeKind Right>
  std::expected<Type, InterpErr> operator()(Left l, Right r) {
    using AgentLexer::IDs;
    switch (token_id) {
    case IDs::ID_CMP_GEQ: {
      STATIC_CHECKED_OP(l >= r);
    }
    case IDs::ID_CMP_GT: {
      STATIC_CHECKED_OP(l > r);
    }
    case IDs::ID_CMP_LEQ: {
      STATIC_CHECKED_OP(l <= r);
    }
    case IDs::ID_CMP_LT: {
      STATIC_CHECKED_OP(l < r);
    }
    case IDs::ID_CMP_NEQ: {
      STATIC_CHECKED_OP(l != r);
    }
    case IDs::ID_CMP_EQ: {
      STATIC_CHECKED_OP(l == r);
    }
    case IDs::ID_OP_REM: {
      STATIC_CHECKED_OP(l % r);
    }
    case IDs::ID_OP_ADD: {
      STATIC_CHECKED_OP(l + r);
      STATIC_CHECKED_OP(PointTy(l.X() + r.X(), l.Y() + r.Y()));
    }
    case IDs::ID_OP_MINUS: {
      STATIC_CHECKED_OP(l - r);
      STATIC_CHECKED_OP(PointTy(l.X() - r.X(), l.Y() - r.Y()));
    }
    case IDs::ID_OP_DIVIDE: {
      STATIC_CHECKED_OP(l / r);
    }
    case IDs::ID_OP_MULT: {
      STATIC_CHECKED_OP(l * r);
    }
    };
    // If we leave the switch, there is no valid operation in this context
    return RuntimeErr(
        RuntimeErr::UNSUPPORTED_OP,
        std::format(
            "No operation defined for '{}' w/ operand types '{}' and '{}'",
            AgentLexer::TokenName(token_id), TypeToName<Left>(),
            TypeToName<Right>()));
  }
};

}; // namespace cse498
