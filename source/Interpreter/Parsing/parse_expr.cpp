#include "Interpreter/Parser.hpp"
#include "Interpreter/agentlang.hpp"
#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"
#include <string>

namespace cse498 {

std::expected<std::unique_ptr<AST::Node>, InterpErr>
Parser::parse_expr(int prec) {
  using agentlang::Operators::OpInfo;
  using AgentLexer::IDs;
  using AgentLexer::Token;

  // Parse expr time >:)

  if (prec <= 0)
    return parse_term();

  auto left = parse_expr(prec - 1);
  if (!left.has_value())
    return left.error();

  auto cur_opinfo = OpInfo::FromBinary(m_Lexer.Peek());
  while (cur_opinfo.has_value() && (cur_opinfo.value().m_Prec == prec)) {
    Token cur_token = m_Lexer.Use().value();

    int right_prec =
        (cur_opinfo.value().m_Assoc == OpInfo::Assoc::RIGHT) ? prec : prec - 1;

    auto right = parse_expr(right_prec);
    if (!right.has_value())
      return right.error();

    std::unique_ptr<AST::ExprBinary> node = std::make_unique<AST::ExprBinary>(
        cur_token, std::move(left.value()), std::move(right.value()));

    if (cur_opinfo.value().m_Assoc == OpInfo::Assoc::LEFT)
      left = std::move(node);
    else
      return node;
  }
  if (!cur_opinfo.has_value())
    return cur_opinfo.error();

  return left;
}

std::expected<std::unique_ptr<AST::Node>, InterpErr> Parser::parse_term() {
  using namespace agentlang::Types;
  using agentlang::Operators::OpInfo;
  using AgentLexer::IDs;
  using AgentLexer::Token;

  auto token_res = m_Lexer.Use();
  if (!token_res.has_value())
    return token_res.error();

  Token const &next_token = token_res.value();

  switch (next_token.id) {
  case IDs::ID_IDENTIFIER: {
    auto sym = m_Syms.GetSym(next_token.lexeme);
    if (!sym.has_value())
      return sym.error();
    return std::make_unique<AST::ValVariable>(next_token, sym.value());
  }
  case IDs::ID_LITERAL_DIR: {
    Direction dir;
    if (next_token.lexeme == "left")
      dir = Direction::LEFT;
    else if (next_token.lexeme == "right")
      dir = Direction::RIGHT;
    else if (next_token.lexeme == "up")
      dir = Direction::UP;
    else if (next_token.lexeme == "down")
      dir = Direction::DOWN;
    else
      return ParseErr(ParseErr::INVALID_LITERAL);

    return std::make_unique<AST::ValLiteral>(next_token, dir);
  }
  case IDs::ID_LITERAL_DBL: {
    return std::make_unique<AST::ValLiteral>(next_token,
                                             std::stod(next_token.lexeme));
  }
  case IDs::ID_LITERAL_INT: {
    return std::make_unique<AST::ValLiteral>(next_token,
                                             std::stoi(next_token.lexeme));
  }
  case IDs::ID_LITERAL_STR: {
    return std::make_unique<AST::ValLiteral>(next_token, next_token.lexeme);
  }
  default:
    return ParseErr(ParseErr::INVALID_TERM);
  }
}

}; // namespace cse498
