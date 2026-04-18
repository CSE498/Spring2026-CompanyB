#include "Interpreter/Parser.hpp"
#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"
#include "Interpreter/lexer.hpp"
#include <memory>

namespace cse498 {

std::expected<std::unique_ptr<AST::Node>, InterpErr>
Parser::parse_stmt(ParseSetting setting) {
  using AgentLexer::IDs;

  std::expected<std::unique_ptr<AST::Node>, InterpErr> res;

  switch (m_Lexer.Peek()) {
  case IDs::ID_KW_BREAK: // Use fallthrough to do OR
  case IDs::ID_KW_CONTINUE:
    res = parse_loop_ctl();
    break;
  case IDs::ID_KW_WHILE:
    // Doesn't expect a semi after
    return parse_while();
  case IDs::ID_KW_IF:
    // Doesn't expect a semi after
    return parse_if();
  case IDs::ID_KW_MOVE:
    res = parse_move();
    break;
  case IDs::ID_KW_LET:
    res = parse_var_def();
    break;
  case IDs::ID_IDENTIFIER:
    res = parse_expr();
    break;
  case IDs::ID_DELIM_SEMICLN: {
    auto use_semi = m_Lexer.Use();
    if (!use_semi.has_value())
      return use_semi.error();
    return std::make_unique<AST::EmptyNode>(use_semi.value());
  };
  case IDs::ID__EOF_: {
    if (setting == ParseSetting::REQUIRED)
      return ParseErr(ParseErr::EXPECTED_STMT,
                      "Missing statement, unexpectedly reached EOF");
    return ParseErr(ParseErr::AT_EOF);
  }
  default:
    res = parse_expr();
  }

  if (!res.has_value())
    return res.error();

  // Consume remaining semicolon
  auto use_semi = m_Lexer.UseIf(IDs::ID_DELIM_SEMICLN);
  if (!use_semi.has_value())
    return use_semi.error();

  return std::move(res.value());
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
    return res.error();

  std::unique_ptr<AST::StmtBlock> node =
      std::make_unique<AST::StmtBlock>(res.value());

  while (!m_Lexer.Is(IDs::ID_DELIM_CLY_CLOSE)) {
    auto stmt = parse_stmt();
    if (!stmt.has_value())
      return stmt.error();

    node->add_node(std::move(stmt.value()));
  }

  // Expect: }
  res = m_Lexer.UseIf(IDs::ID_DELIM_CLY_CLOSE);
  if (!res)
    return res.error();

  return node;
}

}; // namespace cse498
