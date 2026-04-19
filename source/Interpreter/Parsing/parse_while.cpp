#include "Interpreter/Parser.hpp"
#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"

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
    return res.error();

  Token while_token = res.value();

  // Expect: DELIM_PAREN_OPEN
  res = m_Lexer.UseIf(IDs::ID_DELIM_PAREN_OPEN);
  if (!res.has_value())
    return res.error();

  // Extract expr
  auto expr = parse_expr();
  if (!expr.has_value())
    return expr.error();

  // Expect: DELIM_PAREN_CLOSE
  res = m_Lexer.UseIf(IDs::ID_DELIM_PAREN_CLOSE);
  if (!res.has_value())
    return res.error();

  ++m_InLoop;
  m_Syms.PushSymbolScope();

  auto body = (m_Lexer.Is(IDs::ID_DELIM_CLY_OPEN))
                  ? parse_stmt_block()
                  : parse_stmt(ParseSetting::REQUIRED);

  --m_InLoop;
  m_Syms.PopSymbolScope();
  assert(m_InLoop >= 0);

  if (!body.has_value())
    return body.error();

  return std::make_unique<AST::StmtWhile>(while_token, std::move(expr.value()),
                                          std::move(body.value()));
}

std::expected<std::unique_ptr<AST::Node>, InterpErr> Parser::parse_loop_ctl() {
  using AgentLexer::IDs;
  using AgentLexer::Token;

  assert(m_InLoop >= 0);
  if (m_InLoop == 0)
    return ParseErr(ParseErr::OUT_OF_LOOP);

  auto res = m_Lexer.UseIf(IDs::ID_KW_BREAK, IDs::ID_KW_CONTINUE);
  if (!res.has_value())
    return res.error();

  return std::make_unique<AST::StmtLoopCtl>(res.value(),
                                            (res.value().id == IDs::ID_KW_BREAK)
                                                ? AST::StmtLoopCtl::BREAK
                                                : AST::StmtLoopCtl::CONTINUE);
}

}; // namespace cse498
