#include "Interpreter/agentlang.hpp"
#include "Interpreter/errors.hpp"
#include "RobinHoodMap.hpp"
#include "core.hpp"
#include <functional>
#include <type_traits>

template <typename... Ts> struct Overload : Ts... {
  using Ts::operator()...;
};

using namespace cse498::agentlang::Types;

namespace cse498 {

template <typename Dataclass> class OpVisits {
  // clang-format off
  static constexpr auto m_OpVisit_Un_GEQ = Overload{
    [](auto a) -> std::expected<Type, InterpErr>
    {return RuntimeErr(RuntimeErr::TYPE_MISMATCH);}
  };
  static constexpr auto m_OpVisit_Un_GT = Overload{
    [](auto a) -> std::expected<Type, InterpErr>
    {return RuntimeErr(RuntimeErr::TYPE_MISMATCH);}
  };
  static constexpr auto m_OpVisit_Un_LEQ = Overload{
    [](auto a) -> std::expected<Type, InterpErr>
    {return RuntimeErr(RuntimeErr::TYPE_MISMATCH);}
  };
  static constexpr auto m_OpVisit_Un_LT = Overload{
    [](auto a) -> std::expected<Type, InterpErr>
    {return RuntimeErr(RuntimeErr::TYPE_MISMATCH);}
  };
  static constexpr auto m_OpVisit_Un_NEQ = Overload{
    [](auto a) -> std::expected<Type, InterpErr>
    {return RuntimeErr(RuntimeErr::TYPE_MISMATCH);}
  };
  static constexpr auto m_OpVisit_Un_EQ = Overload{
    [](auto a) -> std::expected<Type, InterpErr>
    {return RuntimeErr(RuntimeErr::TYPE_MISMATCH);}
  };
  static constexpr auto m_OpVisit_Un_REM = Overload{
    [](auto a) -> std::expected<Type, InterpErr>
    {return RuntimeErr(RuntimeErr::TYPE_MISMATCH);}
  };
  static constexpr auto m_OpVisit_Un_ADD = Overload{
    [](auto a) -> std::expected<Type, InterpErr>
    {return RuntimeErr(RuntimeErr::TYPE_MISMATCH);}
  };
  static constexpr auto m_OpVisit_Un_DIVIDE = Overload{
    [](auto a) -> std::expected<Type, InterpErr>
    {return RuntimeErr(RuntimeErr::TYPE_MISMATCH);}
  };
  static constexpr auto m_OpVisit_Un_MULT = Overload{
    [](auto a) -> std::expected<Type, InterpErr>
    {return RuntimeErr(RuntimeErr::TYPE_MISMATCH);}
  };
  static constexpr auto m_OpVisit_Un_LNOT = Overload{
    [](bool a) -> std::expected<Type, InterpErr>
    {return !a;},
    [](auto a) -> std::expected<Type, InterpErr>
    {return RuntimeErr(RuntimeErr::TYPE_MISMATCH);}
  };
  static constexpr auto m_OpVisit_Un_MINUS = Overload{
    [](Concepts::IsOneOf<int, double> auto a) -> std::expected<Type, InterpErr>
    {return -a;},
    [](auto a) -> std::expected<Type, InterpErr>
    {return RuntimeErr(RuntimeErr::TYPE_MISMATCH);}
  };

  // Binary
  // :<=|
  static constexpr auto m_OpVisit_Bi_GEQ = Overload{
    [](auto a, auto b) -> std::expected<Type, InterpErr>
    requires requires(decltype(a) a, decltype(b) b){a >= b; }
    { return a >= b; },

    [](auto a, auto b) -> std::expected<Type, InterpErr>
    {return RuntimeErr(RuntimeErr::TYPE_MISMATCH);}
  };
  static constexpr auto m_OpVisit_Bi_GT = Overload{
    [](auto a, auto b) -> std::expected<Type, InterpErr>
    requires requires(decltype(a) a, decltype(b) b){a > b; }
    { return a > b; },

    [](auto a, auto b) -> std::expected<Type, InterpErr>
    {return RuntimeErr(RuntimeErr::TYPE_MISMATCH);}
  };
  static constexpr auto m_OpVisit_Bi_LEQ = Overload{
    [](auto a, auto b) -> std::expected<Type, InterpErr>
    requires requires(decltype(a) a, decltype(b) b){a <= b; }
    { return a <= b; },

    [](auto a, auto b) -> std::expected<Type, InterpErr>
    {return RuntimeErr(RuntimeErr::TYPE_MISMATCH);}
  };
  static constexpr auto m_OpVisit_Bi_LT = Overload{
    [](auto a, auto b) -> std::expected<Type, InterpErr>
    requires requires(decltype(a) a, decltype(b) b){a < b; }
    { return a < b; },

    [](auto a, auto b) -> std::expected<Type, InterpErr>
    {return RuntimeErr(RuntimeErr::TYPE_MISMATCH);}
  };
  static constexpr auto m_OpVisit_Bi_NEQ = Overload{
    [](auto a, auto b) -> std::expected<Type, InterpErr>
    requires requires(decltype(a) a, decltype(b) b){a != b; }
    { return a != b; },

    [](auto a, auto b){return RuntimeErr(RuntimeErr::TYPE_MISMATCH);}
  };
  static constexpr auto m_OpVisit_Bi_EQ = Overload{
    [](auto a, auto b) -> std::expected<Type, InterpErr>
    requires requires(decltype(a) a, decltype(b) b){a == b; }
    { return a == b; },

    [](auto a, auto b) -> std::expected<Type, InterpErr>
    {return RuntimeErr(RuntimeErr::TYPE_MISMATCH);}
  };
  static constexpr auto m_OpVisit_Bi_REM = Overload{
    [](auto a, auto b) -> std::expected<Type, InterpErr>
    requires requires(decltype(a) a, decltype(b) b){a % b; }
    { return a % b; },

    [](auto a, auto b) -> std::expected<Type, InterpErr>
    {return RuntimeErr(RuntimeErr::TYPE_MISMATCH);}
  };
  static constexpr auto m_OpVisit_Bi_ADD = Overload{
    [](auto a, auto b) -> std::expected<Type, InterpErr>
    requires requires(decltype(a) a, decltype(b) b){a + b; }
    { return a >= b; },

    [](Point a, Point b) -> std::expected<Type, InterpErr>
    { return Point(a.X() + b.X(), a.Y() + b.Y()); },

    [](auto a, auto b) -> std::expected<Type, InterpErr>
    {return RuntimeErr(RuntimeErr::TYPE_MISMATCH);}
  };
  static constexpr auto m_OpVisit_Bi_DIVIDE = Overload{
    [](auto a, auto b) -> std::expected<Type, InterpErr>
    requires requires(decltype(a) a, decltype(b) b){a / b; }
    { return a / b; },

    [](auto a, auto b){return RuntimeErr(RuntimeErr::TYPE_MISMATCH);}
  };
  static constexpr auto m_OpVisit_Bi_MULT = Overload{
    [](auto a, auto b) -> std::expected<Type, InterpErr>
    requires requires(decltype(a) a, decltype(b) b){a * b; }
    { return a * b; },

    [](auto a, auto b) -> std::expected<Type, InterpErr>
    {return RuntimeErr(RuntimeErr::TYPE_MISMATCH);}
  };
  static constexpr auto m_OpVisit_Bi_MINUS = Overload{
    [](auto a, auto b) -> std::expected<Type, InterpErr>
    requires requires(decltype(a) a, decltype(b) b){a - b; }
    { return a - b; },

    [](Point a, Point b) -> std::expected<Type, InterpErr>
    { return Point(a.X() - b.X(), a.Y() - b.Y()); },

    [](auto a, auto b) -> std::expected<Type, InterpErr>
    {return RuntimeErr(RuntimeErr::TYPE_MISMATCH);}
  };
  // clang-format on

  using OpFuncUnary = std::function<std::expected<Type, InterpErr>(Type)>;
  using OpFuncBinary =
      std::function<std::expected<Type, InterpErr>(Type, Type)>;
};

}; // namespace cse498
