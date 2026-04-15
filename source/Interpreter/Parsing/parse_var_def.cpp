#include "Interpreter/Parser.hpp"
#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"
#include "Interpreter/lexer.hpp"

namespace cse498 {

std::expected<std::unique_ptr<AST::Node>, InterpErr> Parser::parse_var_def() {
  /*
  ---
  let <ID> : <TYPE> = <Expr>;
  ---
  */
  using AgentLexer::IDs;
  using AgentLexer::Token;

  // Expect: KW_LET
  std::expected<Token, InterpErr> res = m_Lexer.UseIf(IDs::ID_KW_LET);
  if (!res)
    return res.error();

  // Expect: ID
  res = m_Lexer.UseIf(IDs::ID_IDENTIFIER);
  if (!res)
    return res.error();

  // Imp: Symbol table interaction
  Token id_token = res.value();

  // Expect: DELIM_CLN
  res = m_Lexer.UseIf(IDs::ID_DELIM_CLN);
  if (!res)
    return res.error();

  // Expect: One of the types
  res = parse_type();
  if (!res)
    return res.error();

  // Imp: Symbol table interaction
  Token type_token = res.value();
  auto sym_add_res = m_Syms.AddSym(id_token, type_token);
  if (!res)
    return res.error();

  auto sym_retrieve_res = m_Syms.GetSym(sym_add_res.value());
  if (!res)
    return res.error();

  // Can be done here if we see a semicolon, otherwise we should see OP_ASSIGN
  if (m_Lexer.Is(IDs::ID_DELIM_SEMICLN))
    return std::make_unique<AST::EmptyNode>(m_Lexer.Use().value());

  // Otherwise we need an assign node
  // Expect: OP_ASSIGN
  if (m_Lexer.Is(IDs::ID_OP_ASSIGN)) {
    res = m_Lexer.UseIf(IDs::ID_OP_ASSIGN);
    if (!res)
      return res.error();

    Token assign_token = res.value();
    auto expr = parse_expr();
    if (!expr.has_value()) {
      return expr.error();
    }

    return std::make_unique<AST::Assign>(assign_token, sym_retrieve_res.value(),
                                         std::move(expr.value()));
  } else {
    auto expr = parse_agent_def();
    if (!expr.has_value()) {
      return expr.error();
    }

    return std::make_unique<AST::Assign>(type_token, sym_retrieve_res.value(),
                                         std::move(expr.value()));
  }
}

}; // namespace cse498
