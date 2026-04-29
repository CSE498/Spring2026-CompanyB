#include "Interpreter/Parser.hpp"
#include "Interpreter/agentlang.hpp"
#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"
#include "Interpreter/lexer.hpp"
#include "Interpreter/macros.hpp"

namespace cse498 {

/** @brief Parse and return an expected containing either the variable
 * definition parsed or an `InterpErr` describing the error which occured.
 *
 * Variables may shadow symbols only from higher scopes than the current one.
 */
std::expected<std::unique_ptr<AST::Node>, InterpErr> Parser::parse_var_def() {
  /*
  ---
  let <ID> : <TYPE> = <Expr>;
  ---
  */
  using AgentLexer::IDs;
  using AgentLexer::Token;
  using namespace agentlang::Types;

  // Expect: KW_LET
  std::expected<Token, InterpErr> res = m_Lexer.UseIf(IDs::ID_KW_LET);
  if (!res) return res.error();

  // Expect: ID
  res = m_Lexer.UseIf(IDs::ID_IDENTIFIER);
  if (!res) return res.error();

  // Imp: Symbol table interaction
  Token id_token = res.value();

  // Expect: DELIM_CLN
  res = m_Lexer.UseIf(IDs::ID_DELIM_CLN);
  if (!res) return res.error();

  // Expect: One of the types
  res = parse_type();
  if (!res) return res.error();

  // Imp: Symbol table interaction
  Token type_token = res.value();
  // auto sym_add_res = m_Syms.AddSym(id_token, type_token);
  auto sym_add_res =
      m_Syms.AddSym(id_token, NameToType(type_token).value_or(NullType{}));
  if (!sym_add_res) return sym_add_res.error();

  auto sym_retrieve_res = m_Syms.GetSym(sym_add_res.value());
  if (!sym_retrieve_res) return sym_retrieve_res.error();

  // Expect: OP_ASSIGN
  if (m_Lexer.Is(IDs::ID_OP_ASSIGN)) {
    res = m_Lexer.UseIf(IDs::ID_OP_ASSIGN);
    if (!res) return res.error();

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

};  // namespace cse498
