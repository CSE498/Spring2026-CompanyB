#include "Interpreter/Parser.hpp"
#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"

namespace cse498 {

std::expected<std::unique_ptr<AST::Node>, InterpErr> Parser::parse_agent_def() {
  // clang-format off
  /*
  ---
  <{> <KW_INIT> <:> <STMT|STMT_BLOCK> <;> <KW_TURN> <:> <STMT|STMT_BLOCK> <;> <}> <;>
  ---
  */
  // clang-format on
  using AgentLexer::IDs;
  using AgentLexer::Token;

  auto res = m_Lexer.UseIf(IDs::ID_DELIM_CLY_OPEN);
  if (!res.has_value())
    return res.error();

  Token agent_token = res.value();

  // Expect: <KW_INIT>
  res = m_Lexer.UseIf(IDs::ID_KW_INIT);
  if (!res.has_value())
    return res.error();

  // Expect: <:>
  res = m_Lexer.UseIf(IDs::ID_DELIM_CLN);
  if (!res.has_value())
    return res.error();

  m_InInit = true;
  m_Syms.PushSymbolScope();
  // Expect: <STMT_BLOCK|STMT>
  auto init =
      (m_Lexer.Is(IDs::ID_DELIM_CLY_OPEN)) ? parse_stmt_block() : parse_stmt();
  m_InInit = false;
  if (!init.has_value())
    return init.error();

  // Expect: <;>
  res = m_Lexer.UseIf(IDs::ID_DELIM_SEMICLN);
  if (!res.has_value())
    return res.error();

  // Expect: <KW_TURN>
  res = m_Lexer.UseIf(IDs::ID_KW_TURN);
  if (!res.has_value())
    return res.error();

  // Expect: <:>
  res = m_Lexer.UseIf(IDs::ID_DELIM_CLN);
  if (!res.has_value())
    return res.error();

  m_InTurn = true;
  // Expect: <STMT_BLOCK|STMT>
  auto turn =
      (m_Lexer.Is(IDs::ID_DELIM_CLY_OPEN)) ? parse_stmt_block() : parse_stmt();
  m_InTurn = false;
  m_Syms.PopSymbolScope();
  if (!turn.has_value())
    return turn.error();

  // Expect: <;>
  res = m_Lexer.UseIf(IDs::ID_DELIM_SEMICLN);
  if (!res.has_value())
    return res.error();

  // Expect: <}>
  res = m_Lexer.UseIf(IDs::ID_DELIM_CLY_CLOSE);
  if (!res.has_value())
    return res.error();

  // Expect: <;>
  res = m_Lexer.UseIf(IDs::ID_DELIM_SEMICLN);
  if (!res.has_value())
    return res.error();

  return std::make_unique<AST::StmtAgentDef>(
      agent_token, std::move(init.value()), std::move(turn.value()));
}

}; // namespace cse498
