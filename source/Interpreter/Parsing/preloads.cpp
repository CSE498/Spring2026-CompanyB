#include <expected>
#include <tuple>

#include "FuncInfo.hpp"
#include "Interpreter/Evaluation/OpVisits.hpp"
#include "Interpreter/Parser.hpp"
#include "Interpreter/agentlang.hpp"
#include "Interpreter/errors.hpp"
#include "Interpreter/lexer.hpp"

using cse498::agentlang::Symbols::FuncSym;
using cse498::agentlang::Types::Type;
using cse498::agentlang::Types::TypeKind;
using cse498::AgentLexer::IDs;

namespace cse498 {

template <size_t idx, typename Head, typename... Tail>
std::expected<void, InterpErr> templ_recurse_typecheck(
    std::vector<Type> const &args) {
  // Fail early if types don't match
  if (!std::holds_alternative<Head>(args.at(idx)))
    return RuntimeErr(RuntimeErr::TYPE_MISMATCH,
                      std::format("Function argument type mismatch at position "
                                  "{}, expected '{}' but got '{}'",
                                  idx, TypeVariantToName(Head{}),
                                  TypeVariantToName(args.at(idx))));

  // Recurse only if Tail is length greater than 0
  if constexpr (sizeof...(Tail) > 0) {
    return templ_recurse_typecheck<idx + 1, Tail...>(args);
  } else {
    return {};
  }
}

template <typename... Ts>
std::expected<void, InterpErr> enforce_signature(
    std::vector<Type> const &input) {
  // Fail early if numbers don't match
  if (input.size() < sizeof...(Ts))
    return RuntimeErr(RuntimeErr::TOO_FEW_ARGS);
  else if (input.size() > sizeof...(Ts))
    return RuntimeErr(RuntimeErr::TOO_FEW_ARGS);

  // Now enforce argument types match what's defined
  TRY((templ_recurse_typecheck<0, Ts...>(input)));

  // Good to go
  return {};
}

/** @brief Register preloaded functions into the symbol table. Returns an
 * expected containing information only upon failure. New function preloads
 * should be added here.
 */
std::expected<void, InterpErr> Parser::preload_functions() {
  FuncSym preload_makepoint(
      [](std::vector<Type> &&args) -> std::expected<Type, InterpErr> {
        TRY((enforce_signature<int, int>(args)));

        return PointTy{std::get<int>(args.at(0)), std::get<int>(args.at(1))};
      });
  FuncSym preload_getx(
      [](std::vector<Type> &&args) -> std::expected<Type, InterpErr> {
        TRY((enforce_signature<PointTy>(args)));

        return static_cast<int>(std::get<PointTy>(args.at(0)).X());
      });
  FuncSym preload_gety(
      [](std::vector<Type> &&args) -> std::expected<Type, InterpErr> {
        TRY((enforce_signature<PointTy>(args)));

        return static_cast<int>(std::get<PointTy>(args.at(0)).Y());
      });

  TRY((m_Syms.AddSym("make_point", std::move(preload_makepoint))));
  TRY((m_Syms.AddSym("get_x", std::move(preload_getx))));
  TRY((m_Syms.AddSym("get_y", std::move(preload_gety))));

  return {};
}

/** @brief Register pre-loaded magic (dunder) values into the symbol table.
 * Returns an expected containing information only upon failure. New magic
 * preloads should be added here.
 */
std::expected<void, InterpErr> Parser::preload_magic_vals() {
  using Value = cse498::agentlang::Symbols::MagicSym::Value;

  // Always-present preloads
  TRY((m_Syms.AddSym("__spawn__", Value::SPAWN)));
  TRY((m_Syms.AddSym("__position__", Value::POSITION)));
  // TRY((m_Syms.AddSym("__destination__", Value::DESTINATION)));
  TRY((m_Syms.AddSym("__pOsition__", Value::DESTINATION)));

  // World-specific preloads
  switch (m_Env) {
    case Env::TRAFFIC: {
      TRY(m_Syms.AddSym("__facing__", Value::FACING));
      break;
    }
    case Env::INFECTION: {
      TRY(m_Syms.AddSym("__infected__", Value::INFECTED));
      TRY(m_Syms.AddSym("__susceptible__", Value::SUSCEPTIBLE));
      TRY(m_Syms.AddSym("__recovered__", Value::RECOVERED));
      break;
    }
  };

  return {};
}

};  // namespace cse498
