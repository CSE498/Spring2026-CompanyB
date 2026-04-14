#pragma once
#include "Interpreter/errors.hpp"
#include "agentlang.hpp"
#include "tools/RobinHoodMap.hpp"
#include <string>

namespace cse498 {
using namespace cse498::agentlang::Types;

template <typename... Ts> struct Overload : Ts... {
  using Ts::operator()...;
};

template <typename T>
concept Numeric = Concepts::IsOneOf<int, double>;

template <typename Dataclass> class Interpreter {
  using OpFuncUnary = std::function<std::expected<Type, InterpErr>(Type)>;
  using OpFuncBinary =
      std::function<std::expected<Type, InterpErr>(Type, Type)>;

  // --- Operator visits ---
  // clang-format off
  // Unary
  static auto m_OpVisit_Un_GEQ = Overload{
    [](auto a){return std::unexpected(RuntimeErr(RuntimeErr::TYPE_MISMATCH));}
  };
  static auto m_OpVisit_Un_GT = Overload{
    [](auto a){return std::unexpected(RuntimeErr(RuntimeErr::TYPE_MISMATCH));}
  };
  static auto m_OpVisit_Un_LEQ = Overload{
    [](auto a){return std::unexpected(RuntimeErr(RuntimeErr::TYPE_MISMATCH));}
  };
  static auto m_OpVisit_Un_LT = Overload{
    [](auto a){return std::unexpected(RuntimeErr(RuntimeErr::TYPE_MISMATCH));}
  };
  static auto m_OpVisit_Un_NEQ = Overload{
    [](auto a){return std::unexpected(RuntimeErr(RuntimeErr::TYPE_MISMATCH));}
  };
  static auto m_OpVisit_Un_EQ = Overload{
    [](auto a){return std::unexpected(RuntimeErr(RuntimeErr::TYPE_MISMATCH));}
  };
  static auto m_OpVisit_Un_REM = Overload{
    [](auto a){return std::unexpected(RuntimeErr(RuntimeErr::TYPE_MISMATCH));}
  };
  static auto m_OpVisit_Un_ADD = Overload{
    [](auto a){return std::unexpected(RuntimeErr(RuntimeErr::TYPE_MISMATCH));}
  };
  static auto m_OpVisit_Un_DIVIDE = Overload{
    [](auto a){return std::unexpected(RuntimeErr(RuntimeErr::TYPE_MISMATCH));}
  };
  static auto m_OpVisit_Un_MULT = Overload{
    [](auto a){return std::unexpected(RuntimeErr(RuntimeErr::TYPE_MISMATCH));}
  };
  static auto m_OpVisit_Un_LNOT = Overload{
    [](bool a){return !a;},
    [](auto a){return std::unexpected(RuntimeErr(RuntimeErr::TYPE_MISMATCH));}
  };
  static auto m_OpVisit_Un_MINUS = Overload{
    [](Numeric auto a){return -a;},
    [](auto a){return std::unexpected(RuntimeErr(RuntimeErr::TYPE_MISMATCH));}
  };

  // Binary
  static auto m_OpVisit_Bi_GEQ = Overload{
    [](Numeric auto a, Numeric auto b)
    { return a >= b; },

    [](auto a, auto b){return std::unexpected(RuntimeErr(RuntimeErr::TYPE_MISMATCH));}
  };
  static auto m_OpVisit_Bi_GT = Overload{
    [](Numeric auto a, Numeric auto b)
    { return a >= b; },

    [](auto a, auto b){return std::unexpected(RuntimeErr(RuntimeErr::TYPE_MISMATCH));}
  };
  static auto m_OpVisit_Bi_LEQ = Overload{
    [](Numeric auto a, Numeric auto b)
    { return a >= b; },

    [](auto a, auto b){return std::unexpected(RuntimeErr(RuntimeErr::TYPE_MISMATCH));}
  };
  static auto m_OpVisit_Bi_LT = Overload{
    [](Numeric auto a, Numeric auto b)
    { return a >= b; },

    [](auto a, auto b){return std::unexpected(RuntimeErr(RuntimeErr::TYPE_MISMATCH));}
  };
  static auto m_OpVisit_Bi_NEQ = Overload{
    [](Numeric auto a, Numeric auto b)
    { return a >= b; },

    [](auto a, auto b){return std::unexpected(RuntimeErr(RuntimeErr::TYPE_MISMATCH));}
  };
  static auto m_OpVisit_Bi_EQ = Overload{
    [](auto a, decltype(a) b)
    { return a == b; },

    [](auto a, auto b){return std::unexpected(RuntimeErr(RuntimeErr::TYPE_MISMATCH));}
  };
  static auto m_OpVisit_Bi_REM = Overload{
    [](int a, int b)
    { return a % b; },

    [](auto a, auto b){return std::unexpected(RuntimeErr(RuntimeErr::TYPE_MISMATCH));}
  };
  static auto m_OpVisit_Bi_ADD = Overload{
    [](Numeric auto a, Numeric auto b)
    { return a + b; },

    [](Point a, Point b)
    { return Point(a.X() + b.X(), a.Y() + b.Y()); },

    [](auto a, auto b){return std::unexpected(RuntimeErr(RuntimeErr::TYPE_MISMATCH));}
  };
  static auto m_OpVisit_Bi_DIVIDE = Overload{
    [](Numeric auto a, Numeric auto b)
    { return a / b; },

    [](auto a, auto b){return std::unexpected(RuntimeErr(RuntimeErr::TYPE_MISMATCH));}
  };
  static auto m_OpVisit_Bi_MULT = Overload{
    [](Numeric auto a, Numeric auto b)
    { return a - b; },

    [](auto a, auto b){return std::unexpected(RuntimeErr(RuntimeErr::TYPE_MISMATCH));}
  };
  static auto m_OpVisit_Bi_MINUS = Overload{
    [](Numeric auto a, Numeric auto b)
    { return a - b; },

    [](Point a, Point b)
    { return Point(a.X() - b.X(), a.Y() - b.Y()); },

    [](auto a, auto b){return std::unexpected(RuntimeErr(RuntimeErr::TYPE_MISMATCH));}
  };
  // clang-format on
  // -----------------------

  RobinHoodMap<int, OpFuncUnary> m_OpDispUnary{
      {emplex::Lexer::ID_CMP_GEQ, m_OpVisit_Un_GEQ},
      {emplex::Lexer::ID_CMP_GT, m_OpVisit_Un_GT},
      {emplex::Lexer::ID_CMP_LEQ, m_OpVisit_Un_LEQ},
      {emplex::Lexer::ID_CMP_LT, m_OpVisit_Un_LT},
      {emplex::Lexer::ID_CMP_NEQ, m_OpVisit_Un_NEQ},
      {emplex::Lexer::ID_CMP_EQ, m_OpVisit_Un_EQ},
      {emplex::Lexer::ID_OP_REM, m_OpVisit_Un_REM},
      {emplex::Lexer::ID_OP_ADD, m_OpVisit_Un_ADD},
      {emplex::Lexer::ID_OP_DIVIDE, m_OpVisit_Un_DIVIDE},
      {emplex::Lexer::ID_OP_MULT, m_OpVisit_Un_MULT},
      {emplex::Lexer::ID_OP_LNOT, m_OpVisit_Un_LNOT},
      {emplex::Lexer::ID_OP_MINUS, m_OpVisit_Un_MINUS},
  };
  RobinHoodMap<int, OpFuncBinary> m_OpDispBinary{
      {emplex::Lexer::ID_CMP_GEQ, m_OpVisit_Bi_GEQ},
      {emplex::Lexer::ID_CMP_GT, m_OpVisit_Bi_GT},
      {emplex::Lexer::ID_CMP_LEQ, m_OpVisit_Bi_LEQ},
      {emplex::Lexer::ID_CMP_LT, m_OpVisit_Bi_LT},
      {emplex::Lexer::ID_CMP_NEQ, m_OpVisit_Bi_NEQ},
      {emplex::Lexer::ID_CMP_EQ, m_OpVisit_Bi_EQ},
      {emplex::Lexer::ID_OP_REM, m_OpVisit_Bi_REM},
      {emplex::Lexer::ID_OP_ADD, m_OpVisit_Bi_ADD},
      {emplex::Lexer::ID_OP_DIVIDE, m_OpVisit_Bi_DIVIDE},
      {emplex::Lexer::ID_OP_MULT, m_OpVisit_Bi_MULT},
      {emplex::Lexer::ID_OP_MINUS, m_OpVisit_Bi_MINUS},
  };
};

}; // namespace cse498
