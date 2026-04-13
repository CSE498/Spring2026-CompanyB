#include "Interpreter/ast.hpp"
#include "Interpreter/lexer.hpp"
#include "Parser.hpp"

namespace cse498 {

std::expected<std::unique_ptr<AST::Node>, ParseErr> Parser::parse_var_def() {
  /*
  ---
  let <ID> : <TYPE> = <Expr>;
  ---
  */
  using AgentLexer::IDs;
  using AgentLexer::LexerErr;
  using AgentLexer::Token;

  // Expect: KW_LET
  std::expected<Token, LexerErr> res = m_Lexer.UseIf(IDs::ID_KW_LET);
  if (!res) {
    // Figure out what to do with a LexerErr
    return std::unexpected(ParseErr(ParseErr::TODO, res.error().m_Msg));
  }

  // Expect: ID
  res = m_Lexer.UseIf(IDs::ID_IDENTIFIER);
  if (!res) {
    // Figure out what to do with a LexerErr
    return std::unexpected(ParseErr(ParseErr::TODO, res.error().m_Msg));
  }
  // Imp: Symbol table interaction
  Token id_token = res.value();

  // Expect: DELIM_CLN
  res = m_Lexer.UseIf(IDs::ID_DELIM_CLN);
  if (!res) {
    // Figure out what to do with a LexerErr
    return std::unexpected(ParseErr(ParseErr::TODO, res.error().m_Msg));
  }

  // Expect: One of the types
  res = parse_type();
  if (!res) {
    // Figure out what to do with a LexerErr
    return std::unexpected(ParseErr(ParseErr::TODO, res.error().m_Msg));
  }
  // Imp: Symbol table interaction
  Token type_token = res.value();
  auto sym_add_res = m_Syms.AddSym(id_token, type_token);
  if (!sym_add_res.has_value()) {
    // Figure out what to do with a SymbolError
    return std::unexpected(ParseErr(ParseErr::TODO, "SymbolError"));
  }
  auto sym_retrieve_res = m_Syms.GetSym(sym_add_res.value());
  if (!sym_retrieve_res.has_value()) {
    // Figure out what to do with a SymbolError
    return std::unexpected(ParseErr(ParseErr::TODO, "SymbolError"));
  }

  // Can be done here if we see a semicolon, otherwise we should see OP_ASSIGN
  if (m_Lexer.Is(IDs::ID_DELIM_SEMICLN))
    return std::make_unique<AST::EmptyNode>(m_Lexer.Use().value());

  // Otherwise we need an assign node
  // Expect: OP_ASSIGN
  res = m_Lexer.UseIf(IDs::ID_OP_ASSIGN);
  if (!res) {
    // Figure out what to do with a LexerErr
    return std::unexpected(ParseErr(ParseErr::TODO, res.error().m_Msg));
  }
  Token assign_token = res.value();

  // We'll go ahead and construct the binaryexpr node now
  auto expr = parse_expr();
  if (!expr.has_value()) {
    return std::unexpected(expr.error());
  }

  return std::make_unique<AST::Assign>(assign_token, sym_retrieve_res.value(),
                                       std::move(expr.value()));
}

}; // namespace cse498
