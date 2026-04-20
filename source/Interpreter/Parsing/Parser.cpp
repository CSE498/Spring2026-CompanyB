#include "Interpreter/Parser.hpp"
#include "Interpreter/Evaluation/OpVisits.hpp"
#include "Interpreter/Lexing/lexer-gen.hpp"
#include "Interpreter/agentlang.hpp"
#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"
#include "Interpreter/lexer.hpp"
#include "Interpreter/macros.hpp"
#include <expected>
#include <memory>
#include <variant>
#include <vector>

namespace cse498 {

std::expected<std::vector<std::unique_ptr<AST::StmtAgentDef>>, InterpErr>
Parser::parse(std::istream &in) {
  using AgentLexer::IDs;
  m_AgentDefs.clear();

  auto tokenize_res = m_Lexer.Tokenize(in);
  if (!tokenize_res.has_value())
    return tokenize_res.error();

  using Value = agentlang::Symbols::MagicSym::Value;

  m_Syms.PushSymbolScope();

  // Preload special function(s)
  FuncSym preload_addone(
      [](std::vector<Type> &&args) -> std::expected<Type, InterpErr> {
        if (args.size() < 1)
          return RuntimeErr(RuntimeErr::TOO_FEW_ARGS);
        else if (args.size() > 1)
          return RuntimeErr(RuntimeErr::TOO_MANY_ARGS);

        return evaluate_binary(IDs::ID_OP_ADD, args.at(0), 1);
      });

  FuncSym preload_makepoint(
      [](std::vector<Type> &&args) -> std::expected<Type, InterpErr> {
        // TODO : These checks can be pulled out
        if (args.size() < 2)
          return RuntimeErr(RuntimeErr::TOO_FEW_ARGS);
        else if (args.size() > 2)
          return RuntimeErr(RuntimeErr::TOO_MANY_ARGS);

        if (!std::holds_alternative<int>(args.at(0)) ||
            !std::holds_alternative<int>(args.at(1)))
          return RuntimeErr(RuntimeErr::TYPE_MISMATCH);

        return Point{std::get<int>(args.at(0)), std::get<int>(args.at(1))};
      });
  FuncSym preload_getx(
      [](std::vector<Type> &&args) -> std::expected<Type, InterpErr> {
        // TODO : These checks can be pulled out
        if (args.size() < 1)
          return RuntimeErr(RuntimeErr::TOO_FEW_ARGS);
        else if (args.size() > 1)
          return RuntimeErr(RuntimeErr::TOO_MANY_ARGS);

        if (!std::holds_alternative<Point>(args.at(0)))
          return RuntimeErr(RuntimeErr::TYPE_MISMATCH);

        return std::get<Point>(args.at(0)).X();
      });
  FuncSym preload_gety(
      [](std::vector<Type> &&args) -> std::expected<Type, InterpErr> {
        // TODO : These checks can be pulled out
        if (args.size() < 1)
          return RuntimeErr(RuntimeErr::TOO_FEW_ARGS);
        else if (args.size() > 1)
          return RuntimeErr(RuntimeErr::TOO_MANY_ARGS);

        if (!std::holds_alternative<Point>(args.at(0)))
          return RuntimeErr(RuntimeErr::TYPE_MISMATCH);

        return std::get<Point>(args.at(0)).Y();
      });

  TRY(m_Syms.AddSym("preload_addone", std::move(preload_addone)));
  TRY(m_Syms.AddSym("make_point", std::move(preload_makepoint)));
  TRY(m_Syms.AddSym("get_x", std::move(preload_getx)));
  TRY(m_Syms.AddSym("get_y", std::move(preload_gety)));

  // Put in the universal magic vals
  TRY((m_Syms.AddSym("__destination__", Value::DESTINATION)));
  TRY((m_Syms.AddSym("__spawn__", Value::SPAWN)));
  TRY((m_Syms.AddSym("__position__", Value::POSITION)));

  /*
  We first need to check that the first statement configures the world
  ---
  <KW_WORLD> <KW_TRAFFIC|KW_INFECTION>;
  ---
  */
  // Expect: <KW_WORLD>
  auto token_res = m_Lexer.UseIf(IDs::ID_KW_WORLD);
  if (!token_res.has_value())
    return token_res.error();

  // Expect: <KW_TRAFFIC|KW_INFECTION>
  token_res = m_Lexer.UseIf(IDs::ID_KW_INFECTION, IDs::ID_KW_TRAFFIC);
  if (!token_res.has_value())
    return token_res.error();

  switch (token_res.value()) {
  case IDs::ID_KW_TRAFFIC: {
    m_Env = Env::TRAFFIC;
    // Put in the traffic-related magic vals
    TRY(m_Syms.AddSym("__facing__", Value::FACING));
    break;
  }
  case IDs::ID_KW_INFECTION: {
    m_Env = Env::INFECTION;
    // Put in the infection-related magic vals
    TRY(m_Syms.AddSym("__infected__", Value::INFECTED));
    TRY(m_Syms.AddSym("__susceptible__", Value::SUSCEPTIBLE));
    TRY(m_Syms.AddSym("__recovered__", Value::RECOVERED));
    break;
  }
  default:
    return ParseErr(ParseErr::INVALID_WORLD,
                    std::format("Token '{}' is not a valid world configuration",
                                AgentLexer::TokenName(token_res.value())));
  }

  // Expect: <;>
  token_res = m_Lexer.UseIf(IDs::ID_DELIM_SEMICLN);
  if (!token_res.has_value())
    return token_res.error();

  m_Syms.PushSymbolScope();
  // Now we parse every remaining statement
  while (m_Lexer.Any()) {
    auto stmt_res = parse_stmt();
    if (!stmt_res.has_value()) {
      return stmt_res.error();
    }

    m_Nodes.emplace_back(std::move(stmt_res.value()));
  }
  m_Syms.PopSymbolScope();

  // Now we'll finalize all nodes
  for (auto &node : m_Nodes) {
    TRY(node->Finalize(m_Syms));
  }

  for (auto &node : m_AgentDefs) {
    TRY(node->Finalize(m_Syms));
  }

  return std::move(m_AgentDefs);
}

std::expected<AgentLexer::Token, InterpErr> Parser::parse_type() {
  using AgentLexer::IDs;
  // Check for world-type mismatch
  if (m_Lexer.Is(IDs::ID_KW_CAR) && (m_Env == Env::INFECTION))
    return ParseErr(ParseErr::WORLD_MISMATCH,
                    "Cannot instantiate car in infection world");
  else if (m_Lexer.Is(IDs::ID_KW_STUDENT) && (m_Env == Env::TRAFFIC))
    return ParseErr(ParseErr::WORLD_MISMATCH,
                    "Cannot instantiate student in traffic world");
  return m_Lexer.UseIf(IDs::ID_KW_BOOL, IDs::ID_KW_INT, IDs::ID_KW_DOUBLE,
                       IDs::ID_KW_STR, IDs::ID_KW_POINT, IDs::ID_KW_DIRECTION_T,
                       IDs::ID_KW_CAR, IDs::ID_KW_STUDENT);
}
}; // namespace cse498
