#pragma once
#include "Interpreter/SymbolTable.hpp"
#include "Interpreter/agentlang.hpp"
#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"
#include "Interpreter/lexer.hpp"
#include <memory>
namespace cse498 {

struct Parser {
  enum class Env { TRAFFIC, INFECTION } m_Env;
  SymbolTable m_Syms{};
  std::vector<std::unique_ptr<AST::Node>> m_Nodes;

  std::expected<void, InterpErr> parse(std::istream &);

private:
  cse498::AgentLexer::Lexer m_Lexer{};
  int m_InLoop = 0;
  bool m_InInit = false;
  bool m_InTurn = false;

  std::expected<std::unique_ptr<AST::Node>, InterpErr> parse_stmt();
  std::expected<std::unique_ptr<AST::Node>, InterpErr>
  parse_expr(int prec = agentlang::Operators::MAX_PREC);
  std::expected<std::unique_ptr<AST::Node>, InterpErr> parse_term();
  std::expected<std::unique_ptr<AST::Node>, InterpErr> parse_stmt_block();

  std::expected<std::unique_ptr<AST::Node>, InterpErr> parse_if();
  std::expected<std::unique_ptr<AST::Node>, InterpErr> parse_while();
  std::expected<std::unique_ptr<AST::Node>, InterpErr> parse_var_def();
  std::expected<std::unique_ptr<AST::Node>, InterpErr> parse_agent_def();
  std::expected<std::unique_ptr<AST::Node>, InterpErr> parse_loop_ctl();
  std::expected<std::unique_ptr<AST::Node>, InterpErr> parse_move();

  std::expected<Token, InterpErr> parse_type();
};

}; // namespace cse498
