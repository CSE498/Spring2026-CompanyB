#include <memory>
#include <string>
#include <variant>

#include "Interpreter/Parser.hpp"
#include "Interpreter/SymbolTable.hpp"
#include "Interpreter/agentlang.hpp"
#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"
#include "Interpreter/macros.hpp"

namespace cse498 {

std::expected<std::unique_ptr<AST::Node>, InterpErr>
Parser::parse_expr_expect_semicln(int prec) {
  // Parse an expression
  auto ret = parse_expr(prec);
  if (!ret.has_value()) return ret.error();

  auto semi = m_Lexer.UseIf(AgentLexer::IDs::ID_DELIM_SEMICLN);
  if (!semi.has_value()) return semi.error();

  return std::move(ret.value());
}

std::expected<std::unique_ptr<AST::Node>, InterpErr> Parser::parse_expr(
    int prec) {
  using agentlang::Operators::OpInfo;
  using AgentLexer::IDs;
  using AgentLexer::Token;

  // Parse expr time >:)

  if (prec <= 0) return parse_term();

  auto left = parse_expr(prec - 1);
  if (!left.has_value()) return left.error();

  auto cur_opinfo = OpInfo::FromBinary(m_Lexer.Peek());
  while (cur_opinfo.has_value() && (cur_opinfo.value().m_Prec == prec)) {
    Token cur_token = m_Lexer.Use().value();

    int right_prec =
        (cur_opinfo.value().m_Assoc == OpInfo::Assoc::RIGHT) ? prec : prec - 1;

    auto right = parse_expr(right_prec);
    if (!right.has_value()) return right.error();

    std::unique_ptr<AST::Node> node;
    if (cur_token.id == IDs::ID_OP_ASSIGN) {
      // Left side must be a variable or a magic val
      AST::ValVariable *left_as_var;

      if ((left_as_var =
               dynamic_cast<AST::ValVariable *>(left.value().get()))) {
        node = std::make_unique<AST::Assign>(cur_token, left_as_var->m_Symbol,
                                             std::move(right.value()));
      } else {
        return ParseErr(
            ParseErr::ILLEGAL_ASSIGNMENT,
            "Left side of assignment must be a symbol or magic value");
      }

    } else {
      node = std::make_unique<AST::ExprBinary>(
          cur_token, std::move(left.value()), std::move(right.value()));
    }

    assert(node != nullptr);

    if (cur_opinfo.value().m_Assoc == OpInfo::Assoc::LEFT)
      left = std::move(node);
    else
      return node;

    cur_opinfo = OpInfo::FromBinary(m_Lexer.Peek());
  }

  return left;
}

std::expected<std::unique_ptr<AST::Node>, InterpErr> Parser::parse_term() {
  using agentlang::Operators::OpInfo;
  using agentlang::Types::Dir;
  using AgentLexer::IDs;
  using AgentLexer::Token;

  auto token_res = m_Lexer.Use();
  if (!token_res.has_value()) return token_res.error();

  Token const &next_token = token_res.value();

  // Check and try unary op
  if (OpInfo::IsOpToken(next_token)) {
    auto expr = parse_expr();
    if (!expr.has_value()) return expr.error();

    return std::make_unique<AST::ExprUnary>(next_token,
                                            std::move(expr.value()));
  }

  switch (next_token.id) {
    case IDs::ID_MAGIC_VAL: {
      TRY_DECL(sym, m_Syms.GetSym(next_token.lexeme));
      return std::make_unique<AST::ValVariable>(next_token, sym);
    }
    case IDs::ID_IDENTIFIER: {
      TRY_DECL(sym, m_Syms.GetSym(next_token.lexeme));
      if (std::holds_alternative<FuncSym>(sym->sym)) {
        // Function parse expects to start w/ ID, so rewind one token
        m_Lexer.Rewind();
        return parse_func_call();
      }

      return std::make_unique<AST::ValVariable>(next_token, sym);
    }
    case IDs::ID_LITERAL_BOOL: {
      bool b;
      if (next_token.lexeme == "true") {
        b = true;
      } else if (next_token.lexeme == "false") {
        b = false;
      } else {
        return ParseErr(
            ParseErr::INVALID_LITERAL,
            std::format("Invalid boolean literal '{}'", next_token.lexeme));
      }
      return std::make_unique<AST::ValLiteral>(next_token, b);
    }
    case IDs::ID_LITERAL_DIR: {
      Dir dir;
      if (next_token.lexeme == "left")
        dir = Dir::LEFT;
      else if (next_token.lexeme == "right")
        dir = Dir::RIGHT;
      else if (next_token.lexeme == "up")
        dir = Dir::UP;
      else if (next_token.lexeme == "down")
        dir = Dir::DOWN;
      else
        return ParseErr(
            ParseErr::INVALID_LITERAL,
            std::format("Invalid direction literal '{}'", next_token.lexeme));

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
    case IDs::ID_LITERAL_NULL: {
      return std::make_unique<AST::ValLiteral>(next_token,
                                               agentlang::Types::NullType{});
    }
    case IDs::ID_DELIM_PAREN_OPEN: {
      auto expr = parse_expr();
      if (!expr.has_value()) return expr.error();
      auto use_close_paren = m_Lexer.UseIf(IDs::ID_DELIM_PAREN_CLOSE);
      if (!use_close_paren.has_value()) return use_close_paren.error();
      return std::move(expr.value());
    }
    default:
      return ParseErr(ParseErr::INVALID_TERM,
                      std::format("Invalid term '{}'", next_token.lexeme));
  }
}

};  // namespace cse498
