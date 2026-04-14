#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"
#include "Parser.hpp"

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
    return std::unexpected(res.error());

  Token agent_token = res.value();

  // init
  res = m_Lexer.UseIf(IDs::ID_KW_INIT);
  if (!res.has_value())
    return std::unexpected(res.error());

  res = m_Lexer.UseIf(IDs::ID_DELIM_CLN);
  if (!res.has_value())
    return std::unexpected(res.error());

  m_InInit = true;
  auto init = (m_Lexer.Is(IDs::ID_DELIM_CLY_OPEN)) ? parse_stmt_block() : parse_stmt();
  m_InInit = false;
  if (!init.has_value())
    return std::unexpected(init.error());

  res = m_Lexer.UseIf(IDs::ID_DELIM_SEMICLN);
  if (!res.has_value())
    return std::unexpected(res.error());

  // turn
  res = m_Lexer.UseIf(IDs::ID_KW_TURN);
  if (!res.has_value())
    return std::unexpected(res.error());

  res = m_Lexer.UseIf(IDs::ID_DELIM_CLN);
  if (!res.has_value())
    return std::unexpected(res.error());

  m_InTurn = true;
  auto turn = (m_Lexer.Is(IDs::ID_DELIM_CLY_OPEN)) ? parse_stmt_block() : parse_stmt();
  m_InTurn = false;
  if (!turn.has_value())
    return std::unexpected(turn.error());

  res = m_Lexer.UseIf(IDs::ID_DELIM_SEMICLN);
  if (!res.has_value())
    return std::unexpected(res.error());

  // close
  res = m_Lexer.UseIf(IDs::ID_DELIM_CLY_CLOSE);
  if (!res.has_value())
    return std::unexpected(res.error());

  res = m_Lexer.UseIf(IDs::ID_DELIM_SEMICLN);
  if (!res.has_value())
    return std::unexpected(res.error());

  // sym is nullptr here because it's already held by the wrapping Assign node
  // in parse_var_def
  return std::make_unique<AST::StmtAgentDef>(agent_token, std::move(init.value()),
                                             std::move(turn.value()), nullptr);
}

}; // namespace cse498
