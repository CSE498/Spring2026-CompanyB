#include "Interpreter/Parser.hpp"
#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"

namespace cse498 {

std::expected<std::unique_ptr<AST::Node>, InterpErr> Parser::parse_if() {
  /*
  Assuming there's a curly-brace-enclosed body:
  ---
  <KW_IF> <DELIM_PAREN_OPEN> <EXPRESSION> <DELIM_PAREN_CLOSE> <STMT_BLOCK>
  ---
  */
  using AgentLexer::IDs;
  using AgentLexer::Token;

  // Expect: KW_IF | KW_ELSE_IF
  auto res = m_Lexer.UseIf(IDs::ID_KW_IF, IDs::ID_KW_ELSE_IF);
  if (!res.has_value())
    return res.error();

  Token if_token = res.value();

  // Expect: DELIM_PAREN_OPEN
  res = m_Lexer.UseIf(IDs::ID_DELIM_PAREN_OPEN);
  if (!res.has_value())
    return res.error();

  // Extract expr
  auto cond = parse_expr();
  if (!cond.has_value())
    return cond.error();

  // Expect: DELIM_PAREN_CLOSE
  res = m_Lexer.UseIf(IDs::ID_DELIM_PAREN_CLOSE);
  if (!res.has_value())
    return res.error();

  // Extract stmt_block
  auto t_body =
      (m_Lexer.Is(IDs::ID_DELIM_CLY_OPEN)) ? parse_stmt_block() : parse_stmt();

  if (!t_body.has_value())
    return t_body.error();

  // Construct the node up to now
  std::unique_ptr<AST::StmtIf> node = std::make_unique<AST::StmtIf>(
      if_token, std::move(cond.value()), std::move(t_body.value()));

  // Optionally handle an else, else-if, or nothing
  if (m_Lexer.Is(IDs::ID_KW_ELSE)) {
    auto f_body = (m_Lexer.Is(IDs::ID_DELIM_CLY_OPEN)) ? parse_stmt_block()
                                                       : parse_stmt();

    if (!f_body.has_value())
      return f_body.error();

    node->m_FBody = std::move(f_body.value());
  } else if (m_Lexer.Is(IDs::ID_KW_ELSE_IF)) {
    auto f_body = parse_if();

    if (!f_body.has_value())
      return f_body.error();

    node->m_FBody = std::move(f_body.value());
  }

  return node;
}

}; // namespace cse498
