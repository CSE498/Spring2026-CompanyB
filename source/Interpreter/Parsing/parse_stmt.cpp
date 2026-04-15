#include "Interpreter/Parser.hpp"
#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"

namespace cse498 {

std::expected<std::unique_ptr<AST::Node>, InterpErr> Parser::parse_stmt() {
  using AgentLexer::IDs;

  switch (m_Lexer.Peek()) {
  case IDs::ID_KW_BREAK: // Use fallthrough to do OR
  case IDs::ID_KW_CONTINUE:
    return parse_loop_ctl();
  case IDs::ID_KW_WHILE:
    return parse_while();
  case IDs::ID_KW_IF:
    return parse_if();
  case IDs::ID_KW_MOVE:
    return parse_move();
  case IDs::ID_IDENTIFIER:
    return parse_expr();
  case IDs::ID_DELIM_SEMICLN: {
    auto res = m_Lexer.Use();
    return (!res.has_value()) ? std::unexpected(res.error()) : parse_stmt();
  };
  default:
    return std::unexpected(ParseErr(ParseErr::EXPECTED_STMT));
  }
}

std::expected<std::unique_ptr<AST::Node>, InterpErr>
Parser::parse_stmt_block() {
  /*
  ---
  <{> <STMT...> <}>
  ---
  */
  using AgentLexer::IDs;

  // Expect: {
  auto res = m_Lexer.UseIf(IDs::ID_DELIM_CLY_OPEN);
  if (!res)
    return std::unexpected(res.error());

  std::unique_ptr<AST::StmtBlock> node =
      std::make_unique<AST::StmtBlock>(res.value());

  while (!m_Lexer.Is(IDs::ID_DELIM_CLY_CLOSE)) {
    auto stmt = parse_expr();
    if (!stmt.has_value())
      return std::unexpected(stmt.error());

    node->add_node(std::move(stmt.value()));
  }

  // Expect: }
  res = m_Lexer.UseIf(IDs::ID_DELIM_CLY_CLOSE);
  if (!res)
    return std::unexpected(res.error());

  return node;
}

}; // namespace cse498
