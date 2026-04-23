#include "Interpreter/Parser.hpp"

#include <expected>
#include <memory>
#include <print>
#include <variant>
#include <vector>

#include "Interpreter/Evaluation/OpVisits.hpp"
#include "Interpreter/Lexing/lexer-gen.hpp"
#include "Interpreter/agentlang.hpp"
#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"
#include "Interpreter/lexer.hpp"
#include "Interpreter/macros.hpp"

namespace cse498 {

std::expected<std::vector<std::unique_ptr<AST::StmtAgentDef>>, InterpErr>
Parser::parse(std::istream &in) {
  using AgentLexer::IDs;
  m_AgentDefs.clear();

  TRY(m_Lexer.Tokenize(in));

  using Value = agentlang::Symbols::MagicSym::Value;

  m_Syms.PushSymbolScope();

  /*
  We first need to check that the first statement configures the world
  ---
  <KW_WORLD> <KW_TRAFFIC|KW_INFECTION>;
  ---
  */
  // Expect: <KW_WORLD>
  TRY(m_Lexer.UseIf(IDs::ID_KW_WORLD));

  // Expect: <KW_TRAFFIC|KW_INFECTION>
  TRY_DECL(world_id, m_Lexer.UseIf(IDs::ID_KW_INFECTION, IDs::ID_KW_TRAFFIC));

  switch (world_id.id) {
    case IDs::ID_KW_TRAFFIC: {
      m_Env = Env::TRAFFIC;
      break;
    }
    case IDs::ID_KW_INFECTION: {
      m_Env = Env::INFECTION;
      break;
    }
    default:
      return ParseErr(
          ParseErr::INVALID_WORLD,
          std::format("Token '{}' is not a valid world configuration",
                      AgentLexer::TokenName(world_id)));
  }

  // Expect: <;>
  TRY(m_Lexer.UseIf(IDs::ID_DELIM_SEMICLN));

  // Do function preloads
  TRY(preload_functions());
  TRY(preload_magic_vals());

  // New scope for global scope
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
};  // namespace cse498
