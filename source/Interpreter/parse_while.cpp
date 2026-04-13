#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"
#include "Parser.hpp"

namespace cse498 {

std::expected<std::unique_ptr<AST::Node>, InterpErr> Parser::parse_while() {
  /*
  Assuming there's a curly-brace-enclosed body:
  ---
  <KW_WHILE> <DELIM_PAREN_OPEN> <EXPRESSION> <DELIM_PAREN_CLOSE> <STMT_BLOCK>
  ---
  */
  using AgentLexer::IDs;
  using AgentLexer::Token;

  // Expect: KW_WHILE
  auto res = m_Lexer.UseIf(IDs::ID_KW_WHILE);
  if (!res.has_value())
    return std::unexpected(res.error());

  Token while_token = res.value();

  // Expect: DELIM_PAREN_OPEN
  res = m_Lexer.UseIf(IDs::ID_DELIM_PAREN_OPEN);
  if (!res.has_value())
    return std::unexpected(res.error());

  // Extract expr
  auto expr = parse_expr();
  if (!expr.has_value())
    return std::unexpected(expr.error());

  // Expect: DELIM_PAREN_CLOSE
  res = m_Lexer.UseIf(IDs::ID_DELIM_PAREN_CLOSE);
  if (!res.has_value())
    return std::unexpected(res.error());

  auto body =
      (m_Lexer.Is(IDs::ID_DELIM_CLY_OPEN)) ? parse_stmt_block() : parse_stmt();

  if (!body.has_value())
    return std::unexpected(body.error());

  return std::make_unique<AST::StmtWhile>(while_token, std::move(expr.value()),
                                          std::move(body.value()));
}
}; // namespace cse498
