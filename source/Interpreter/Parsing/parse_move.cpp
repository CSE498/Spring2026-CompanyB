#include "Interpreter/Parser.hpp"
#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"

namespace cse498 {

std::expected<std::unique_ptr<AST::Node>, InterpErr> Parser::parse_move() {
  /*
  ---
  <KW_MOVE> <(> <EXPR> <)> <;>
  ---
  */
  using AgentLexer::IDs;
  using AgentLexer::Token;
  // A move is only legal if we're currently in a turn body
  if (!m_InTurn)
    return ParseErr(ParseErr::OUT_OF_TURN);

  // Expect: <KW_MOVE>
  auto res = m_Lexer.UseIf(IDs::ID_KW_MOVE);
  if (!res.has_value())
    return res.error();

  Token mv_token = res.value();

  // Expect: <(>
  res = m_Lexer.UseIf(IDs::ID_DELIM_PAREN_OPEN);
  if (!res.has_value())
    return res.error();

  // Expect: <EXPR>
  auto node_res = parse_expr();
  if (!node_res.has_value())
    return node_res.error();

  // Expect: <)>
  res = m_Lexer.UseIf(IDs::ID_DELIM_PAREN_CLOSE);
  if (!res.has_value())
    return res.error();

  // Expect: <;>
  res = m_Lexer.UseIf(IDs::ID_DELIM_SEMICLN);
  if (!res.has_value())
    return res.error();

  // Return built node
  return std::make_unique<AST::StmtAction>(mv_token,
                                           std::move(node_res.value()));
}

}; // namespace cse498
