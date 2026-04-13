#include "Interpreter/ast.hpp"
#include "Parser.hpp"

namespace cse498 {

std::expected<std::unique_ptr<AST::Node>, ParseErr> Parser::parse_while() {}
  /*
  Assuming there's a curly-brace-enclosed body:
  ---
  <KW_WHILE> <DELIM_PAREN_OPEN> <EXPRESSION> <DELIM_PAREN_CLOSE> <STMT_BLOCK>
  ---
  */
  using AgentLexer::IDs;
  using AgentLexer::LexerErr;
  using AgentLexer::Token;

  // Expect: KW_WHILE
  res = m_Lexer.UseIf(IDs::KW_WHILE);
  if (!res) {
    // Figure out what to do with a LexerErr
    return std::unexpected(ParseErr(ParseErr::TODO, res.error().m_Msg));
  }
  // Imp: Symbol table interaction
  Token while_token = res.value();

  // Expect: DELIM_PAREN_OPEN
  res = m_Lexer.UseIf(IDs::DELIM_PAREN_OPEN);
  if (!res) {
    // Figure out what to do with a LexerErr
    return std::unexpected(ParseErr(ParseErr::TODO, res.error().m_Msg));
  }

  // Extract expr
  auto expr = parse_expr();
  if (!expr.has_value()) {
    return std::unexpected(expr.error());
  }

  // Expect: DELIM_PAREN_CLOSE
  res = m_Lexer.UseIf(IDs::DELIM_PAREN_CLOSE);
  if (!res) {
    // Figure out what to do with a LexerErr
    return std::unexpected(ParseErr(ParseErr::TODO, res.error().m_Msg));
  }

  // Extract stmt_block
  auto stmt_block = parse_stmt_block();
  if (!stmt_block.has_value()) {
    return std::unexpected(stmt_block.error());
  }

  m_InLoop = true;

  return std::make_unique<AST::StmtWhile>(while_token, std::move(expr.value()), std::move(stmt_block.value()));
}; // namespace cse498
