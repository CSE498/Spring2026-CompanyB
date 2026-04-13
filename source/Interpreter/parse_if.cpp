#include "Interpreter/ast.hpp"
#include "Parser.hpp"

namespace cse498 {

std::expected<std::unique_ptr<AST::Node>, ParseErr> Parser::parse_if() {}
  /*
  Assuming there's a curly-brace-enclosed body:
  ---
  <KW_IF> <DELIM_PAREN_OPEN> <EXPRESSION> <DELIM_PAREN_CLOSE> <STMT_BLOCK>
  ---
  */
  using AgentLexer::IDs;
  using AgentLexer::LexerErr;
  using AgentLexer::Token;

  Token else_if_token = nullptr;

  // Expect: KW_IF
  res = m_Lexer.UseIf(IDs::KW_IF);
  if (!res) {
    // Expect: KW_ELSE_IF
    res = m_Lexer.UseIf(IDs::KW_ELSE_IF);
    if (!res) {
        return std::unexpected(ParseErr(ParseErr::TODO, res.error().m_Msg));
    }

    // Imp: Symbol table interaction
    Token else_if_token = res.value();
  }
  // Imp: Symbol table interaction
  Token if_token = res.value();

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

  if (else_if_token) {
    // Make recursive call to parse_if()
    return parse_if();
  }

  // Expect: KW_ELSE
  res = m_Lexer.UseIf(IDs::KW_ELSE);
  if (!res) {
    // Return Regular if-block
    return std::make_unique<AST::StmtIf>(if_token, std::move(expr.value()), std::move(stmt_block.value()));
  }
  // Imp: Symbol table interaction
  Token else_token = res.value();

  // Extract stmt_block
  auto stmt_block = parse_stmt_block();
  if (!stmt_block.has_value()) {
    return std::unexpected(stmt_block.error());
  }
}; // namespace cse498
